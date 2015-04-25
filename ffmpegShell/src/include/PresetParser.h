#pragma once

#include "stdafx.h"
#include "LinkedList.h"
#include "string\AutoString.h"
#include "Preset.h"

//ffmpegShell Preset Script Linear parser

// < comment out begin
// > comment out end

typedef enum
{
	None,
	DoubleQuotes, // "
	String, // String
	BraceOpen, // {
	BraceClose, // }
	Colon, // :
	Comma, // ,
	SemiColon, // ;
	WhiteSpace, // " "
	EscapeChar // backslash
}TOKENTYPE;

typedef enum
{
	KWNOTKEYWORD,
	KWPRESET,
	KWNAME,
	KWCOMMAND,
	KWMEDIATYPE,
	KWOPTYPE,
	KWSRCFORMAT,
	KWDSTFORMAT,
	KWRUNONFINISH
}KEYWORD;

typedef struct 
{
	TOKENTYPE type;
	wchar *data;
	int pos;
	int line;
}TOKENINFO;

typedef struct
{
	PRESET preset;
	KEYWORD waitingPropertyId;
	bool waitingProperty;
	bool waitingPropertyValue;
}PRESETOBJECT;


#define _CL(vl,size) ((vl >= size) ? (size-1) : vl)

typedef void (*COMPILATION_EVENT_HANDLER)(void *, wnstring);

#define NodePtr(node) node->GetValue()

#define NodeNextPtr(node) node->Next()->GetValue()

#define NodePrevPtr(node) node->Previous()->GetValue()


class PresetParser
{
private:
	LinkedList<TOKENINFO *> *tokens;
	LinkedList<PRESETOBJECT *> *presetObjectList;
	COMPILATION_EVENT_HANDLER eventHandler;
	void *handlerArg;

	PRESETOBJECT *PresetParser::ps_PresetObject;

	
	KEYWORD CheckKeyword(TOKENINFO *token)
	{
		if (token->data == NULL)
			return KWNOTKEYWORD;

		if (!wcsicmp(token->data,L"preset"))
			return KWPRESET;
		else if (!wcsicmp(token->data,L"name"))
			return KWNAME;
		else if (!wcsicmp(token->data,L"command"))
			return KWCOMMAND;
		else if (!wcsicmp(token->data,L"mediatype"))
			return KWMEDIATYPE;
		else if (!wcsicmp(token->data,L"operationtype"))
			return KWOPTYPE;
		else if (!wcsicmp(token->data,L"sourceformat"))
			return KWSRCFORMAT;
		else if (!wcsicmp(token->data,L"destinationformat"))
			return KWDSTFORMAT;
		else if (!wcsicmp(token->data,L"runonfinish"))
			return KWRUNONFINISH;

		return KWNOTKEYWORD;
	}

	bool SetObjectPropertyA(PRESETOBJECT *instance, KEYWORD propertyKw, void *value)
	{
		bool result=false;

		wnstring wstr = ffhelper::Helper::AnsiToWideString((anstring)value);

		if (!wstr)
			return false;

		result = SetObjectProperty(instance,propertyKw,wstr);

		FREESTRING(wstr);

		return result;
	}

	bool SetObjectProperty(PRESETOBJECT *instance, KEYWORD propertyKw, void *value)
	{
		uint4 vl;

		//Not a keyword?
		if (!propertyKw)
			return false;

		vl = wcslen((wchar *)value);

		switch (propertyKw)
		{
		case KWNAME:
			wcsncpy(instance->preset.name,(wchar *)value,_CL(vl,64));
			break;
		case KWCOMMAND:
			wcsncpy(instance->preset.command,(wchar *)value,_CL(vl,512));
			break;
		case KWMEDIATYPE:
			{
				if (!wcsicmp((wchar *)value,L"audio"))
					instance->preset.mediaType = Audio;
				else if (!wcsicmp((wchar *)value,L"video"))
					instance->preset.mediaType = Video;
				else
					return false;
			}
			break;
		case KWOPTYPE:
			{
				if (!wcsicmp((wchar *)value,L"conversion"))
					instance->preset.opType = Conversion;
				else if (!wcsicmp((wchar *)value,L"extraction"))
					instance->preset.opType = Extraction;
				else if (!wcsicmp((wchar *)value,L"combination"))
					instance->preset.opType = Combination;
				else
					return false;
			}
			break;
		case KWSRCFORMAT:
			wcsncpy(instance->preset.sourceFormat,(wchar *)value,_CL(vl,6));
			break;
		case KWDSTFORMAT:
			wcsncpy(instance->preset.destinationFormat,(wchar *)value,_CL(vl,6));
			break;
		case KWRUNONFINISH:
			wcsncpy(instance->preset.runOnFinish,(wchar *)value,_CL(vl,127));
			break;
		}

		return true;
	}

