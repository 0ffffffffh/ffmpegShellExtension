#pragma once

#include "stdafx.h"
#include "Memory.h"


struct InvalidAutoStringException { };

typedef struct 
{
	void (*copy)(void *dst, const void *src);
	void (*ncopy)(void *dst, const void *src, uint4 count);
	uint4 (*len)(void *str);
	int4 (*cmp)(const void *s1, const void *s2);
	int4 (*icmp)(const void *s1, const void *s2);
	int4 (*ncmp)(const void *s1, const void *s2,uint4 maxCount);
	int4 (*nicmp)(const void *s1, const void *s2, uint4 maxCount);
	void (*cat)(void *dst, const void *src);
	void* (*sstr)(void *str, const void *subStr);
	uint4 (*fbuflen)(void *format, va_list argList);
	uint4 (*fprnt)(void *buf, uint4 maxCount, void *format, va_list argList);
	int4 (*toint)(void *str);
	int8 (*toint64)(void *str);
	double (*todoub)(void *str);
}AutoStringRoutines;

#if defined(_MSC_VER)
#define cpsf(x) _##x
#else
#define cpsf(x) x
#endif

#if defined(_MSC_VER)

#define strtoll(s,t,b) _strtoi64(s,t,b)
#define wcstoll(s,t,b) _wcstoi64(s,t,b)

#define strtoull(s,t,b) _strtoui64(s,t,b)
#define wcstoull(s,t,b) _wcstoui64(s,t,b)

#endif


//ANSI IMPL
static void __ansicopy(void *dst, const void *src)
{
	strcpy((char *)dst,(const char *)src);
}

static void __ansincopy(void *dst, const void *src, uint4 count)
{
	strncpy((char *)dst,(const char *)src,count);
}

static uint4 __ansilen(void *str)
{
	return (uint4)strlen((const char *)str);
}

static int4 __ansicmp(const void *s1, const void *s2)
{
	return (int4)strcmp((const char *)s1,(const char *)s2);
}

static int4 __ansiicmp(const void *s1, const void *s2)
{
	return (int4)stricmp((const char *)s1,(const char *)s2);
}

static int4 __ansincmp(const void *s1, const void *s2, uint4 maxCount)
{
	return (int4)strncmp((const char *)s1,(const char *)s2,maxCount);
}

static int4 __ansinicmp(const void *s1, const void *s2, uint4 maxCount)
{
	return (int4)strnicmp((const char *)s1,(const char *)s2,maxCount);
}

static void __ansicat(void *dst, const void *src)
{
	strcat((char *)dst,(const char *)src);
}

static void* __ansisstr(void *str, const void *subStr)
{
	return strstr((char *)str,(const char *)subStr);
}

static uint4 __ansifbuflen(void *format, va_list argList)
{
	return (uint4) cpsf(vscprintf((char *)format,argList));
}

static uint4 __ansifprnt(void *buf, uint4 maxCount, void *format, va_list argList)
{
	return (uint4) cpsf(vsnprintf((char *)buf,maxCount,(char *)format, argList));
}

static int4 __atoi(void *str)
{
	return (int4)strtol((const char *)str,NULL,0);
}

static int8 __atoll(void *str)
{
	return (int8)strtoll((const char *)str,NULL,0);
}

static double __atod(void *str)
{
	return (double)strtod((const char *)str,NULL);
}

//UNICODE IMPL
static void __widecopy(void *dst, const void *src)
{
	wcscpy((wchar *)dst,(const wchar *)src);
}

static void __widencopy(void *dst, const void *src, uint4 count)
{
	wcsncpy((wchar *)dst,(const wchar *)src,count);
}

static uint4 __widelen(void *str)
{
	return (uint4)wcslen((wchar *)str);
}

static int4 __widecmp(const void *s1, const void *s2)
{
	return (int4)wcscmp((const wchar *)s1,(const wchar *)s2);
}

static int4 __wideicmp(const void *s1, const void *s2)
{
	return (int4)wcsicmp((const wchar *)s1,(const wchar *)s2);
}

