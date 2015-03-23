#pragma once

#include "stdafx.h"
#include "UiWrapper.h"


class CompileDialog : public UiWrapper
{
private:
	AutoString<wchar> *status;
public:
	CompileDialog() : UiWrapper(IDD_DLGCOMPILE)
	{
	}
	
	void OnCommand(WPARAM wp, LPARAM lp)
	{
		switch (LOWORD(wp))
		{
		case IDC_BTNDCOK:
			Close();
			break;
		}
	}

	void OnClose()
	{
		delete this->status;
	}

	bool ShowDialog()
	{
		if (!UiWrapper::ShowDialog())
			return false;

		//DisableControl(IDC_BTNDCOK);
		status = new AutoString<wchar>();
		return true;
	}

	void AddStatusLine(wnstring status)
	{
		this->status->AppendFormat(L"%s\r\n",status);
		SetControlText(IDC_TXTCOMPILELOG,this->status->GetNativeString(false));
	}
};


class SettingsDlg : public UiWrapper
{
public:
	
	SettingsDlg() : UiWrapper(IDD_DLGSETTINGS)
	{
	}

	void OnCommand(WPARAM wp, LPARAM lp)
	{
	}
};