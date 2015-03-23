#pragma once
#include "stdafx.h"
#include "ui\UI.h"


class UiWrapper
{
private:

	static UINT_PTR CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, PVOID arg)
	{
		UiWrapper *_this = (UiWrapper *)arg;
		
		switch (msg)
		{
		case WM_INITDIALOG:
			_this->OnInit();
			break;
		case WM_COMMAND:
			_this->OnCommand(wp,lp);
			break;
		case WM_CLOSE:
			_this->OnClose();
			break;
		}


		return 0;
	}

	UIOBJECT *uiObject;
	int4 dlgId;

private:

	bool SetControlEnableState(uint4 ctrlId, bool state)
	{
		HWND ctrlHwnd = GetDlgItem(this->uiObject->hwnd,ctrlId);

		if (!ctrlHwnd)
			return false;

		return (bool)EnableWindow(ctrlHwnd,(BOOL)state);
	}

public:

	UiWrapper(int4 dlgId)
	{
		this->dlgId = dlgId;
	}

	~UiWrapper(void)
	{
		UiDestroyDialog(this->uiObject);
	}

	virtual bool ShowDialog()
	{
		this->uiObject = UiCreateDialog((UIDLGPROC)this->DialogProc,NULL,this->dlgId,this,NULL,NULL);
		return this->uiObject != NULL;
	}

	void Close()
	{
		UiDestroyDialog(this->uiObject);
	}

	bool SetControlText(uint4 ctrlId, wnstring str)
	{
		HWND ctrlHwnd = GetDlgItem(this->uiObject->hwnd,ctrlId);

		if (!ctrlHwnd)
			return false;

		return (bool)SetWindowTextW(ctrlHwnd,(LPCWSTR)str);
	}

	bool EnableControl(uint4 ctrlId)
	{
		return SetControlEnableState(ctrlId,true);
	}

	bool DisableControl(uint4 ctrlId)
	{
		return SetControlEnableState(ctrlId,false);
	}

	virtual void OnClose()
	{
	}

	virtual void OnCommand(WPARAM wp, LPARAM lp)
	{
	}

	virtual void OnInit()
	{
	}
};