static int4 __widencmp(const void *s1, const void *s2, uint4 maxCount)
{
	return (int4)wcsncmp((const wchar *)s1,(const wchar *)s2,maxCount);
}

static int4 __widenicmp(const void *s1, const void *s2, uint4 maxCount)
{
	return (int4)wcsnicmp((const wchar *)s1,(const wchar *)s2,maxCount);
}

static void __widecat(void *dst, const void *src)
{
	wcscat((wchar *)dst,(const wchar *)src);
}

static void* __widesstr(void *str, const void *subStr)
{
	return wcsstr((wchar *)str,(const wchar *)subStr);
}

static uint4 __widefbuflen(void *format, va_list argList)
{
	return (uint4)cpsf(vscwprintf((wchar_t *)format,argList));
}

static uint4 __widefprnt(void *buf, uint4 maxCount, void *format, va_list argList)
{
	return (uint4)cpsf(vsnwprintf((wchar_t *)buf,maxCount,(wchar_t *)format, argList));
}

static int4 __wtoi(void *str)
{
	return wcstol((const wchar_t *)str,NULL,0);
}

static int8 __wtoll(void *str)
{
	return (int8)wcstoll((const wchar_t *)str,NULL,0);
}

static double __wtod(void *str)
{
	return wcstod((const wchar_t *)str,NULL);
}

const AutoStringRoutines __AutoStringFuncTable[2] = 
{
	{
		__ansicopy,
		__ansincopy,
		__ansilen,
		__ansicmp,
		__ansiicmp,
		__ansincmp,
		__ansinicmp,
		__ansicat,
		__ansisstr,
		__ansifbuflen,
		__ansifprnt,
		__atoi,
		__atoll,
		__atod
	},
	{
		__widecopy,
		__widencopy,
		__widelen,
		__widecmp,
		__wideicmp,
		__widencmp,
		__widenicmp,
		__widecat,
		__widesstr,
		__widefbuflen,
		__widefprnt,
		__wtoi,
		__wtoll,
		__wtod
	}
};


template <class T>
class AutoString
{
	int SIZE_PER_CHR;
	int ROUTINE_INDEX;
private:
	T *str;
	AutoStringRoutines *routineSlot;
	uint4 size;
	uint4 length;
	bool destroyOriginal;

	bool Allocate(uint4 size)
	{
		if (!size)
			return false;

		this->str = (T *)MemoryAlloc(OSIB(T,size+1),TRUE);
		
		if (!this->str)
			return false;

		this->size=size;
		this->length=0;

		return true;
	}

	bool Extend(uint4 amount)
	{
		T* tmp;

		uint4 remain = this->size-this->length;

		//Reallocation not needed.
		if (remain >= amount)
			return true;

		if (remain > 0)
			amount -= remain;


		if (amount < 32)
			amount = 128;

		if (!MemoryReAlloc(OSIB(T,this->size+1),OSIB(T,amount),(void **)&this->str))
		{
			//memory block isnt growable.  
			tmp = (T *)MemoryAlloc(OSIB(T,this->size + amount + 1),TRUE);
			
			if (!tmp)
				return false;

			memcpy(tmp,this->str,OSIB(T,this->length));
			MemoryFree(this->str);
			this->str = tmp;
		}

		this->size += amount;

		return true;
	}

	void Free(bool virt)
	{
		if (this->str)
		{
			if (!virt)
				MemoryFree(this->str);
			this->str = NULL;
			this->size = 0;
			this->length = 0;
		}
	}


	bool Set(T* s)
	{
		Free(false);
		
		if (Allocate(this->routineSlot->len(s)))
		{
			this->routineSlot->copy(this->str,s);
			this->length = this->routineSlot->len(this->str);
			return true;
		}

		return false;
	}

	bool _Append(const T* s, uint4 len)
	{
		if (len > GetRemain())
		{
			if (!Extend(256 - (len - GetRemain())))
				return false;
		}

		this->routineSlot->ncopy(this->str+this->length,s,len);
		this->length += len;
		*(this->str+this->length) = 0;

		return true;
	}

