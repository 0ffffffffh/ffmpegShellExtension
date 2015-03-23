#pragma once

#include "stdafx.h"

typedef void (*STDOUT_RECEIVE_ROUTINE)(LPVOID,LPSTR,int);

typedef struct 
{
	STDOUT_RECEIVE_ROUTINE		callback;
	LPVOID						arg;
	char *						buffer;
	uint4						size;
}RECEIVE_CALLBACK_INFO;

typedef struct
{
	wnstring					commandLine;
	HANDLE					processHandle;
	HANDLE					stdOutWorkerHandle;
	HANDLE					stdOutPipeHandle;
	RECEIVE_CALLBACK_INFO	stdOutReceiveCallback;
	BOOL					running;
}PROCESS;

PROCESS *PsExecuteProcess(wnstring processImageName, wnstring arg, STDOUT_RECEIVE_ROUTINE callback,LPVOID cbArg);

PROCESS *PsExecuteProcessEx(wnstring processImageName, wnstring arg, STDOUT_RECEIVE_ROUTINE callback, LPVOID cbArg, bool passAsArgv, uint4 bufferStringSize);

BOOL PsKillProcess(PROCESS *process);

BOOL PsReleaseProcessResources(PROCESS *process);

int PsWaitForExit(PROCESS *process);

void PsWaitForStdoutReadCompletion(PROCESS *process);