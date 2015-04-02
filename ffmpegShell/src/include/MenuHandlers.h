#pragma once
#include "stdafx.h"
#include "PresetCompiler.h"
#include "ui\UI.h"
#include "ui\DialogImpls.h"
#include "ffmpeg\MediaInfo.h"
#include "ffmpeg\CommandExecutor.h"
#include "helper\ArgPack.h"
#include "Preset.h"

#define DECL_HANDLER(handlerName) static bool handlerName(vptr arg)

#define CASTARG(type) ((type)arg)

//Handle exit standart macros
#define HE_SUCCESS return true
#define HE_FAILED return false
#define HE_RET(x) return x


class MenuHandlers
{
	 static void CompilationEventHandler(void *p, LPCSTR msg)
	 {
		 wnstring ws = mbtowc_h(msg);
		 CompileDialog *dlg = (CompileDialog *)p;

		 dlg->AddStatusLine(ws);

		 //dlg->SetControlText(IDC_TXTCOMPILELOG,ws);

		 FREESTRING(ws);
	 }

	 static MediaInfo *gps_MediaInfo;

public:
	DECL_HANDLER(ShowSettings)
	{
		SettingsDlg * settings = new SettingsDlg();
		settings->ShowDialog();

		HE_SUCCESS;
	}

	DECL_HANDLER(StartConvertingOperation)
	{
		vptr argPack;
		WCHAR fileName[MAX_PATH];
		FileList *fileList;
		PRESET *preset;
		int4 pkOff=0;

		argPack = CASTARG(vptr);

		pkOff = READPACKET(argPack,PRESET *,pkOff,&preset);
		pkOff = READPACKET(argPack,FileList *,pkOff,&fileList);

		FREEPACKET(argPack);

		CommandExecutor cmexec;
		cmexec.Execute(preset,fileList);
		
		HE_SUCCESS;
	}

	DECL_HANDLER(ShowMediaInformations)
	{
		WCHAR fileName[MAX_PATH];
		FILEPATHITEM *item = CASTARG(FILEPATHITEM *);
		Duration dt;

		FlGeneratePathString(item,fileName,MAX_PATH,PAS_NONE,NULL);

		MediaInfo *mediaInfo = new MediaInfo((wnstring)fileName);

		//TODO: Fix valid stream index
		mediaInfo->GetBitrate(0);
		HE_SUCCESS;
	}

	DECL_HANDLER(CompilePresetHandler)
	{
		bool ret;
		WCHAR fileName[MAX_PATH];
		wnstring outputFile;

		CompileDialog *dlg = new CompileDialog();
		
		FILEPATHITEM *item = CASTARG(FILEPATHITEM *);

		if (!item)
			HE_FAILED;

		outputFile = ffhelper::Helper::MakeAppPath(L"presets.cpf");

		dlg->ShowDialog();

		FlGeneratePathString(item,fileName,MAX_PATH,PAS_NONE,NULL);
		
		ret = PcCompilePreset(fileName,outputFile,MenuHandlers::CompilationEventHandler,dlg);

		if (ret)
			PtReLoadPreset(outputFile);

		HE_RET(ret);
	}

	DECL_HANDLER(OpenPresetSettingsDialog)
	{
		NOTIMPLEMENTED_R(false);
	}
};