	bool _Append(const T* s)
	{
		if (s == NULL)
			return false;

		return _Append(s,this->routineSlot->len((void *)s));
	}

	bool _AppendFormat(wstring format, va_list argList)
	{
		uint4 reqBufLen;

		if (!format)
			return false;

		reqBufLen = this->routineSlot->fbuflen((void *)format,argList);

		if (reqBufLen > GetRemain())
		{
			if (!Extend(128 - (reqBufLen - GetRemain())))
				return false;
		}
		
		this->routineSlot->fprnt((void *)(this->str+this->length),GetRemain(),(void *)format,argList);
		this->length += reqBufLen;
		*(this->str+this->length) = 0;

		return true;
	}

	void Init()
	{
		this->SIZE_PER_CHR = sizeof(T);
		this->ROUTINE_INDEX = this->SIZE_PER_CHR-1;

		if (this->SIZE_PER_CHR > 2)
			throw InvalidAutoStringException();

		this->str = NULL;
		this->length = this->size = 0;
		this->destroyOriginal=true;
		this->routineSlot = (AutoStringRoutines *)&__AutoStringFuncTable[this->ROUTINE_INDEX];
	}
public:

	AutoString() throw()
	{
		Init();
		Allocate(64);
	}

	AutoString(uint4 initial) throw()
	{
		Init();
		Allocate(initial);
	}

	AutoString(const T *initialStr) throw()
	{
		Init();
		Set((T *)initialStr);
	}

	~AutoString()
	{
		Free(!this->destroyOriginal);
	}

	AutoString<T> &operator+(const AutoString<T> &s)
	{
		AutoString<T> *newStr = new AutoString<T>(this->length + s.length);
		newStr->Set(this->str);
		newStr->_Append(s.str);
		return *newStr;
	}

	AutoString<T> & operator+(const T* s)
	{
		AutoString<T> *newStr = new AutoString<T>(this->length + this->routineSlot->len((void *)s));
		newStr->Set(this->str);
		newStr->_Append(s);
		return *newStr;
	}

	AutoString<T> & operator+=(const T *s)
	{
		_Append(s);
		return *this;
	}

	AutoString<T> & operator+=(const AutoString<T> &s)
	{
		_Append(s.str);
		return *this;
	}

	AutoString<T> & operator+=(const T chr) 
	{
		Append(chr);
		return *this;
	}

	bool operator==(const T *str) const
	{
		return this->routineSlot->cmp(this->str,str) == 0;
	}

	bool operator==(const AutoString<T> *s) const
	{
		return this->routineSlot->cmp(this->str,s->str) == 0;
	}

	AutoString<T> &SubString(uint4 startIndex, uint4 count)
	{
		AutoString<T> *newStr;

		if (startIndex > this->length)
			return *this;

		if (startIndex + count > this->length)
			return *this;

		newStr = new AutoString<T>(count);
		newStr->routineSlot->ncopy(newStr->str,this->str+startIndex,count);
		newStr->length=count;

		return *newStr;
	}

	int4 IndexOf(T *s)
	{
		T* result;

		result = (T *)this->routineSlot->sstr(this->str,s);

		if (!result)
			return -1;

		return (int4)(result - this->str);
	}

	void TrimLeft()
	{
		uint4 ts=0;
		T *p=this->str;

		if (*p != 0x20)
			return;

		while (((uint4)*p++) == 0x20);

		ts = (p-1) - this->str;

		memmove(this->str,this->str+ts,OSIB(T,this->length-ts));
		memset(this->str+(this->length-ts),0,OSIB(T,ts));

		this->length -= ts;

	}

	void TrimRight()
	{
		uint4 ts=0;
		T* p=this->str+this->length-1;

		if (*p != 0x20)
			return;

		while (((uint4)*p) == 0x20)
		{
			*p=0;
			p--;
		}

		this->length = (p - this->str) + 1;
	}

	void Trim()
	{
		TrimRight();
		TrimLeft();
	}

