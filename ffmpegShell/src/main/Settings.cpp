#include "stdafx.h"
#include "Settings.h"

SETTINGS g_settings = {0};

#define SETTING_FILE_VER MAKEWORD(0x01,0x00)

FileReadWrite *__IntInitializeSettingsFileIoObject(bool forWrite)
{
	OpenMode openMode = forWrite ? OpenForWrite : OpenForRead;
	OpenType openType = forWrite ? CreateAlways : OpenExisting; 

	wnstring settingsFile = ffhelper::Helper::MakeAppPath(L".conf");
	FileReadWrite *fileIo = new FileReadWrite(settingsFile,openMode,openType);

	if (!fileIo->Open())
	{
		delete fileIo;
		fileIo = NULL;
	}

	FREESTRING(settingsFile);

	return fileIo;
}

bool IntLoadSettings()
{
	FileReadWrite *fileIo;

	if (g_settings.ver > 0)
		return true;

	fileIo = __IntInitializeSettingsFileIoObject(false);

	if (fileIo == NULL)
		return false;

	fileIo->Read((byte *)&g_settings,-1,sizeof(SETTINGS));
	delete fileIo;

	if (g_settings.ver != SETTING_FILE_VER)
	{
		memset(&g_settings,0,sizeof(SETTINGS));
		return false;
	}

	return true;
}

bool IntCommitSettings()
{
	FileReadWrite *fileIo = __IntInitializeSettingsFileIoObject(true);

	if (fileIo == NULL)
		return false;

	g_settings.ver = SETTING_FILE_VER;

	fileIo->Write((byte *)&g_settings,0,sizeof(SETTINGS));

	delete fileIo;

	return true;
}