	void CopyPreset(PRESETOBJECT *dest, PRESETOBJECT *src)
	{
		wcscpy(dest->preset.command,src->preset.command);
		wcscpy(dest->preset.destinationFormat,src->preset.destinationFormat);
		wcscpy(dest->preset.runOnFinish,src->preset.runOnFinish);
		wcscpy(dest->preset.sourceFormat,src->preset.sourceFormat);

		dest->preset.mediaType = src->preset.mediaType;
		dest->preset.opType = src->preset.opType;
	}

	bool RaiseError(LinkedListNode<TOKENINFO *> *node, const wchar *err)
	{
		TOKENINFO *tokPtr;
		wchar buf[512]={0};

		tokPtr = node->GetValue();

		wsprintf(buf,L"Syntax error at Line: %d, Pos: %d (%s)",tokPtr->line,tokPtr->pos,err);

		if (this->eventHandler != NULL)
			this->eventHandler(this->handlerArg,buf);

		return false;
	}


	__forceinline bool IsPropertyToken(TOKENINFO *token)
	{
		KEYWORD kw = CheckKeyword(token);

		return (kw != KWNOTKEYWORD && kw != KWPRESET);
	}

	bool HandleForINVALID(LinkedListNode<TOKENINFO *> **token)
	{
		return RaiseError(*token,L"Invalid token");
	}

	bool HandleForSemiColon(LinkedListNode<TOKENINFO *> **token)
	{
		return RaiseError(*token,L"Invalid semi-colon (;) usage");
	}

	bool HandleForComma(LinkedListNode<TOKENINFO *> **token)
	{
		return RaiseError(*token,L"Invalid comma usage");
	}

	bool HandleForBraceClose(LinkedListNode<TOKENINFO *> **token)
	{
		if (PresetParser::ps_PresetObject->waitingProperty ||
			PresetParser::ps_PresetObject->waitingPropertyValue)
		{
			return RaiseError(*token,L"Uncompleted property definition exists before }");
		}

		if (!(*token)->HasNext())
			return RaiseError(*token,L"Uncompleted preset type definition. Missing (;) end of the type");

		if ((*token)->Next()->GetValue()->type != SemiColon)
			return RaiseError(*token,L"Uncompleted preset type definition. Missing (;) end of the type");

		*token = (*token)->Next();

		PresetParser::presetObjectList->Insert(PresetParser::ps_PresetObject);
		PresetParser::ps_PresetObject = NULL;

		return true;
	}

	bool HandleForColon(LinkedListNode<TOKENINFO *> **token)
	{
		if (!(*token)->HasPrevious())
		{
			return RaiseError(*token,L"Property expected");
		}

		if ( !IsPropertyToken( (*token)->Previous()->GetValue() ) )
		{
			return RaiseError(*token,L"Property expected");
		}

		PresetParser::ps_PresetObject->waitingPropertyValue=true;

		return true;
	}

	bool HandleForProperty(LinkedListNode<TOKENINFO *> **token)
	{
		LinkedListNode<TOKENINFO *> *tokNode = *token;
		KEYWORD propName;
		PRESETOBJECT *presetInstance = PresetParser::ps_PresetObject;
		AutoString<wchar> *str;

		propName = CheckKeyword(tokNode->GetValue());

		if (!presetInstance->waitingProperty)
		{
			if (IsPropertyToken(tokNode->GetValue()))
			{
				presetInstance->waitingProperty=true;
				presetInstance->waitingPropertyId = propName;
				return true;
			}

			return RaiseError(*token,L"Invalid string token");
		}

		if (!presetInstance->waitingPropertyValue)
			return RaiseError(tokNode,L"Expected property name before");

		if (presetInstance->waitingPropertyId == KWMEDIATYPE ||
			presetInstance->waitingPropertyId == KWOPTYPE)
		{
			if (!SetObjectProperty(
					presetInstance,
					presetInstance->waitingPropertyId,
					tokNode->GetValue()->data)
					)
			{
				return RaiseError(tokNode,L"Invalid property value");
			}

			if (!tokNode->HasNext())
				return RaiseError(tokNode,L"; expected");

			if (tokNode->Next()->GetValue()->type != SemiColon)
				return RaiseError(tokNode,L"; expected");

			presetInstance->waitingProperty = false;
			presetInstance->waitingPropertyId = KWNOTKEYWORD;
			presetInstance->waitingPropertyValue = false;


			*token = tokNode->Next();

			return true;
		}


		str = ExtractString(token);

		//reset updated tokennode
		tokNode = *token;

		if (str == NULL)
			return RaiseError(tokNode,L"String value expected");


		SetObjectProperty(
			presetInstance,
			presetInstance->waitingPropertyId,
			str->GetNativeString(false)
			);

		
		if (!tokNode->HasNext())
			return RaiseError(tokNode,L"; expected");
		
		if (tokNode->Next()->GetValue()->type != SemiColon)
			return RaiseError(tokNode,L"; expected");

		presetInstance->waitingProperty = false;
		presetInstance->waitingPropertyId = KWNOTKEYWORD;
		presetInstance->waitingPropertyValue = false;

		*token = tokNode->Next();

		return true;
	}

