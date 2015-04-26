#pragma once

#include "stdafx.h"
#include "UiWrapper.h"
#include "FileFolderDialog.h"
#include "Settings.h"
#include "lang\LanMan.h"
#include "ctrl\UiProgressbar.h"
#include "ctrl\UiComboBox.h"
#include "..\ffmpeg\MediaInfo.h"
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

FORWARDED LanguageManager * gs_LanMan;

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

		SetWindowTitle(LANGSTR("FSL_UI_PSTVALUE_TITLE"));

		SetControlText(IDC_LBL_PV_VBIT,LANGSTR("FSL_UI_PSTVALUE_VBIT_STATIC"));
		SetControlText(IDC_LBL_PV_ABIT,LANGSTR("FSL_UI_PSTVALUE_ABIT_STATIC"));
		SetControlText(IDC_LBL_PV_STARTTIME,LANGSTR("FSL_UI_PSTVALUE_STARTTIME_STATIC"));
		SetControlText(IDC_LBL_PV_DURLEN,LANGSTR("FSL_UI_PSTVALUE_DURLEN_STATIC"));
		SetControlText(IDC_BTNOK,LANGSTR("FSL_UI_GEN_OK_BUTTON"));

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

		status = new AutoString<wchar>();
		return true;
	}

	void AddStatusLine(wnstring status)
	{
		this->status->AppendFormat(L"%s\r\n",status);
		SetControlText(IDC_TXTCOMPILELOG,this->status->c_str());
	}

	void OnInit()
	{
		this->SetWindowTitle(LANGSTR("FSL_UI_COMPILE_TITLE"));
		this->SetControlText(IDC_BTNDCOK,LANGSTR("FSL_UI_GEN_OK_BUTTON"));
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

	void OnInit()
	{
		this->SetWindowTitle(LANGSTR("FSL_UI_SETTING_TITLE"));
		this->SetControlText(IDC_LBLSETTING_BINDIR, LANGSTR("FSL_UI_SETTING_FFMPEG_DIR_STATIC"));
		this->SetControlText(IDC_LBLSETTING_LANG,LANGSTR("FSL_UI_SETTING_CURRENT_LANG_STATIC"));
		this->SetControlText(IDC_BTNBROWSE,LANGSTR("FSL_UI_SETTING_BROWSE_BUTTON"));
		this->SetControlText(IDC_BTN_MIOK,LANGSTR("FSL_UI_GEN_OK_BUTTON"));
	}

};



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

	void UpdateProgress(uint4 pos)
	{
		if (pos == ULONG_MAX)
			this->pbar->Finish();
		else
			this->pbar->SetPos(pos);
	}

	void SetProgressStatusText(wnstring status)
	{
		SetControlText(IDC_LBLPROGR_STAT,status);
	}

	void SetProgressMax(uint4 max)
	{
		this->pbar->SetRange(0,max);
	}


	void OnCommand(WPARAM wp, LPARAM lp)
	{
	}

	void OnInit()
	{
		this->pbar = GetControlById<UiProgressbar>(IDC_PROGR_PBAR);
		this->pbar->SetRange(0,100);

		SetControlText(IDC_BTNPROGCANCEL,LANGSTR("FSL_UI_PROGRESSDLG_CANCEL_BUTTON"));
	}
};

#include "..\ffmpeg\ThumbnailExtractor.h"

class MediaInfoDlg : public UiWrapper
{
private:
	MediaInfo *mediaInfo;
	ThumbnailExtractor *thumbExtract;
	wnstring fileName;
	bool mediaLoadDone;
	int4 thumbIndex;
	UiComboBox *cbStreamIds;
	UiProgressbar *pbLoad;

	void LoadThumbnail(bool wmPaint)
	{
		HDC dc,softDc;
		HGDIOBJ prev;
		RECT rcThumBox;

		if (!this->mediaLoadDone)
			return;

		HWND pictHwnd = GetDlgItem(GetHWND(),IDC_PICT_MITHUMB);
		dc = GetDC(pictHwnd);

		softDc = CreateCompatibleDC(dc);

		if (!wmPaint)
		{
			if (this->thumbIndex >= 4)
				this->thumbIndex = 0;
			else
				this->thumbIndex++;
		}

		prev = SelectObject(softDc,this->thumbExtract->GetFrameBitmapByFrameId(this->thumbIndex));

		GetWindowRect(pictHwnd,&rcThumBox);
		
		StretchBlt(dc,
			0,0,
			rcThumBox.right-rcThumBox.left,
			rcThumBox.bottom - rcThumBox.top,
			softDc,
			0,0,
			256,
			192,
			SRCCOPY);

		DeleteDC(softDc);
		
	}