	uint4 GetLength() const
	{
		return this->length;
	}

	uint4 GetRemain() const
	{
		return this->size - this->length;
	}

	uint4 CopyString(T buffer[],uint4 maxCch) 
	{
		uint4 copyLen=this->length;

		if (this->length > maxCch)
			copyLen = maxCch;

		this->routineSlot->ncopy(buffer,this->str,copyLen);
		return copyLen;
	}

	bool EndsWith(const T *cstr, bool caseSensitive)
	{
		uint4 slen;
		
		if (!cstr)
			return false;

		slen = this->routineSlot->len((void *)cstr);

		if (slen > this->length)
			return false;

		if (caseSensitive)
			return this->routineSlot->cmp(this->str+(this->length-slen),cstr) == 0;
		
		return this->routineSlot->icmp(this->str+(this->length-slen),cstr) == 0;
	}

	bool StartsWith(const T *cstr, bool caseSensitive)
	{
		uint4 slen;

		if (!cstr)
			return false;

		slen = this->routineSlot->len((void *)cstr);

		if (slen > this->length)
			return false;

		if (caseSensitive)
			return this->routineSlot->ncmp(this->str,cstr,slen) == 0;

		return this->routineSlot->nicmp(this->str,cstr,slen) == 0;
	}

	bool Insert(uint4 insertIndex, const T *s)
	{
		NOTIMPLEMENTED_R(false);
	}

	AutoString<T> & Replace(const T *what, const T* with)
	{
		AutoString<T> *clone = new AutoString<T>(this->str);
		DynamicArray<int> foundPos(10);
		int pos=0,copyPos=0,wd=0,findStrLen,replStrLen;
		T *fptr=clone->str;
		bool shiftLeft=false;

		while ((fptr = (T *)this->routineSlot->sstr(fptr,what)) != NULL)
		{
			foundPos.Add(fptr-clone->str);
			fptr++;
		}

		if (!foundPos.GetCount())
			return *this;

		findStrLen = this->routineSlot->len((void *)what);
		replStrLen = this->routineSlot->len((void *)with);
		wd = replStrLen - findStrLen;

		if (wd > 0)
			clone->Extend(wd * foundPos.GetCount());
		else
			shiftLeft = true;

		for (int i=0;i<foundPos.GetCount();i++)
		{
			pos = foundPos[i] + findStrLen;
			copyPos = foundPos[i];

			if (i)
			{
				pos += wd;
				copyPos += wd;
			}

			ffhelper::DataManipHelper<T>::ShiftBlock(clone->str,pos,abs(wd),clone->length,false,shiftLeft);
			ffhelper::DataManipHelper<T>::Copy(clone->str,copyPos,(T *)with,replStrLen);

			clone->length += wd;
		}

		return *clone;
	}

	uint4 Append(const T *str, uint4 count)
	{
		if (_Append(str,count))
			return count;

		return 0;
	}

	uint4 Append(T chr)
	{
		T sbuf[2] = {chr,0};
		return Append((const T*)sbuf,1);
	}

	uint4 AppendFormat(T *format, ...)
	{
		uint4 written;
		va_list argList;

		va_start(argList,format);

		written = _AppendFormat(format,argList);

		va_end(argList);

		return written;
	}

	int4 ToInt32() const
	{
		return this->routineSlot->toint(this->str);
	}

	int8 ToInt64() const
	{
		return this->routineSlot->toint64(this->str);
	}

	double ToDouble() const
	{
		return this->routineSlot->todoub(this->str);
	}

	T *c_str() const
	{
		return this->str;
	}

	T* GetNativeString(bool createNewInstance=true)
	{
		T* strInstance;

		this->destroyOriginal = !createNewInstance;

		if (!createNewInstance)
			return this->str;

		strInstance = ALLOCARRAY(T,this->length + 1);

		if (!strInstance)
			return NULL;

		CopyString(strInstance,this->length);

		return strInstance;
	}
};

typedef AutoString<char> AutoStringA;
typedef AutoString<wchar> AutoStringW;