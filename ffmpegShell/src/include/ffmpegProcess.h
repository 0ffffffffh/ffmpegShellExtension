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

static const wstring FFMPEG_BINARY = L"D:\\systools\\ffmpeg\\bin\\ffmpeg.exe";
static const wstring FFPROBE_BINARY = L"D:\\systools\\ffmpeg\\bin\\ffprobe.exe";

class ffmpegProcess
{
	PROCESS *process;
	wstring argList;
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

	ffmpegProcess(FFMPEG_PROCESS_TYPE type = FFMPEG_PROCESS_TYPE::FFMPEG)
	{
		this->OnLineReceived = NULL;
		this->procType = type;
		this->argList = NULL;
	}

	~ffmpegProcess()
	{
	}

	void SetArg(wstring argLine)
	{
		this->argList = argLine;
	}

	bool Start(vptr arg)
	{
		wstring binary;
		wstring cmdLine;

		this->cbArg = arg;

		if (this->procType == FFPROBE)
			binary = FFPROBE_BINARY;
		else
			binary = FFMPEG_BINARY;

		cmdLine = (wstring)ALLOCSTRINGW(wcslen(binary) + wcslen(this->argList));

		wsprintf(cmdLine,L"%s %s",binary,this->argList);

		this->process = PsExecuteProcess(NULL,cmdLine,(STDOUT_RECEIVE_ROUTINE)ffmpegProcess::StdoutReceive,this);
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