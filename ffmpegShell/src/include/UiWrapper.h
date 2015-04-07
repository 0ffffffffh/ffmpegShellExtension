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
		return ShowDialog(true);
	}

	virtual bool ShowDialog(bool seperateUiThread)
	{
		this->uiObject = UiCreateDialog(
			(UIDLGPROC)this->DialogProc,
			NULL,
			this->dlgId,
			(BOOL)seperateUiThread,
			this,
			NULL,
			NULL);

		return this->uiObject != NULL;
	}

	void Close()
	{
		UiDestroyDialog(this->uiObject);
	}

	uint4 GetControlTextA(uint4 ctrlId, anstring strBuf, uint4 bufSize)
	{
		wnstring wbuf = ALLOCSTRINGW(bufSize);
		anstring as;
		uint4 textLen;

		if (!wbuf)
			return 0;

		textLen = GetControlText(ctrlId,wbuf,bufSize);

		if (textLen > 0)
		{
			as = ffhelper::Helper::WideToAnsiString(wbuf);

			if (as != NULL)
			{
				strncpy(strBuf,as,textLen);
				FREESTRING(as);
			}
		}

		FREESTRING(wbuf);

		return textLen;
	}

	uint4 GetControlText(uint4 ctrlId, wnstring strBuf, uint4 bufSize)
	{
		HWND ctrlHwnd = GetDlgItem(this->uiObject->hwnd,ctrlId);

		if (!ctrlHwnd)
			return 0;

		return GetWindowTextW(ctrlHwnd,(LPWSTR)strBuf,bufSize);
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

