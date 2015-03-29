#pragma once
#include "stdafx.h"
#include "PresetParser.h"

typedef struct
{
	uint4		magic;
	uint4		presetCount;
	uint4		cvidStartOffset; //compiled variable information directory 
	uint4		reserved[4];
}PRESET_FILE_HEADER;

typedef struct 
{
	PRESET		preset;
	uint4		ffmpegCmdCvidId;
	uint4		runAfterCmdCvidId;
}PRESET_RECORD;

typedef struct
{
	uint4		id;
	uint4		varCount;
	uint4		nextCvidHeaderOffset;
	uint4		varExtGroupOffset;
}CVID_RECORD_HEADER;

typedef struct
{
	wchar		ext[8];
	uint4		refCount;
}CVID_EXT_INFO;

typedef struct
{
	ushort		type;
	uint4		bPos;
	uint4		ePos;
	uint4		linkedExtOffsets[16];
}CVID_RECORD;




#define PCF_MAGIC (uint4)0x40FF88FE

bool PcCompilePreset(wnstring presetFile, wnstring outputFile,COMPILATION_EVENT_HANDLER eventHandler, void *arg);

bool PcDecompilePreset(wnstring compiledPresetFile);