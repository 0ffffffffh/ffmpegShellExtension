#pragma once

#include "stdafx.h"
#include "FileReadWrite.h"

typedef struct 
{
	uint2 ver;
	wchar ffmpegBinaryPath[MAX_PATH];
}SETTINGS;

bool IntLoadSettings();

bool IntCommitSettings();