	bool HandleForReservedWord(LinkedListNode<TOKENINFO *> **token)
	{
		KEYWORD keyword;
		PRESETOBJECT *presetInstance;

		wchar buf[512]={0};
		
		LinkedListNode<TOKENINFO *> *tokNode = *token;

		
		keyword = CheckKeyword(tokNode->GetValue());
		presetInstance = PresetParser::ps_PresetObject;

		if (keyword == KWNOTKEYWORD)
		{
			if (presetInstance != NULL)
			{
				if (presetInstance->waitingPropertyId != KWNOTKEYWORD && 
					presetInstance->waitingPropertyValue)
				{
					return HandleForProperty(token);
				}
			}

			wsprintf(buf,L"unknown identifier : %s",tokNode->GetValue()->data);
			
			RaiseError(tokNode,buf);
			return false;
		}

		if (keyword == KWPRESET) //preset keyword.
		{
			if (!tokNode->HasNext())
			{
				RaiseError(tokNode,L"not completed preset structure");
				return false;
			}

			if (tokNode->Next()->GetValue()->type != BraceOpen)
			{
				RaiseError(tokNode,L"syntax error. expected { ");
				return false;
			}

			PRESETOBJECT *presetInstance = ALLOCOBJECT(PRESETOBJECT);
			PresetParser::ps_PresetObject = presetInstance;

			*token = tokNode->Next();

		}
		else //It must be property. handle that.
		{
			return HandleForProperty(token);
		}

		return true;
	}

	AutoString<wchar>* ExtractString(LinkedListNode<TOKENINFO *> **token)
	{
		bool done=false;
		LinkedListNode<TOKENINFO *> *iter = *token;
		AutoString<wchar> *string;

		
		if (iter->GetValue()->type != DoubleQuotes)
		{
			RaiseError(iter,L"Internal error");
			return NULL;
		}

		if (!iter->HasNext())
		{
			RaiseError(iter,L"String content expected.");
			return NULL;
		}

		iter = iter->Next();

		string = new AutoString<wchar>();
		
		/*
			BraceOpen, // {
			BraceClose, // }
			Colon, // :
			Comma, // ,
			SemiColon, // ;
			WhiteSpace // " "
		*/

		while (iter != NULL)
		{
			switch (iter->GetValue()->type)
			{
			case DoubleQuotes:
				{
					done = true;
					break;
				}
				break;
			case String:
				*string += iter->GetValue()->data;
				break;
			case WhiteSpace:
				*string += L" ";
				break;
			case BraceOpen:
				*string += L"{";
				break;
			case BraceClose:
				*string += L"}";
				break;
			case Colon:
				*string += L":";
				break;
			case Comma:
				*string += L",";
				break;
			case SemiColon:
				*string += L";";
			}

			if (done)
				break;

			iter = iter->Next();
		}


		if (!done)
		{
			delete string;
			RaiseError(iter,L"string was not terminated with quote");
			return NULL;
		}

		*token = iter;

		return string;
	}

public:

	PresetParser(LinkedList<TOKENINFO *> *tokenList, COMPILATION_EVENT_HANDLER handler, void *handlerArg)
	{
		this->tokens = tokenList;
		this->presetObjectList = new LinkedList<PRESETOBJECT *>();
		this->eventHandler = handler;
		this->handlerArg = handlerArg;
	}

	~PresetParser(void)
	{
		delete this->presetObjectList;
	}

	bool Parse()
	{
		TOKENINFO *token;

		for (LinkedListNode<TOKENINFO *> *node = this->tokens->Begin();
			node != NULL;
			node = node->Next())
		{
			token = node->GetValue();

			switch (token->type)
			{
			case DoubleQuotes: //handle user string
				{
					if (!HandleForProperty(&node))
						return false;
				}
				break;
			case BraceOpen:
				break;
			case BraceClose:
				{
					if (!HandleForBraceClose(&node))
						return false;
				}
				break;
			case String:
				{
					if (!HandleForReservedWord(&node))
						return false;
				}
				break;
			case Colon:
				{
					if (!HandleForColon(&node))
						return false;
				}
				break;
			case Comma:
				{
					if (!HandleForComma(&node))
						return false;
				}
				break;
			case SemiColon:
				{
					if (!HandleForSemiColon(&node))
						return false;
				}
				break;

			}

		}

		return true;
	}

	LinkedList<PRESETOBJECT *> *GetPresetObjects() const
	{
		return this->presetObjectList;
	}

};

