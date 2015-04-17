#pragma once

#include "..\stdafx.h"
#include "..\ffmpegProcess.h"
#include "MediaInfo.h"
#include "..\Preset.h"
#include "..\FileList.h"

#define CMVS_VARSIGN		0x00000001
#define CMVS_IDENT			0x00000002
#define CMVS_BROP			0x00000004
#define CMVS_BRCL			0x00000008

#define CMVT_INF			0x00000001
#define CMVT_OUTF			0x00000002
#define CMVT_OPT_SS			0x00000003
#define CMVT_OPT_T			0x00000004
#define CMVT_OPT_TO			0x00000005

#define TPP_INVALID			-1
#define TPP_HOUR			0
#define TPP_MINUTE			1
#define TPP_SECOND			2
#define TPP_MILLISECOND		3

#define TPP_NEXT_PART(x)	((*x)++)

typedef struct
{
	LinkedList<wchar *> *varExt;
	byte type;
	int4 bpos;
	int4 epos;
	vptr lfo; 
}__cmd_var;

static __forceinline __cmd_var *_alloc_cvar()
{
	__cmd_var *cvar = ALLOCOBJECT(__cmd_var);

	if (cvar == NULL)
		return NULL;

	cvar->varExt = new LinkedList<wchar *>();

	return cvar;
}


#define reset_state() cvar->bpos = 0; \
	bufPtr = buf; \
	flag = 0; \
	memset(buf,0,sizeof(buf)) \

#define reset_buf() bufPtr = buf; \
	memset(buf,0,sizeof(buf)) \

#define insert_varlist() { \
	wchar *val = ALLOCSTRINGW(8); \
	wcscpy(val,buf); \
	cvar->varExt->Insert(val); \
 } \


static __forceinline byte InfToCmvt(InputVariableField inf)
{
	switch (inf)
	{
	case IvfStartTime:
		return CMVT_OPT_SS;
	case IvfDuration:
		return CMVT_OPT_T;
	case IvfPosition:
		return CMVT_OPT_TO;
	}

	return 0;
}

static __forceinline InputVariableField CmvtToInf(byte cmvt)
{
	switch (cmvt)
	{
	case CMVT_OPT_SS:
		return IvfStartTime;
	case CMVT_OPT_T:
		return IvfDuration;
	case CMVT_OPT_TO:
		return IvfPosition;
	}

	return IvfNone;
}

static __forceinline bool __try_set_vartype(wchar *buf, __cmd_var *cvar)
{
	cvar->type = 0;

	if (!wcsnicmp(buf,L"INF\0",4))
		cvar->type = CMVT_INF;
	else if (!wcsnicmp(buf,L"OUTF\0",5))
		cvar->type = CMVT_OUTF;
	else if (!wcsnicmp(buf,L"SS\0",3))
		cvar->type = CMVT_OPT_SS;
	else if (!wcsnicmp(buf,L"TO\0",3))
		cvar->type = CMVT_OPT_TO;
	else if (!wcsnicmp(buf,L"T\0",2))
		cvar->type = CMVT_OPT_T;

	return cvar->type > 0;
}

static __forceinline int4 iscmdspecialchr(wchar chr)
{
	return chr == L'"' || chr == L'.';
}

static int cvarArrayComparer(__cmd_var *v1, __cmd_var *v2)
{
	return v1->bpos - v2->bpos;
}

wnstring g_wtokDuplPtr=NULL;

inline wnstring wcstok2(wnstring str, const wnstring delim)
{
	wnstring s,tok;

	if (str == NULL && delim == NULL)
	{
		if (g_wtokDuplPtr != NULL)
		{
			FREESTRING(g_wtokDuplPtr);
			g_wtokDuplPtr=NULL;
		}

		return NULL;
	}

	if (str != NULL)
	{
		s = ALLOCSTRINGW(wcslen((const wchar_t *)str));
		g_wtokDuplPtr = s;
		wcscpy((wchar_t *)s,(const wchar_t *)str);

		tok = wcstok((wchar_t *)s,(const wchar_t *)delim);
	}
	else
		tok = wcstok(NULL,(const wchar_t *)delim);

	return tok;
}

