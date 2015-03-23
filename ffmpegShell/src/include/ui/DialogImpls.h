#pragma once

#include "stdafx.h"
#include "UiWrapper.h"
#include "FileFolderDialog.h"
#include "Settings.h"

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

FORWARDED SETTINGS g_settings;

class SettingsDlg : public UiWrapper
{
public:
	
	SettingsDlg() : UiWrapper(IDD_DLGSETTINGS)
	{
	}

	bool ShowDialog()
	{
		if (!UiWrapper::ShowDialog())
			return false;

		this->SetControlText(IDC_TXTFBP,g_settings.ffmpegBinaryPath);
		return true;
	}

	void OnCommand(WPARAM wp, LPARAM lp)
	{
		DIALOG *dlg;

		switch (LOWORD(wp))
		{
		case IDC_BTNBROWSE:
			{
				if (DlgOpenSpecialDialog(
					NULL,FolderDialogSelect,
					L"Select ffmpeg binary directory",L"C:\\",&dlg))
				{
					wcscpy(g_settings.ffmpegBinaryPath,dlg->path);
					IntCommitSettings();

					this->SetControlText(IDC_TXTFBP,dlg->path);
				}
			}
			break;
		}
	}

};