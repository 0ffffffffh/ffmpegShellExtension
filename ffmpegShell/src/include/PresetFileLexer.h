#pragma once

#include "stdafx.h"
#include <Windows.h>
#include "LinkedList.h"
#include "Memory.h"
#include "FileReadWrite.h"
#include "PresetParser.h"

const byte UNICODE_MARK[] = {0xff,0xfe};

const int STRING_BUF_MAX_LEN = 128;

class PresetFileLexer
{
private:
	LinkedList<TOKENINFO *> *tokenList;
	wchar fileName[MAX_PATH];
	PresetParser *parser;
	FileReadWrite *fileIo;
	COMPILATION_EVENT_HANDLER handler;
	void *handlerArg;
	int currentPos;
	int currentLine;

	wchar ReadChar(bool ansiSourceMode)
	{
		if (this->fileIo->EndOfFile())
			return L'';

		if (ansiSourceMode)
			return (wchar)this->fileIo->ReadAnsiChar();

		return this->fileIo->ReadWideChar();
	}

	bool __forceinline IsEscapeChar(wchar chr)
	{
		switch (chr)
		{
		case '\'':
		case '\"':
		case '\?':
		case '\0':
		case '\a':
		case '\b':
		case '\f':
		case '\r':
		case '\n':
		case '\t':
		case '\v':
			return true;

		}

		return false;
	}

	static void __ListItemDisposer(void *object)
	{
		TOKENINFO *token;

		if (object == NULL)
			return;

		token = (TOKENINFO *)object;

		if (token->data == NULL)
			return;

		FREEOBJECT(token->data);
	}

public:

	

	PresetFileLexer(wnstring presetFile, COMPILATION_EVENT_HANDLER handler, void *arg)
	{
		memset(this->fileName,0,sizeof(this->fileName));
		wcsncpy(this->fileName,presetFile,wcslen(presetFile));
		this->tokenList = new LinkedList<TOKENINFO *>();
		this->tokenList->SetDisposer(__ListItemDisposer);

		this->currentPos=0;
		this->currentLine=1;
		this->handler = handler;
		this->handlerArg = arg;
		this->fileIo = new FileReadWrite(this->fileName,OpenForRead,OpenExisting);
	}

	~PresetFileLexer(void)
	{
		delete this->tokenList;
		delete this->parser;

		delete this->fileIo;
	}

	bool Lex()
	{
		wchar c;
		wchar buffer[STRING_BUF_MAX_LEN] = {0};
		byte magic[2];
		int bufIndex=0;
		bool commentOut=false;
		bool quoted=false;
		TOKENINFO *token = NULL;
		TOKENTYPE tokenType;
		bool ansiMode=false;

		if (!this->fileIo->Open())
			return false;

		if (this->fileIo->Read((byte *)magic,-1,sizeof(magic)) == sizeof(magic))
		{
			if (memcmp(magic,UNICODE_MARK,sizeof(UNICODE_MARK)))
			{
				ansiMode = true;
				this->fileIo->Seek(0,Begin);
			}
		}


		while ((c = ReadChar(ansiMode)) != L'')
		{
			if (c == L'\n')
			{
				this->currentPos=-1;
				this->currentLine++;
			}
			else
				this->currentPos++;

			tokenType = None;

			if (c == L'>')
			{
				commentOut = false;
				continue;
			}

			if (commentOut)
				continue;

			if (c == L'<')
			{
				commentOut = true;
				continue;
			}

			switch (c)
			{
			case L'"':
				{
					quoted = !quoted;
					tokenType = DoubleQuotes;
				}
				break;
			case L'{':
				tokenType = BraceOpen;
				break;
			case L'}':
				tokenType = BraceClose;
				break;
			case L':':
				tokenType = Colon;
				break;
			case L',':
				tokenType = Comma;
				break;
			case L';':
				tokenType = SemiColon;
				break;
			case L'\\':
				tokenType = EscapeChar;
				break;
			case L' ':
				{
					if (!quoted)
						continue;
					else
						tokenType = WhiteSpace;

					break;
				}
			}

			if (tokenType != None)
			{
				//check waiting string token 
				if (bufIndex > 0)
				{
					ASSERT(token != NULL);
					token->type = String;
					token->data = ALLOCARRAY(wchar,bufIndex+1);
					token->pos = this->currentPos;
					token->line = this->currentLine;
					wcsncpy(token->data,buffer,bufIndex);
					memset(buffer,0,sizeof(buffer));
					bufIndex=0;
					tokenList->Insert(token);
					token = NULL;

				}

				token = ALLOCOBJECT(TOKENINFO);
				token->type = tokenType;
				token->pos = this->currentPos;
				token->line = this->currentLine;

				tokenList->Insert(token);
				token = NULL;
			}
			else
			{
				if (!quoted && IsEscapeChar(c))
					continue;
				
				if (bufIndex < sizeof(buffer))
				{
					if (!token)
					{
						token = ALLOCOBJECT(TOKENINFO);
						token->pos = this->currentPos;
						token->line = this->currentLine;
					}

					buffer[bufIndex++] = c;
				}
			}
		}

		this->fileIo->Close();

		this->parser = new PresetParser(this->tokenList,this->handler,this->handlerArg);

		return true;
	}

	PresetParser *GetParser() const
	{
		return this->parser;
	}

};