class CommandExecutor
{
private:

	typedef struct
	{
		wnstring cmdLine;
		ffmpegTime ss;
		ffmpegTime t;
		ffmpegTime to;
	}ffmpegCommandInfo;

	ffmpegTime sourceTimeLength;
	ffmpegCommandInfo cmdInfo;
	ffmpegProgressWindow *progressWnd;

	static bool SetTimePart(const wnstring valstr, int4 part, ffmpegTime *time)
	{
		wchar *lastChrPtr = NULL;
		uint4 val = (uint4)wcstoul((const wchar_t *)valstr,&lastChrPtr,10);
		
		if (val == ULONG_MAX && errno == ERANGE)
			return false;

		if (!val)
		{
			//make sure the conversion really failed.
			if (lastChrPtr && *lastChrPtr == L'\0')
				goto setPart;

			return false;
		}

	setPart:
		*( ((uint4 *)time) + part ) = val;

		return true;
	}

	static bool ParseProcessedTime(wnstring str, const wnstring initiator, ffmpegTime *time)
	{
		wnstring tok;
		const wnstring delim = L" =:.";
		int4 part=TPP_INVALID;

		tok = wcstok2(str,delim);
		
		if (initiator == NULL)
			part=TPP_HOUR;

		while (tok != NULL)
		{
			if (part > TPP_INVALID)
			{
				if (part > TPP_MILLISECOND)
				{
					wcstok2(NULL,NULL);
					return true;
				}

				if (!SetTimePart(tok,part,time))
				{
					wcstok2(NULL,NULL);

					//if part number is TPP_MINUTE
					//it means that the time/duration value just provided in seconds.
					//we must set seconds field from the hours
					if (part == TPP_MINUTE)
					{
						time->seconds = time->hours;
						time->hours=0;
						return true;
					}

					//when part number is TPP_MILLISECOND,
					//it means that the millisecond part was not provided. 
					//so we can return immediately with success
					return part == TPP_MILLISECOND;
				}

				TPP_NEXT_PART(&part);
			}
			else if (!wcscmp(tok,(const wchar_t *)initiator))
				TPP_NEXT_PART(&part);

			tok = wcstok2(NULL,delim);
		}


		if (part == TPP_MINUTE)
		{
			time->seconds = time->hours;
			time->hours=0;
		}

		wcstok2(NULL,NULL);
		return part != TPP_INVALID;
	}

	static void OnStdoutLineReceived(vptr arg, LPCSTR line)
	{
		uint4 len;
		ffmpegTime currTime;

		wnstring wline;

		CommandExecutor *_this = (CommandExecutor *)arg;
		

		//TODO: Dont forget to remove that ugly & quick code
		len = strlen(line);
		wline = ffhelper::Helper::AnsiToWideString((anstring)line);

		
		if (ParseProcessedTime(wline,L"time",&currTime))
		{
			memset(wline,0,sizeof(wchar) * len);
			
			wsprintf(wline,L"TIME: %d hr. %d min. %d sec",
				currTime.hours,currTime.minutes,currTime.seconds);

			_this->progressWnd->SetControlText(IDC_LBLPROGR_STAT,(wnstring)wline);
		}

		FREESTRING(wline);
	}

