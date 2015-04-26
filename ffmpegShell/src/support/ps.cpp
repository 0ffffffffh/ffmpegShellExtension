#include "stdafx.h"
#include "ps.h"
#include "Memory.h"
#include "helper\ByteBuffer.h"

void _PsihDispatchCallback(PROCESS *process, int length)
{
	process->stdOutReceiveCallback.callback(
		process->stdOutReceiveCallback.arg,
		process->stdOutReceiveCallback.buffer,
		length);
					
	memset(
		process->stdOutReceiveCallback.buffer,
		0,
		process->stdOutReceiveCallback.size);
}

INT _PsihPeekAvailableBytesOnPipe(
	HANDLE *pipeHandles,
	INT handleCount, 
	INT peekTry,	
	DWORD peekDelay,
	PDWORD totalAvailableBytes)
{

	DWORD totalAvail=0;

	for (INT i=0;i<handleCount;i++)
	{
		while (peekTry >= 0)
		{
			PeekNamedPipe(pipeHandles[i],NULL,0,NULL,&totalAvail,NULL);

			if (totalAvail > 0)
			{
				if (totalAvailableBytes != NULL)
					*totalAvailableBytes = totalAvail;

				return i;
			}

			peekTry--;
			SleepEx(peekDelay,FALSE);
		}
	}

	if (totalAvailableBytes != NULL)
		*totalAvailableBytes = 0;

	return -1;
}

HANDLE _PsiGetCurrentHandle(PROCESS *process)
{
	return process->stdPipeHandles[process->currentPipeHandleIndex];
}

UINT WINAPI _PsiStandartOutputReceiveWorker(LPVOID p)
{
	PROCESS *process = (PROCESS *)p;
	DWORD readLen=0,totalAvail;
	HANDLE stdHandle;
	int4 pos=0,bufLen=0;
	int4 bufferEndLastChanceWaitTryCount=8;

	ByteBuffer buffer(0x1000);
	BOOL keepRun=TRUE,crLf=FALSE;
	INT index;
	
	if (!process->running)
		return 0;


	
	//Do the initial peek for a bit long
	index = _PsihPeekAvailableBytesOnPipe(process->stdPipeHandles,PS_PIPE_HANDLE_COUNT,20,100,NULL);

	AcquireSpinLock(&process->stdPipeIoLock);

	if (index < 0)
	{
		DPRINT("There is no ready pipe for read. Assumed stderr");
		process->currentPipeHandleIndex = PS_STDERR_PIPE_HANDLE;
	}
	else
		process->currentPipeHandleIndex = index;

	ReleaseSpinLock(&process->stdPipeIoLock);


	DPRINT("Initial seed: %d",process->currentPipeHandleIndex);

	
	while (!process->cancelRequested && keepRun)
	{
		stdHandle = _PsiGetCurrentHandle(process);

		_PsihPeekAvailableBytesOnPipe(&stdHandle,1,5,50,&totalAvail);

		DPRINT("Total avail %d bytes on #%d",totalAvail,process->currentPipeHandleIndex);
		
		if (totalAvail > 0 && ReadFile(stdHandle,buffer.GetWritableBuffer(),buffer.GetRemainSize(),&readLen,NULL))
		{
			DPRINT("%d bytes readed #%d",readLen,process->currentPipeHandleIndex);

			buffer.SetWrittenSize(readLen);
			bufLen = buffer.GetLength();


			if (bufferEndLastChanceWaitTryCount < 8)
			{
				bufferEndLastChanceWaitTryCount=8;
			}
			
			for (pos = 0; pos < buffer.GetLength(); pos++)
			{
				if ( ((char)buffer[pos]) == '\n' || ((char)buffer[pos] == '\r'))
				{
					if (pos > 0)
						crLf = ((char)buffer[pos-1]) == '\r';
					else
						crLf = FALSE;

					if (pos <= 1) 
					{
						//skip empty line
						buffer.Remove(0,crLf ? 2 : 1);
						pos=0;
						continue;
					}

					buffer.Read(
						(byte *)process->stdOutReceiveCallback.buffer,
						0,
						pos - (crLf ? 1 : 0), //we dont need newline
						false);

					buffer.Remove(0,pos + (crLf ? 1 : 0));
					
					_PsihDispatchCallback(process,pos);

					pos = 0;
				}
			}
		}
		else
		{
			if (bufferEndLastChanceWaitTryCount >= 0)
			{
				bufferEndLastChanceWaitTryCount--;
				continue;
			}
			else
				bufferEndLastChanceWaitTryCount=8;

			DPRINT("Passing next handle");

			AcquireSpinLock(&process->stdPipeIoLock);

			process->currentPipeHandleIndex++;

			ReleaseSpinLock(&process->stdPipeIoLock);

			if (buffer.GetLength() > 0)
				_PsihDispatchCallback(process,buffer.GetLength());

			if (process->currentPipeHandleIndex == PS_PIPE_HANDLE_COUNT)
			{
				keepRun=FALSE;
			}
			else
			{
				stdHandle = _PsiGetCurrentHandle(process);

				_PsihPeekAvailableBytesOnPipe(&stdHandle,1,20,100,NULL);
				
				pos = 0;
				bufLen = 0;
				buffer.Clear();
			}
		}

		ffhelper::Helper::DelayExecution(1);
	}

	return 0;
}

