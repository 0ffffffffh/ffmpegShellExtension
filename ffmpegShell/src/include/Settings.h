#pragma once

#include "stdafx.h"
#include "FileReadWrite.h"

typedef struct 
{
	uint2	ver;
	wchar	ffmpegBinaryPath[MAX_PATH];
	wchar	langFilename[MAX_PATH];
	byte	reserved[1024 * 4];
}SETTINGS;

bool IntLoadSettings();

bool IntCommitSettings();