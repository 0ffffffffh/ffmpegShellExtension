#pragma once

#include "stdafx.h"
#include "..\ui\UI.h"
#include "..\ui\ctrl\UiControlBase.h"



class UiWrapper
{
private:

	static UINT_PTR CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, PVOID arg)
	{
		UiWrapper *_this = (UiWrapper *)arg;
		
		switch (msg)
		{
		case WM_INITDIALOG:
			{
				_this->uiObject = (UIOBJECT *)lp;
				_this->OnInitInternal();
			}
			break;
		case WM_COMMAND:
			_this->OnCommand(wp,lp);
			break;
		case WM_PAINT:
			_this->OnPaint();
			break;
		case WM_TIMER:
			_this->OnTimerTick((int4)wp);
			break;
		case WM_CLOSE:
			_this->OnClose();
			break;
		}


		return 0;
	}

private:
	HANDLE initCompletedEvent;
	UIOBJECT *uiObject;
	WINDOWCREATIONINFO wci;
	int4 dlgId;

private:

	bool SetControlEnableState(uint4 ctrlId, bool state)
	{
		HWND ctrlHwnd = GetDlgItem(this->uiObject->hwnd,ctrlId);

		if (!ctrlHwnd)
			return false;

		return (bool)EnableWindow(ctrlHwnd,(BOOL)state);
	}

	void InitCommon(int4 dlgId, bool center)
	{
		this->initCompletedEvent = CreateEvent(NULL,FALSE,FALSE,NULL);

		this->dlgId = dlgId;

		this->wci.pci = NULL;

		if (center)
		{
			this->wci.pci = ALLOCOBJECT(PRECREATEWINDOWINFO);
			this->wci.pci->wri.flag = WRIF_CENTER;
		}

	}

	void OnInitInternal()
	{
		this->OnInit();

		SetEvent(this->initCompletedEvent);
	}

public:

	UiWrapper(int4 dlgId, bool center)
	{
		InitCommon(dlgId,center);
	}

	UiWrapper(int4 dlgId)
	{
		InitCommon(dlgId,false);
	}

	~UiWrapper(void)
	{
		CloseHandle(this->initCompletedEvent);
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
			&this->wci);

		return this->uiObject != NULL;
	}

	HWND GetHWND() const
	{
		return this->uiObject->hwnd;
	}

	bool SetTimer(int4 timerId,uint4 period)
	{
		return ::SetTimer(this->uiObject->hwnd,(UINT_PTR)timerId,(UINT)period,NULL) 
			== timerId;
	}

	int4 MessageBox(wnstring msg, wnstring title, uint4 flags)
	{
		return ::MessageBoxW(this->uiObject->hwnd,(LPCWSTR)msg,(LPCWSTR)title,flags);
	}

	void WaitForInitCompletion()
	{
		WaitForSingleObject(this->initCompletedEvent,INFINITE);
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

	void SetWindowTitle(wnstring title)
	{
		SetWindowTextW(this->uiObject->hwnd,title);
	}

	bool SetControlTextA(uint4 ctrlId, anstring str)
	{
		bool ret;
		wnstring wstr;

		wstr = ffhelper::Helper::AnsiToWideString(str);

		ret = SetControlText(ctrlId,wstr);

		FREESTRING(wstr);

		return ret;
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

	template <class T> T *GetControlById(uint4 ctrlId)
	{
		T *ctrl = new T(this->uiObject, ctrlId);
		
		ctrl->OnInitControl();

		return ctrl;
	}

	virtual void OnTimerTick(int4 timerId)
	{

	}

	virtual void OnClose()
	{
	}

	virtual void OnPaint()
	{
	}

	virtual void OnCommand(WPARAM wp, LPARAM lp)
	{
	}

	virtual void OnInit()
	{
	}
};