BOOL PsiCreateStdOutWorker(PROCESS *process)
{
	DWORD tid=0;

	if (!process)
		return FALSE;

	//Create suspended worker thread.
	process->stdOutWorkerHandle = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)_PsiStandartOutputReceiveWorker,process,CREATE_SUSPENDED,&tid);

	return process->stdOutWorkerHandle != INVALID_HANDLE_VALUE;
}

BOOL PsiStillRunning(PROCESS *process)
{
	DWORD exitCode;

	if (!process->running)
		return FALSE;

	if (!GetExitCodeProcess(process->processHandle,&exitCode))
		return FALSE;

	return exitCode == STILL_ACTIVE;
}

PROCESS *PsExecuteProcess(wnstring processImageName, wnstring arg, STDOUT_RECEIVE_ROUTINE callback,LPVOID cbArg)
{
	return PsExecuteProcessEx(processImageName,arg,callback,cbArg,false,0x4000);
}

PROCESS *PsExecuteProcessEx(wnstring processImageName, wnstring arg, STDOUT_RECEIVE_ROUTINE callback, LPVOID cbArg, bool passAsArgv, uint4 bufferStringSize)
{
	STARTUPINFOW psi;
	PROCESS_INFORMATION processInfo;
	PROCESS *process = NULL;
	HANDLE pipeWrite=NULL,pipeRead=NULL;
	HANDLE pipeErrWrite=NULL,pipeErrRead=NULL;
	SECURITY_ATTRIBUTES sa;
	LPWSTR cmdLine = NULL,argvPref=NULL;
	
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	//create dynamic process object for us
	process = ALLOCOBJECT(PROCESS);

	if (!process)
		return NULL;

	//create pipe for STDOUT redirection
	if (!CreatePipe(&pipeRead,&pipeWrite,&sa,0))
		goto cleanUp;

	if (!CreatePipe(&pipeErrRead,&pipeErrWrite,&sa,0))
		goto cleanUp;

	//fill PSI with STDOUT redirection option
	RtlZeroMemory(&psi,sizeof(STARTUPINFOW));
	psi.cb = sizeof(STARTUPINFOW);
	psi.dwFlags = STARTF_USESTDHANDLES;
	psi.hStdOutput = pipeWrite;
	psi.hStdError = pipeErrWrite;

	//Create stdout comsume worker thread
	if (!PsiCreateStdOutWorker(process))
		goto cleanUp;

	
	if (arg != NULL)
	{
		cmdLine = ALLOCARRAY(wchar,MAX_PATH);

		if (passAsArgv)
		{
			argvPref = ffhelper::Helper::StringLastIndexOf(processImageName,L'\\');
			wsprintfW(cmdLine,L"%s ",argvPref != NULL ? argvPref : processImageName);
		}

		wcsncat(cmdLine,arg,wcslen(arg));
	}

	//Ok. All required resources guaranteed. Now fire up the process
	if (!CreateProcessW(processImageName,cmdLine,&sa,&sa,TRUE,CREATE_NO_WINDOW,NULL,NULL,&psi,&processInfo))
	{
		Win32Error();
		goto cleanUp;
	}

	//We dont need write pipe anymore. Because its duplicated by fired up process
	CloseHandle(pipeWrite);
	CloseHandle(pipeErrWrite);
	
	//So we collect some required things
	process->processHandle = processInfo.hProcess;
	process->stdPipeHandles[PS_STDOUT_PIPE_HANDLE] = pipeRead;
	process->stdPipeHandles[PS_STDERR_PIPE_HANDLE] = pipeErrRead;

	process->commandLine = cmdLine;

	//allocate receive buffer
	process->stdOutReceiveCallback.buffer = ALLOCSTRINGA(bufferStringSize);
	process->stdOutReceiveCallback.callback = callback;
	process->stdOutReceiveCallback.arg = cbArg;
	process->stdOutReceiveCallback.size = bufferStringSize;

	process->running = TRUE;

	//raise worker thread
	ResumeThread(process->stdOutWorkerHandle);
	
	return process;

cleanUp: //Release the resources if it has failed

	if (process->stdOutWorkerHandle != NULL)
		TerminateThread(process->stdOutWorkerHandle,0);

	
	if (pipeWrite != NULL)
		CloseHandle(pipeWrite);

	if (pipeErrWrite != NULL)
		CloseHandle(pipeErrWrite);

	for (int i=0;i<PS_PIPE_HANDLE_COUNT;i++)
	{
		if (process->stdPipeHandles[i] != NULL)
		{
			CloseHandle(process->stdPipeHandles[i]);
		}
	}

	if (process != NULL)
		FREEOBJECT(process);

	return NULL;

}

