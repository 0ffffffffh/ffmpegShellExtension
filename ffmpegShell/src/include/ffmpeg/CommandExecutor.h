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

static __forceinline bool __try_set_vartype(wchar *buf, __cmd_var *cvar)
{
	cvar->type = 0;

	if (!wcsnicmp(buf,L"INF\0",4))
		cvar->type = CMVT_INF;
	else if (!wcsnicmp(buf,L"OUTF\0",5))
		cvar->type = CMVT_OUTF;

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

class CommandExecutor
{
private:
	static void OnStdoutLineReceived(vptr arg, LPCSTR line)
	{
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
				if (iswspace(*p) || iscmdspecialchr(*p))
				{
					if (buf[0]>0)
					{
						if (__try_set_vartype(buf,cvar))
						{
							if (cvar->type == CMVT_OUTF)
								(*outfCount)++;

							cvar->epos = i-1;
							varList->Insert(cvar);
							cvar = _alloc_cvar();
						}

						reset_buf();
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

	wnstring GenerateActualFFmpegCommand(const wchar *cmd, DynamicArray<__cmd_var *> *varList)
	{
		__cmd_var *var;
		FILEPATHITEM *fileItem,*outFileInfo;
		wchar *fileName = ALLOCSTRINGW(MAX_PATH);
		AutoStringW str(cmd);
		uint4 shiftLen=0, fNameLen=0,copyPos=0,copyLen=0;

		
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

				fNameLen = FlGeneratePathString2(
					outFileInfo,
					fileName,
					MAX_PATH,
					FL_FLAG_ZERO_BUFFER | FL_FLAG_SURROUND_QUOTES,
					PAS_OBJECTNAME,
					L"output");
			}
			else
			{

				if (fileItem == NULL)
				{
					//There is no linked fileItem for the current variable
					return NULL;
				}

				//we'll using the first input filename to generating output fname.
				outFileInfo = fileItem;

				fNameLen = FlGeneratePathString2(
					fileItem,
					fileName,
					MAX_PATH,
					FL_FLAG_ZERO_BUFFER | FL_FLAG_SURROUND_QUOTES,
					PAS_NONE,
					NULL);
			}

			var->bpos += shiftLen;
			var->epos += shiftLen;

			str = str.Replace(var->bpos,var->epos,fileName);

			shiftLen = fNameLen - (var->epos-var->bpos);

			ZEROSTRINGW(fileName,MAX_PATH);
		}

		return str.GetNativeStringWithNoDestroy();
	}

public:

	void Execute(PRESET *preset, FileList *fileList)
	{
		wnstring ffmpegCmd = NULL;
		ffmpegProcess *process = NULL;
		LinkedListNode<__cmd_var *> *cvarNode = NULL;
		uint4 outfCount=0;

		LinkedList<__cmd_var *> *varList;
		DynamicArray<__cmd_var *> processedVarList(
			(DynamicArray<__cmd_var *>::DYNAMIC_ARRAY_SORT_COMPARER)cvarArrayComparer
			);



		varList = new LinkedList<__cmd_var *>();
		
		process = new ffmpegProcess(FFMPEG);
		process->OnLineReceived = CommandExecutor::OnStdoutLineReceived;

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

		if (varList->GetCount() > 1 )
		{
			goto cleanUp;
			//Raise error : still exists mismatched inf variable.
		}


		cvarNode = varList->Begin();
		varList->DetachNode(cvarNode);
		processedVarList.Add(cvarNode->GetValue());
		delete cvarNode;

		ffmpegCmd = GenerateActualFFmpegCommand(preset->command,&processedVarList);

		fileList->Release();

		process = new ffmpegProcess(FFMPEG);

		process->OnLineReceived = CommandExecutor::OnStdoutLineReceived;
		process->SetArg(ffmpegCmd);
		process->Start(this);
		process->Wait(true);

cleanUp:
		delete varList;

		delete process;

		if (ffmpegCmd != NULL)
			FREESTRING(ffmpegCmd);
		

	}
};
