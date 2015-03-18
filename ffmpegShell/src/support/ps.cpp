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

UINT WINAPI _PsiStandartOutputReceiveWorker(LPVOID p)
{
	PROCESS *process = (PROCESS *)p;
	DWORD readLen=0;
	int pos=0,bufLen=0;
	ByteBuffer buffer(0x1000);
	BOOL keepRun=TRUE,crLf=FALSE;

	if (!process->running)
		return 0;

	while (keepRun)
	{
		//Read the process console output buffer.
		if (ReadFile(process->stdOutPipeHandle,buffer.GetWritableBuffer(),buffer.GetRemainSize(),&readLen,NULL))
		{
			buffer.SetWrittenSize(readLen);
			bufLen = buffer.GetLength();
			
			for (pos = 0; pos < buffer.GetLength(); pos++)
			{
				if ((char)buffer[pos] == '\n')
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
			if (buffer.GetLength() > 0)
				_PsihDispatchCallback(process,buffer.GetLength());

			keepRun=FALSE;
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

PROCESS *PsExecuteProcess(wstring processImageName, wstring arg, STDOUT_RECEIVE_ROUTINE callback,LPVOID cbArg)
{
	return PsExecuteProcessEx(processImageName,arg,callback,cbArg,false,0x4000);
}


PROCESS *PsExecuteProcessEx(wstring processImageName, wstring arg, STDOUT_RECEIVE_ROUTINE callback, LPVOID cbArg, bool passAsArgv, uint4 bufferStringSize)
{
	STARTUPINFOW psi;
	PROCESS_INFORMATION processInfo;
	PROCESS *process = NULL;
	HANDLE pipeWrite=NULL,pipeRead=NULL;
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

	//fill PSI with STDOUT redirection option
	RtlZeroMemory(&psi,sizeof(STARTUPINFOW));
	psi.cb = sizeof(STARTUPINFOW);
	psi.dwFlags = STARTF_USESTDHANDLES;
	psi.hStdOutput = pipeWrite;

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

	
	//So we collect some required things
	process->processHandle = processInfo.hProcess;
	process->stdOutPipeHandle = pipeRead;
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

	if (process != NULL)
		FREEOBJECT(process);

	if (pipeRead != NULL)
		CloseHandle(pipeRead);
	if (pipeWrite != NULL)
		CloseHandle(pipeWrite);

	return NULL;

}

BOOL PsKillProcess(PROCESS *process)
{
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

	CloseHandle(process->processHandle);
	CloseHandle(process->stdOutWorkerHandle);
	CloseHandle(process->stdOutWorkerHandle);

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