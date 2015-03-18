#pragma once

#include "stdafx.h"

typedef enum
{
	FileDialogSave,
	FileDialogSelect,
	FolderDialogSelect
}DialogType;

typedef struct
{
	DialogType type;
	wchar path[MAX_PATH];
}DIALOG;

bool DlgOpenSpecialDialog(HWND hwnd,DialogType type, PCWCHAR title, PCWCHAR initialDirectory, DIALOG **dlgObject);

bool DlgOpenSpecialDialogEx(HWND hwnd,DialogType type, PCWCHAR title, PCWCHAR initialDirectory, PCWCHAR filter, DIALOG **dlgObject);

void DlgFreeDialog(DIALOG *dlgObject);

