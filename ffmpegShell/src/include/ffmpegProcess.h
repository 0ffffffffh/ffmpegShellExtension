#pragma once

#include "stdafx.h"
#include "ps.h"
#include "string\AutoString.h"
#include "LinkedList.h"

typedef void (*OnLineReceivedHandler)(void*, LPCSTR line);

typedef enum 
{
	FFMPEG,
	FFPROBE
}FFMPEG_PROCESS_TYPE;

static const wnstring FFMPEG_BINARY = L"ffmpeg.exe";
static const wnstring FFPROBE_BINARY = L"ffprobe.exe";

class ffmpegProcess
{
	PROCESS *process;
	wnstring argList;
	vptr cbArg;

	
	static void StdoutReceive(LPVOID arg, LPCSTR line, int size)
	{
		ffmpegProcess *ffp = (ffmpegProcess *)arg;
		
		if (ffp->OnLineReceived != NULL)
			ffp->OnLineReceived(ffp->cbArg,line);
	}

private:

	FFMPEG_PROCESS_TYPE procType;

public:

	ffmpegProcess(FFMPEG_PROCESS_TYPE type = FFMPEG)
	{
		this->OnLineReceived = NULL;
		this->procType = type;
		this->argList = NULL;
	}

	~ffmpegProcess()
	{
		PsReleaseProcessResources(this->process);
	}

	void SetArg(wnstring argLine)
	{
		this->argList = argLine;
	}

	bool Start(vptr arg)
	{
		wnstring binary;
		wnstring cmdLine;

		this->cbArg = arg;

		
		if (this->procType == FFPROBE)
			binary = ffhelper::Helper::PathJoin((wnstring)g_settings.ffmpegBinaryPath,FFPROBE_BINARY);
		else
			binary = ffhelper::Helper::PathJoin((wnstring)g_settings.ffmpegBinaryPath,FFMPEG_BINARY);


		cmdLine = (wnstring)ALLOCSTRINGW(wcslen(binary) + wcslen(this->argList));

		wsprintf(cmdLine,L"%s %s",binary,this->argList);

		FREESTRING(binary);

		this->process = PsExecuteProcess(NULL,cmdLine,(STDOUT_RECEIVE_ROUTINE)ffmpegProcess::StdoutReceive,this);
		
		FREESTRING(cmdLine);

		return this->process != NULL;
	}

	void Close()
	{
		if (this->process != NULL)
			PsKillProcess(this->process);
	}

	void Wait(bool alsoWaitStdoutCompletion)
	{
		if (this->process != NULL)
			PsWaitForExit(this->process);

		if (alsoWaitStdoutCompletion)
			PsWaitForStdoutReadCompletion(this->process);
	}

	void Wait()
	{
		Wait(false);
	}

	

	OnLineReceivedHandler OnLineReceived;

};