#pragma once
#include "stdafx.h"
#include "PresetParser.h"

typedef struct
{
	uint4 magic;
	uint4 presetCount;
	uint4 reserved[4];
}PRESET_FILE_HEADER;

#define PCF_MAGIC (uint4)0x40FF88FE

bool PcCompilePreset(wnstring presetFile, wnstring outputFile,COMPILATION_EVENT_HANDLER eventHandler, void *arg);

bool PcDecompilePreset(wnstring compiledPresetFile);