	void UpdateInfoPanel(int4 streamIndex)
	{
		wchar buf[256];
		StreamInfo sinfo;
		AutoStringW str;

		if (!this->mediaInfo->GetStreamInfo(&sinfo,streamIndex))
			return;

		memset(buf,0,sizeof(buf));
		ffhelper::Helper::LazyMbToWcs(sinfo.codecName,buf);
		str.AppendFormat(LANGSTR("FSL_UI_MEDIAINFO_CODECNAME", buf));
		str += L"\r\n";
		memset(buf,0,sizeof(buf));

		ffhelper::Helper::LazyMbToWcs(sinfo.codecType,buf);
		str.AppendFormat(LANGSTR("FSL_UI_MEDIAINFO_CODECTYPE",buf));
		str += L"\r\n";
		memset(buf,0,sizeof(buf));

		ffhelper::Helper::LazyMbToWcs(sinfo.codecTag,buf);
		str.AppendFormat(LANGSTR("FSL_UI_MEDIAINFO_CODECTAG",buf));
		str += L"\r\n";
		memset(buf,0,sizeof(buf));

		ffhelper::Helper::LazyMbToWcs(sinfo.aspectRatio,buf);
		str.AppendFormat(LANGSTR("FSL_UI_MEDIAINFO_ASPECTRAT",buf));
		str += L"\r\n";
		memset(buf,0,sizeof(buf));

		str.AppendFormat(LANGSTR("FSL_UI_MEDIAINFO_DURATION",
			sinfo.duration.hours,sinfo.duration.minutes,sinfo.duration.seconds));
		str += L"\r\n";

		str.AppendFormat(LANGSTR("FSL_UI_MEDIAINFO_SIZE", sinfo.width,sinfo.height));
		str += L"\r\n";

		str.AppendFormat(LANGSTR("FSL_UI_MEDIAINFO_BITRATE", sinfo.bitRate));
		str += L"\r\n";

		str.AppendFormat(LANGSTR("FSL_UI_MEDIAINFO_NUMOFFRAMES", sinfo.numberOfFrames));
		str += L"\r\n";

		str.AppendFormat(LANGSTR("FSL_UI_MEDIAINFO_SAMPRATE", sinfo.sampleRate));
		str += L"\r\n";

		str.AppendFormat(LANGSTR("FSL_UI_MEDIAINFO_CHANNELS",sinfo.channels));
		str += L"\r\n";

		SetControlText(IDC_LBL_MIINFOPANEL,str.c_str());
	}

	static DWORD WINAPI MediaInfoGrabWorker(LPVOID arg)
	{
		int4 i;
		wchar indexSbuf[5];
		ffmpegTime ftime;
		MediaInfoDlg *_this = (MediaInfoDlg *)arg;

		_this->SetControlText(IDC_LBL_MILOADING,L"Loading...");
		_this->pbLoad->SetRange(0,5);

		_this->mediaInfo = new MediaInfo(_this->fileName);
		_this->thumbExtract = new ThumbnailExtractor(_this->fileName);
		
		if (!_this->mediaInfo->GetStreamCount())
		{
			_this->MessageBox(LANGSTR("FSL_MSG_NO_STREAM_DETECTED"),NULL, MB_OK | MB_ICONWARNING);
			return 0;
		}

		_this->mediaInfo->GetMediaDuration(&ftime);
		_this->pbLoad->Increment();

		_this->thumbExtract->ExtractFrame(4,&ftime);
		
		_this->pbLoad->Add(4);

		for (i = 0; i < _this->mediaInfo->GetStreamCount(); i++)
		{
			wsprintf(indexSbuf,L"%d",i);
			_this->cbStreamIds->AddItem(indexSbuf);
		}

		_this->cbStreamIds->SetSelectedIndex(0);

		_this->UpdateInfoPanel(0);

		_this->SetTimer(0xDEAD,800);

		_this->HideLoadingControls();
		_this->mediaLoadDone = true;

		return 0;
	}


	void HideLoadingControls()
	{
		ShowWindow(GetDlgItem(GetHWND(),IDC_LBL_MILOADING),SW_HIDE);
		ShowWindow(GetDlgItem(GetHWND(),IDC_PB_MILOADING),SW_HIDE);
	}

public:

	MediaInfoDlg() : UiWrapper(IDD_DLGMEDIA_INFO,true)
	{
		this->mediaInfo = NULL;
		this->thumbIndex = 0;
		this->mediaLoadDone = false;
	}

	bool ShowDialog(wnstring fileName)
	{
		this->fileName = ALLOCSTRINGW(MAX_PATH);
		wcscpy(this->fileName,fileName);
		return UiWrapper::ShowDialog();
	}

	void OnCommand(WPARAM wp, LPARAM lp)
	{
		int4 newIndex;

		switch (LOWORD(wp))
		{
		case IDC_CB_MISTREAMID:
			{
				if (HIWORD(wp) == CBN_SELCHANGE)
				{
					newIndex = this->cbStreamIds->GetSelectedIndex();
					UpdateInfoPanel(newIndex);
				}
			}
			break;
		case IDC_BTN_MIOK:
			{
				Close();
			}
			break;
		}
	}

	void OnTimerTick(int4 timerId)
	{
		LoadThumbnail(false);
	}

	void OnPaint()
	{
		LoadThumbnail(true);
	}

	void OnInit()
	{		
		ffhelper::Threaded::Start(MediaInfoGrabWorker,this);

		SetWindowTitle(LANGSTR("FSL_UI_MEDIAINFO_TITLE"));
		SetControlText(IDC_LBL_MISTREAMID,LANGSTR("FSL_UI_MEDIAINFO_STREAMINDEX_STATIC"));

		this->pbLoad = GetControlById<UiProgressbar>(IDC_PB_MILOADING);
		this->cbStreamIds = GetControlById<UiComboBox>(IDC_CB_MISTREAMID);

	}

};