BOOL PsKillProcess(PROCESS *process)
{
	if (process == NULL)
		return FALSE;

	//request cancellation
	process->cancelRequested = TRUE;

	//Terminate process
	if (TerminateProcess(process->processHandle,0))
	{
		//Wait until stdout worker completed.
		WaitForSingleObject(process->stdOutWorkerHandle,INFINITE);
		
		//release process resources
		PsReleaseProcessResources(process);

		return TRUE;
	}

	return FALSE;
}

BOOL PsReleaseProcessResources(PROCESS *process)
{
	if (!process)
		return FALSE;

	if (PsiStillRunning(process))
		return FALSE;

	process->cancelRequested = TRUE;

	WaitForSingleObject(process->stdOutWorkerHandle,2000);

	CloseHandle(process->processHandle);
	CloseHandle(process->stdOutWorkerHandle);
	
	for (int i=0;i<PS_PIPE_HANDLE_COUNT;i++)
	{
		if (process->stdPipeHandles[i] != NULL)
		{
			CloseHandle(process->stdPipeHandles[i]);
		}
	}

	FREEOBJECT(process->commandLine);
	FREEOBJECT(process->stdOutReceiveCallback.buffer);
	FREEOBJECT(process);

	return TRUE;
}

int PsWaitForExit(PROCESS *process)
{
	int exitCode;

	WaitForSingleObject(process->processHandle,INFINITE);

	GetExitCodeProcess(process->processHandle,(LPDWORD)&exitCode);

	return exitCode;
}

void PsWaitForStdoutReadCompletion(PROCESS *process)
{
	WaitForSingleObject(process->stdOutWorkerHandle,INFINITE);
}