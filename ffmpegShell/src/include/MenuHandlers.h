#pragma once
#include "stdafx.h"
#include "PresetCompiler.h"
#include "ui\UI.h"

#define DECL_HANDLER(handlerName) static bool handlerName(vptr arg)

#define CASTARG(type) ((type)arg)

//Handle exit standart macros
#define HE_SUCCESS return true
#define HE_FAILED return false
#define HE_RET(x) return x

#include "ui\DialogImpls.h"
#include "MediaInfo.h"

class MenuHandlers
{
	 static void CompilationEventHandler(void *p, LPCSTR msg)
	 {
		 wstring ws = mbtowc_h(msg);
		 CompileDialog *dlg = (CompileDialog *)p;

		 dlg->AddStatusLine(ws);

		 //dlg->SetControlText(IDC_TXTCOMPILELOG,ws);

		 FREESTRING(ws);
	 }

	 static MediaInfo *gps_MediaInfo;

public:
	DECL_HANDLER(ShowMediaInformations)
	{
		WCHAR fileName[MAX_PATH];
		FILEPATHITEM *item = CASTARG(FILEPATHITEM *);
		Duration dt;

		FlGeneratePathString(item,fileName,MAX_PATH,PAS_NONE,NULL);

		MediaInfo *mediaInfo = new MediaInfo((wstring)fileName);

		mediaInfo->GetBitrate();
		HE_SUCCESS;
	}

	DECL_HANDLER(CompilePresetHandler)
	{
		WCHAR fileName[MAX_PATH],outputFile[MAX_PATH];
		CompileDialog *dlg = new CompileDialog();
		
		FILEPATHITEM *item = CASTARG(FILEPATHITEM *);
		FILEPATHITEM *outputItem;

		if (!item)
			HE_FAILED;

		outputItem = FlCloneNode(item);
		FlSetNodePart(outputItem,L"cpf",OPL_EXTENSION);

		dlg->ShowDialog();

		FlGeneratePathString(item,fileName,MAX_PATH,PAS_NONE,NULL);
		FlGeneratePathString(outputItem,outputFile,MAX_PATH,PAS_NONE,NULL);

		FlFreeClonedNode(outputItem);

		HE_RET(PcCompilePreset(fileName,outputFile,MenuHandlers::CompilationEventHandler,dlg)); //&dlg
	}

	DECL_HANDLER(OpenPresetSettingsDialog)
	{
		NOTIMPLEMENTED_R(false);
	}
};

