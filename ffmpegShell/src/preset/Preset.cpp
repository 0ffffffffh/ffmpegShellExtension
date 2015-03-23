#include "stdafx.h"
#include "Preset.h"
#include "FileReadWrite.h"
#include "PresetCompiler.h"
#include <map>

using namespace std;

map<wchar *, LinkedList<PRESET *>*> *gp_presetList=NULL;

void PtxInsertPresetToMappedList(PRESET *preset)
{
	map<wchar *,LinkedList<PRESET *> *>::iterator it;
	LinkedList<PRESET *> *presetList = NULL;
	
	it = gp_presetList->find((wchar *)preset->sourceFormat);

	if (it != gp_presetList->end())
	{
		presetList = it->second;
	}
	else
	{
		presetList = new LinkedList<PRESET *>();
		gp_presetList->insert(make_pair((wchar *)preset->sourceFormat,presetList));
	}

	presetList->Insert(preset);
}

bool PtLoadPreset(wnstring presetFile)
{
	PRESET_FILE_HEADER pstHeader;
	PRESET *presetItem;
	FileReadWrite *pstIo = new FileReadWrite(presetFile,OpenForRead,OpenExisting);

	if (!pstIo->Open())
		return false;

	if (pstIo->Read((byte *)&pstHeader,0,sizeof(PRESET_FILE_HEADER)) != sizeof(PRESET_FILE_HEADER))
		goto cleanUp;

	if (pstHeader.magic != PCF_MAGIC || !pstHeader.presetCount)
		goto cleanUp;

	gp_presetList = new map<wchar *,LinkedList<PRESET *> *>();

	for (uint4 i=0;i<pstHeader.presetCount;i++)
	{
		presetItem = ALLOCOBJECT(PRESET);

		pstIo->Read((byte *)presetItem,-1,sizeof(PRESET));
		PtxInsertPresetToMappedList(presetItem);
	}

cleanUp:
	delete pstIo;

	return gp_presetList != NULL;
}

LinkedList<PRESET *> *PtGetPresetsByExtension(wchar *presetExt)
{
	map<wchar *,LinkedList<PRESET *> *>::iterator it;

	it = gp_presetList->find(presetExt);

	if (it == gp_presetList->end())
		return NULL;

	return it->second;
}