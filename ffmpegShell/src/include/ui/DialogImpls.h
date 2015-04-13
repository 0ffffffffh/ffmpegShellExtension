#pragma once

#include "stdafx.h"
#include "UiWrapper.h"
#include "FileFolderDialog.h"
#include "Settings.h"
#include <map>

typedef enum 
{
	IvfNone				= 0,
	IvfVideoBitrate		= 1,
	IvfAudioBitrate		= 2,
	IvfStartTime		= 4,
	IvfPosition			= 8,
	IvfDuration			= 16
}InputVariableField;

inline InputVariableField& operator |=(InputVariableField &a, InputVariableField b)
{
	return a = ( (InputVariableField) ( ((int)a) | ((int)b) ) );
}

class ffmpegVariableInputDialog : public UiWrapper
{
private:
	typedef std::map<InputVariableField,wnstring> __FieldMap;


	InputVariableField fields;
	__FieldMap fMap;

	void StoreToMap(InputVariableField ivf, int4 cid)
	{
		wnstring value;

		if (fMap.find(ivf) != fMap.end())
			return;

		value = ALLOCSTRINGW(255);

		if (GetControlText(cid,value,255)>0)
			fMap.insert(std::make_pair(ivf,value));
		else
			FREESTRING(value);
	}

	void StoreStringsForAvailableFields()
	{
		if (this->fields & IvfAudioBitrate)
			StoreToMap(IvfAudioBitrate,IDC_TXTABIT);
		
		if (this->fields & IvfVideoBitrate)
			StoreToMap(IvfVideoBitrate,IDC_TXTVBIT);
		
		if (this->fields & IvfStartTime)
			StoreToMap(IvfStartTime,IDC_TXTSTARTTIME);
		
		if (this->fields & IvfDuration)
			StoreToMap(IvfDuration,IDC_TXTDURATION);
		
		if (this->fields & IvfPosition)
			StoreToMap(IvfPosition,IDC_TXTDURATION);
	}
	
public:
	ffmpegVariableInputDialog(InputVariableField fields) : UiWrapper(IDD_DLGPRESETVALUE,true)
	{
		this->fields = fields;
	}

	~ffmpegVariableInputDialog()
	{
		for (__FieldMap::iterator it = this->fMap.begin();
			it != this->fMap.end();
			it++)
		{
			FREESTRING(it->second);
		}

		this->fMap.clear();
	}

	void OnCommand(WPARAM wp, LPARAM lp)
	{
		switch (LOWORD(wp))
		{
		case IDC_BTNOK:
			{
				StoreStringsForAvailableFields();
				Close();
				break;
			}
		}
	}

	void OnInit()
	{
		if (this->fields & IvfAudioBitrate)
			EnableControl(IDC_TXTABIT);
		
		if (this->fields & IvfVideoBitrate)
			EnableControl(IDC_TXTVBIT);
		
		if (this->fields & IvfStartTime)
			EnableControl(IDC_TXTSTARTTIME);
		
		if (this->fields & IvfDuration)
			EnableControl(IDC_TXTDURATION);
		
		if (this->fields & IvfPosition)
			EnableControl(IDC_TXTDURATION);
	}

	bool ShowDialog()
	{
		return UiWrapper::ShowDialog(false);
	}

	uint4 GetFieldString(wnstring strBuf, uint4 bufSize,InputVariableField field)
	{
		__FieldMap::iterator it = this->fMap.find(field);
		uint4 len;

		if (it == this->fMap.end())
			return 0;

		len = wcslen(it->second);

		if (len+1 > bufSize)
			return 0;

		memset(strBuf,0,bufSize);
		wcscpy(strBuf,it->second);

		return len;
	}
};

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

#include "ctrl\UiProgressbar.h"

class ffmpegProgressWindow : public UiWrapper
{
private:
	UiProgressbar *pbar;
public:
	ffmpegProgressWindow() : UiWrapper(IDD_DLGPROGRESS,true) 
	{

	}

	~ffmpegProgressWindow()
	{
		delete this->pbar;
	}

	void OnCommand(WPARAM wp, LPARAM lp)
	{
	}

	void OnInit()
	{
		this->pbar = GetControlById<UiProgressbar>(IDC_PROGR_PBAR);
		this->pbar->SetRange(0,100);
	}

};