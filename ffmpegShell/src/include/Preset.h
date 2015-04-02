#pragma once
#include "LinkedList.h"

typedef enum
{
	Audio,
	Video
}MediaType;

typedef enum
{
	Conversion,
	Extraction,
	Combination
}OperationType;

typedef struct
{
	wchar name[64];
	wchar command[512];
	wchar sourceFormat[6];
	wchar destinationFormat[6];
	wchar runOnFinish[128];
	wchar typeName[32];
	MediaType mediaType;
	OperationType opType;
}PRESET;

bool PtDestroyPresets();
bool PtReLoadPreset(wnstring presetFile);
bool PtLoadPreset(wnstring presetFile);


LinkedList<PRESET *> *PtGetPresetsByExtension(wchar *presetExt);