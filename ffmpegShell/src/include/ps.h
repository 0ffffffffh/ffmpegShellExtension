#pragma once

#include "stdafx.h"
#include "Synch.h"

typedef void (*STDOUT_RECEIVE_ROUTINE)(LPVOID,LPSTR,int);

typedef struct 
{
	STDOUT_RECEIVE_ROUTINE		callback;
	LPVOID						arg;
	char *						buffer;
	uint4						size;
}RECEIVE_CALLBACK_INFO;

#define PS_PIPE_HANDLE_COUNT	2

#define PS_STDERR_PIPE_HANDLE	0
#define PS_STDOUT_PIPE_HANDLE	1


typedef struct
{
	wnstring				commandLine;
	HANDLE					processHandle;
	HANDLE					stdOutWorkerHandle;
	HANDLE					stdPipeHandles[PS_PIPE_HANDLE_COUNT];
	DWORD					currentPipeHandleIndex;
	RECEIVE_CALLBACK_INFO	stdOutReceiveCallback;
	BOOL					running;
	SPINLOCK				stdPipeIoLock;
}PROCESS;

PROCESS *PsExecuteProcess(wnstring processImageName, wnstring arg, STDOUT_RECEIVE_ROUTINE callback,LPVOID cbArg);

PROCESS *PsExecuteProcessEx(wnstring processImageName, wnstring arg, STDOUT_RECEIVE_ROUTINE callback, LPVOID cbArg, bool passAsArgv, uint4 bufferStringSize);

BOOL PsKillProcess(PROCESS *process);

BOOL PsReleaseProcessResources(PROCESS *process);

int PsWaitForExit(PROCESS *process);

void PsWaitForStdoutReadCompletion(PROCESS *process);