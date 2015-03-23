#include "stdafx.h"
#include "ui\FileFolderDialog.h"
#include "Memory.h"
#include <Shlobj.h>

#pragma comment(lib,"Comdlg32.lib")

const PCWCHAR DEFAULT_FILESAVE_TITLE = L"Enter filename to save";
const PCWCHAR DEFAULT_FILESELECT_TITLE = L"Select a file";
const PCWCHAR DEFAULT_DIRSELECT_TITLE = L"Select a directory";

bool DlgiCreateDirectorySelectionDialog(HWND hwnd, PCWCHAR title, PCWCHAR initial, DIALOG *dlgObject)
{
	BROWSEINFOW bi={0};
	LPITEMIDLIST iil;
	IMalloc *imalloc=NULL;
	bi.lpszTitle = title == NULL ? DEFAULT_DIRSELECT_TITLE : title;
	
	iil = SHBrowseForFolderW(&bi);

	if (!iil)
		return false;

	if (!SHGetPathFromIDListW(iil,dlgObject->path))
		return false;

	if (SUCCEEDED(SHGetMalloc(&imalloc)))
	{
		imalloc->Free(iil);
		imalloc->Release();
	}

	return true;
}

bool DlgiCreateFileSaveSelectDialog(HWND hwnd,DialogType type, PCWCHAR title, PCWCHAR initial, PCWCHAR filter,DIALOG *dlgObject)
{
	OPENFILENAMEW ofn = {0};

	if (title == NULL)
		title = type == FileDialogSave ? DEFAULT_FILESAVE_TITLE : DEFAULT_FILESELECT_TITLE;

	ofn.hwndOwner = hwnd;
	ofn.lStructSize = sizeof(OPENFILENAMEW);
	ofn.lpstrFileTitle = (LPWSTR)title;
	ofn.lpstrFilter = filter==NULL ? L"All Files\0*.*\0" : filter;
	ofn.lpstrFile = dlgObject->path;
	//ofn->lpstrDefExt = ?;
	ofn.nMaxFile = MAX_PATH;
	ofn.nMaxFileTitle = 0;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST;
	
	if (type == FileDialogSave)
		ofn.Flags |= OFN_FILEMUSTEXIST;

	ofn.lpstrFile[0] = 0;
	ofn.lpstrInitialDir = initial==NULL ? L"C:\\" : initial;
	
	return (bool) (type == FileDialogSave) ? GetSaveFileNameW(&ofn) : GetOpenFileNameW(&ofn);
}

bool DlgOpenSpecialDialog(HWND hwnd, DialogType type, PCWCHAR title, PCWCHAR initialDirectory, DIALOG **dlgObject)
{
	return DlgOpenSpecialDialogEx(hwnd,type,title,initialDirectory,NULL,dlgObject);
}

bool DlgOpenSpecialDialogEx(HWND hwnd, DialogType type, PCWCHAR title, PCWCHAR initialDirectory, PCWCHAR filter, DIALOG **dlgObject)
{
	bool result;

	if (dlgObject == NULL)
		return false;

	*dlgObject = ALLOCOBJECT(DIALOG);
	(*dlgObject)->type = type;

	if (type == FolderDialogSelect)
		return DlgiCreateDirectorySelectionDialog(hwnd,title,initialDirectory,*dlgObject);

	return DlgiCreateFileSaveSelectDialog(hwnd,type,title,initialDirectory,filter,*dlgObject);
}

void DlgFreeDialog(DIALOG *dlgObject)
{
	if (dlgObject)
		FREEOBJECT(dlgObject);
}