	void ParseCommandVariable(
		wchar *cmdLine, 
		LinkedList<__cmd_var *> *varList,
		uint4 *outfCount)
	{
		wchar *p = cmdLine;
		wchar buf[8]={0};
		int4 flag=0;

		int4 i=0;

		__cmd_var *cvar = _alloc_cvar();
		wchar *bufPtr = buf;
		
		for (; *p; p++, i++)
		{
			if (flag & CMVS_VARSIGN)
			{
				//is a standart or a special delim char?
				if (iswspace(*p) || iscmdspecialchr(*p))
				{
					if (buf[0]>0)
					{
						//if the buffer charged, try to set as variable
						if (__try_set_vartype(buf,cvar))
						{
							if (cvar->type == CMVT_OUTF)
								(*outfCount)++;

							cvar->epos = i;
							varList->Insert(cvar);
							cvar = _alloc_cvar();
						}

						reset_buf();

						//If variable token value is not $INF we can finalize 
						//for the current variable token. because $INF can take 
						//additional extensions
						//TODO: Make a decision for OUTF situation. Hmm
						if (cvar->type != CMVT_INF)
							reset_state();
					}

					continue;
				}

				if (*p == L'[')
				{
					if (flag != CMVS_VARSIGN)
					{
						reset_state();
						continue;
					}

					if (!__try_set_vartype(buf,cvar))
					{
						reset_state();
						continue;
					}

					flag |= (CMVS_IDENT | CMVS_BROP);

					reset_buf();
				}
				else if (*p == L']')
				{
					if (!(flag & CMVS_BROP))
					{
						reset_state();
						continue;
					}

					if (buf[0] != 0)
					{
						insert_varlist();
						reset_buf();
					}

					if (cvar->type == CMVT_OUTF)
						(*outfCount)++;
					
					varList->Insert(cvar);
					cvar->epos = i+1;
					cvar = _alloc_cvar();
					flag = 0;
				}
				else if (*p == L',')
				{
					if (!(flag & CMVS_BROP))
					{
						reset_state();
						continue;
					}

					if (buf[0] != 0)
					{
						insert_varlist();
						reset_buf();
					}
				}
				else
				{
					if (buf+sizeof(buf) == bufPtr)
					{
						reset_state();
					}
					else
						*bufPtr++ = *p;
				}
			}
			else if (*p == L'$')
			{
				if (flag > 0)
				{
					reset_state();
					continue;
				}

				cvar->bpos = i;
				flag |= CMVS_VARSIGN;
			}
		}

		if (buf[0] != 0)
		{
			if (flag & CMVS_VARSIGN)
			{
				if (!(flag & CMVS_IDENT))
				{
					cvar->epos = i;

					if (__try_set_vartype(buf,cvar))
					{
						if (cvar->type == CMVT_OUTF)
							(*outfCount)++;
						
						varList->Insert(cvar);
					}
				}
			}
		}

	}

	LinkedListNode<__cmd_var *> *GetCmdVarByFileExtension(
		const wchar *ext, 
		LinkedList<__cmd_var *> *varList)
	{
		__cmd_var *cvar;
		
		//PS: We dont need any hash table or map lookup implementation.
		//The Command probably has approx. 2-5 cmd vars. Linear lookup fits for it.

		LL_FOREACH(__cmd_var *,node,varList)
		{
			cvar = node->GetValue();

			if (cvar->type == CMVT_OUTF)
				continue;

			LL_FOREACH(wchar *,extNode, cvar->varExt)
			{
				if (!wcsicmp(extNode->GetValue(),ext))
				{
					return node;
				}
			}
		}

		return NULL;
	}

	void AskUserVariable(
		DynamicArray<__cmd_var *> *varList, 
		DynamicArray<__cmd_var *> *userInputVarList,
		InputVariableField fields)
	{
		__cmd_var *cvar;
		wnstring str;

		ffmpegTime *optTime = NULL;
		ffmpegVariableInputDialog varDlg(fields);
		varDlg.ShowDialog();

		
		while (userInputVarList->GetCount() > 0)
		{
			str = ALLOCSTRINGW(255);
			cvar = (*userInputVarList)[0];
			varDlg.GetFieldString(str,255,CmvtToInf(cvar->type));
			cvar->lfo = str;

			switch (cvar->type)
			{
			case CMVT_OPT_SS:
				optTime = &this->cmdInfo.ss;
				break;
			case CMVT_OPT_T:
				optTime = &this->cmdInfo.t;
				break;
			case CMVT_OPT_TO:
				optTime = &this->cmdInfo.to;
				break;
			}

			if (optTime != NULL)
			{
				ParseProcessedTime(str,NULL,optTime);
			}

			varList->Add(cvar);
			userInputVarList->Remove(0);
		}
	}

