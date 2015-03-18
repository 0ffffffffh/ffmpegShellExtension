#include "stdafx.h"
#include "PresetCompiler.h"
#include "PresetFileLexer.h"
#include "FileReadWrite.h"
#include "Preset.h"
#include "FileList.h"

FileReadWrite *PciOpenOutput(wstring outputFile)
{
	bool exists = FileReadWrite::FileExists(outputFile);

	FileReadWrite *output = new FileReadWrite(outputFile,
										exists ? OpenForReadWrite : OpenForWrite,
										exists ? OpenExisting : CreateNew);

	if (!output->Open())
	{
		delete output;
		return NULL;
	}

	return output;
}

bool PciCompilePreset(LinkedList<PRESETOBJECT *> *presets, wstring outputFile)
{
	FileReadWrite *fileIo;
	PRESETOBJECT *po;
	PRESET preset;
	PRESET_FILE_HEADER presetHeader;
	bool result=false;
	long written;

	fileIo = PciOpenOutput(outputFile);

	if (fileIo->GetOpenType() == CreateNew)
	{
		presetHeader.magic = PCF_MAGIC;
		presetHeader.presetCount = presets->GetCount();
		memset(presetHeader.reserved,0xcb,sizeof(presetHeader.reserved));

		written = fileIo->Write((byte *)&presetHeader,-1,sizeof(PRESET_FILE_HEADER));

		//validate written size
		if (written != sizeof(PRESET_FILE_HEADER))
		{
			delete fileIo;
			return false;
		}
	}
	else
	{
		if (fileIo->Read((byte *)&presetHeader,0,sizeof(PRESET_FILE_HEADER)) != sizeof(PRESET_FILE_HEADER))
		{
			delete fileIo;
			return false;
		}

		fileIo->Seek(presetHeader.presetCount * sizeof(PRESET),SeekType::Current);
	}

	for (LinkedListNode<PRESETOBJECT *> *node = presets->Begin();
		node != NULL;
		node = node->Next())
	{
		po = node->GetValue();

		wcscpy(preset.name,po->preset.name);
		wcscpy(preset.command,po->preset.command);
		wcscpy(preset.sourceFormat,po->preset.sourceFormat);
		wcscpy(preset.destinationFormat,po->preset.destinationFormat);
		wcscpy(preset.runOnFinish,po->preset.runOnFinish);

		preset.mediaType = po->preset.mediaType;
		preset.opType = po->preset.opType;

		written = fileIo->Write((byte *)&preset,-1,sizeof(PRESET));

		if (written != sizeof(PRESET))
		{
			delete fileIo;
			return false;
		}
	}

	presetHeader.presetCount += presets->GetCount();
	fileIo->Seek(0,SeekType::Begin);

	fileIo->Write((byte *)&presetHeader,0,sizeof(PRESET_FILE_HEADER));

	delete fileIo;
	return true;
}

bool PcCompilePreset(wstring presetFile, wstring outputFile, COMPILATION_EVENT_HANDLER eventHandler, void *arg)
{
	bool result;
	PresetFileLexer *lexer = new PresetFileLexer(presetFile,eventHandler,arg);
	PresetParser *parser;

	LinkedList<PRESETOBJECT *> *presetObjects;
	
	eventHandler(arg,"Compilation started...");

	
	if (lexer->Lex())
	{
		
		parser = lexer->GetParser();

		if (parser->Parse())
			presetObjects = parser->GetPresetObjects();
		else
			return false;
	}
	else
		return false;

	eventHandler(arg,"Parsed preset source. Compiling binary");

	result = PciCompilePreset(presetObjects,outputFile);

	delete lexer;
	
	return result;
}

bool PcDecompilePreset(wstring compiledPresetFile)
{
	PRESET_FILE_HEADER pstHeader;
	PRESET preset;
	AutoString<wchar> codeText;
	FileReadWrite *fileIo = new FileReadWrite(compiledPresetFile,OpenForRead,OpenExisting);
	FileReadWrite *dcmpFile;
	wstring decompFileName,dateStr;
	bool status=false;
	FilePathItem *fpi;
	
	if (!fileIo->Open())
		goto cleanUp;

	fileIo->Read((byte *)&pstHeader,0,sizeof(PRESET_FILE_HEADER));

	if (pstHeader.magic != PCF_MAGIC)
		goto cleanUp;
	
	fpi = FilePathItem::Parse(compiledPresetFile);

	fpi->SetExtension(L"txt");

	fpi->GeneratePathString(&decompFileName);

	delete fpi;

	dcmpFile = new FileReadWrite(decompFileName,OpenForWrite,CreateNew);

	FREESTRING(decompFileName);

	if (!dcmpFile->Open())
		goto cleanUp;

	
	dateStr = ALLOCSTRINGW(32);

	ffhelper::Helper::GetCurrentDateTimeString(dateStr,32,0);

	codeText.AppendFormat(L"< Auto generated decompiled file. Decompilation date: %s >\r\n\r\n",dateStr);

	FREESTRING(dateStr);

	for (uint4 i=0;i<pstHeader.presetCount;i++)
	{
		fileIo->Read((byte *)&preset,-1,sizeof(PRESET));

		codeText += L"preset {\r\n";
		codeText.AppendFormat(L"\tname : \"%s\",\r\n",preset.name);
		codeText.AppendFormat(L"\tcommand : \"%s\",\r\n",preset.command);
		//add other fields
		codeText += L"};\r\n\r\n";
	}

	dcmpFile->Write(PCF_MAGIC,-1);
	dcmpFile->Write((byte *)codeText.GetNativeString(false),-1,codeText.GetLength() * sizeof(wchar));

	delete dcmpFile;

	status = true;

cleanUp:
	delete fileIo;
	return status;
}