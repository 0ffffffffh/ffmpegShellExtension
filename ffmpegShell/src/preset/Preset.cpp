#include "stdafx.h"
#include "Preset.h"
#include "FileReadWrite.h"
#include "PresetCompiler.h"

LinkedList<PRESET *> *PtLoadPreset(wstring presetFile)
{
	LinkedList<PRESET *> *pstList=NULL;
	PRESET_FILE_HEADER pstHeader;
	PRESET *presetItem;
	FileReadWrite *pstIo = new FileReadWrite(presetFile,OpenForRead,OpenExisting);

	if (!pstIo->Open())
		return NULL;

	if (pstIo->Read((byte *)&pstHeader,0,sizeof(PRESET_FILE_HEADER)) != sizeof(PRESET_FILE_HEADER))
		goto cleanUp;

	if (pstHeader.magic != PCF_MAGIC || !pstHeader.presetCount)
		goto cleanUp;

	pstList = new LinkedList<PRESET *>();

	for (uint4 i=0;i<pstHeader.presetCount;i++)
	{
		presetItem = ALLOCOBJECT(PRESET);

		pstIo->Read((byte *)presetItem,-1,sizeof(PRESET));
		pstList->Insert(presetItem);
	}

cleanUp:
	delete pstIo;
	return pstList;
}