	void TryDetectStaticTimeDuration(wnstring cmd, ffmpegCommandInfo *cmdInfo)
	{
		wnstring pTok;
		pTok = wcstok2(cmd,L" ");

		ffmpegTime *optTime = NULL;

		while (pTok != NULL)
		{
			if (optTime != NULL)
			{
				//string to ffmpegTime
				ParseProcessedTime(pTok,NULL,optTime);
				goto contiTokeniz;
			}

			if (!wcsicmp(pTok,L"-ss"))
			{
				optTime = &cmdInfo->ss;
			}
			else if (!wcsicmp(pTok,L"-to"))
			{
				optTime = &cmdInfo->to;
			}
			else if (!wcsicmp(pTok,L"-t"))
			{
				optTime = &cmdInfo->t;
			}

	contiTokeniz:
			pTok = wcstok2(NULL,L" ");
		}

		wcstok2(NULL,NULL);
	}

	wnstring GenerateActualFFmpegCommand(
		const wchar *cmd, 
		DynamicArray<__cmd_var *> *varList, 
		DynamicArray<__cmd_var *> *userInputVarList,
		InputVariableField fields)
	{
		__cmd_var *var;
		FILEPATHITEM *fileItem,*outFileInfo;
		wchar *replStr = ALLOCSTRINGW(MAX_PATH);
		AutoStringW str(cmd);
		uint4 shiftLen=0, replStrLen=0,copyPos=0,copyLen=0;

		//request input first
		AskUserVariable(varList,userInputVarList,fields);
		
		for (uint4 i=0; i<varList->GetCount();i++)
		{
			var = (*varList)[i];

			fileItem = (FILEPATHITEM *)var->lfo;

			//FIX: Make dynamic
			if (var->type == CMVT_OUTF)
			{
				//TODO: Try correction
				if (outFileInfo == NULL)
					return NULL;

				replStrLen = FlGeneratePathString2(
					outFileInfo,
					replStr,
					MAX_PATH,
					FL_FLAG_ZERO_BUFFER | FL_FLAG_QUOTE_SURROUNDED,
					PAS_OBJECTNAME,
					L"output");
			}
			else if (var->type == CMVT_INF)
			{
				if (fileItem == NULL)
				{
					//There is no linked fileItem for the current variable
					return NULL;
				}

				//we'll using the first input filename to generating output fname.
				outFileInfo = fileItem;

				replStrLen = FlGeneratePathString2(
					fileItem,
					replStr,
					MAX_PATH,
					FL_FLAG_ZERO_BUFFER | FL_FLAG_QUOTE_SURROUNDED,
					PAS_NONE,
					NULL);
			}
			else
			{
				//request duration input from the users

				replStr = (wchar *)var->lfo;
				replStrLen = wcslen(replStr);
			}

			var->bpos += shiftLen;
			var->epos += shiftLen;

 			str = str.Replace(var->bpos,var->epos,replStr);

			shiftLen += replStrLen - (var->epos-var->bpos);

			ZEROSTRINGW(replStr,MAX_PATH);
		}

		return str.GetNativeStringWithNoDestroy();
	}

public:

	CommandExecutor()
	{
		memset(&this->cmdInfo,0,sizeof(ffmpegCommandInfo));
		memset(&this->sourceTimeLength,0,sizeof(ffmpegTime));
	}

	void Execute(PRESET *preset, FileList *fileList)
	{
		wnstring ffmpegCmd = NULL;
		ffmpegProcess *process = NULL;
		LinkedListNode<__cmd_var *> *cvarNode = NULL,*tmpNode=NULL;
		__cmd_var *cvar;
		uint4 outfCount=0;
		bool linkToVar=false;
		InputVariableField ivf = IvfNone;

		LinkedList<__cmd_var *> *varList;
		DynamicArray<__cmd_var *> processedVarList(
			(DynamicArray<__cmd_var *>::DYNAMIC_ARRAY_SORT_COMPARER)cvarArrayComparer
			);

		DynamicArray<__cmd_var *> userInputVarList(
			(DynamicArray<__cmd_var *>::DYNAMIC_ARRAY_SORT_COMPARER)cvarArrayComparer
			);


		varList = new LinkedList<__cmd_var *>();
		
		ParseCommandVariable(preset->command,varList,&outfCount);

		if (!outfCount)
		{
			//Where the fuck is the output constant?
			//raise error
			goto cleanUp;
		}

		if (outfCount > 1)
		{
			//Wait. what?
			goto cleanUp;
		}

		LL_FOREACH(FILEPATHITEM *,node,fileList->GetList())
		{
			cvarNode = GetCmdVarByFileExtension(node->GetValue()->objectExtension,varList);


			if (cvarNode == NULL)
			{
				goto cleanUp;
				//Raise error
			}
			
			//Ok. i found it. Link the variable to the file object
			cvarNode->GetValue()->lfo = node->GetValue();

			//And detach varlist and add to the processed list.
			//we dont need to re-iterate them.
			varList->DetachNode(cvarNode);
			processedVarList.Add(cvarNode->GetValue());
			delete cvarNode;

		}

		//Try to get user input variable infos
		LL_FOREACH(__cmd_var *,cvNode,varList)
		{
beginAgain:
			cvar = cvNode->GetValue();
			
			if (cvar->type == CMVT_OPT_SS)
				ivf |= IvfStartTime;
			else if (cvar->type == CMVT_OPT_TO)
				ivf |= IvfPosition;
			else if (cvar->type == CMVT_OPT_T)
				ivf |= IvfDuration;
			else
				continue;

			//update foreach node for detachment operation
			tmpNode = cvNode;
			
			//remove list and add cvar value to the user input list
			tmpNode = cvNode->Previous();

			varList->DetachNode(cvNode);
			userInputVarList.Add(cvar);

			delete cvNode;

			if (tmpNode != NULL)
				cvNode = tmpNode;
			else
			{
				//if detached node is head we dont 
				//want iterate to next. 
				cvNode = varList->Begin();
				goto beginAgain;
			}
			
		}

		//still exists a variable except OUTF?

		if (varList->GetCount() > 1 )
		{
			goto cleanUp;
			//Raise error : still exists mismatched inf variable.
		}


		cvarNode = varList->Begin();
		varList->DetachNode(cvarNode);
		processedVarList.Add(cvarNode->GetValue());
		delete cvarNode;

		if (userInputVarList.GetCount() == 0)
		{
			//try to grab hardcoded time/duration info for progressbar
			TryDetectStaticTimeDuration(ffmpegCmd,&this->cmdInfo);
		}

		ffmpegCmd = GenerateActualFFmpegCommand(
			preset->command,
			&processedVarList,
			&userInputVarList,
			ivf);

		this->cmdInfo.cmdLine = ffmpegCmd;

		fileList->Release();

		process = new ffmpegProcess(FFMPEG);

		this->progressWnd = new ffmpegProgressWindow();
		this->progressWnd->ShowDialog();

		this->progressWnd->WaitForInitCompletion();

		process->OnLineReceived = CommandExecutor::OnStdoutLineReceived;
		process->SetArg(ffmpegCmd);
		process->Start(this);
		process->Wait(true);
		
		this->progressWnd->Close();

		delete this->progressWnd;
		this->progressWnd = NULL;

cleanUp:
		delete varList;

		delete process;

		if (ffmpegCmd != NULL)
			FREESTRING(ffmpegCmd);
	}
};
