#pragma once

#include "stdafx.h"
#include "ffmpegProcess.h"
#include <math.h>

typedef struct
{
	uint4 hours;
	uint4 minutes;
	uint4 seconds;
	uint4 milliseconds;
}Duration;

typedef enum
{
	SrcVideo,
	SrcAudio
}MediaSourceType;

typedef struct
{
	char codecName[16];
	char codecType[16];
	char codecTag[8];
	char aspectRatio[6];
	Duration duration;
	uint4 width;
	uint4 height;
	uint4 bitRate;
	uint4 numberOfFrames;
	uint4 sampleRate;
	uint4 channels;
}StreamInfo;

LinkedList<AutoStringA*> *FpParseInfo(LPCSTR line)
{
	LinkedList<AutoStringA*> *fiObject = new LinkedList<AutoStringA *>();
	AutoStringA *value;
	char *p = (char *)line,*lptr=(char *)line;
	uint4 len = 0;
	bool quotedVal=false;
	
	while (*p != 0)
	{
		if (*p == '.' || *p == '=')
		{
			//skip dot
			
			len = p - lptr;

			value = new AutoStringA();
			value->Append(lptr,len);

			fiObject->Insert(value);

			lptr = p+1;

			if (*p == '=')
			{
				if (*(++p) == '"')
				{
					quotedVal=true;
					p++;
				}

				value = new AutoStringA();

				while (*p != 0)
				{
					if (quotedVal && *p == '"')
						break;

					(*value) += *p++;
				}

				fiObject->Insert(value);

				break;
			}
		}

		p++;
	}

	return fiObject;
}

class MediaInfo
{
private:
	Duration time;
	uint8 size;
	MediaSourceType type;
	StreamInfo streams[2];

	ffmpegProcess *process;
	wchar mediaFileName[MAX_PATH];
	DynamicArray<LinkedList<AutoStringA *> *> ffprobeInfoList;

	void DoubleToDuration(double d, Duration *dt)
	{
		const int SECONDS_IN_MINUTE = 1 * 60;
		const int SECONDS_IN_HOUR = SECONDS_IN_MINUTE * 60;

		dt->hours = d / SECONDS_IN_HOUR;
		d = fmod(d,SECONDS_IN_HOUR);
		dt->minutes = d / SECONDS_IN_MINUTE;
		d = fmod(d,SECONDS_IN_MINUTE);
		
		dt->seconds = d;
		d = fmod(d,dt->seconds);
		dt->milliseconds = d * 1000;
	}

	void SetStreamProperty(LinkedList<AutoStringA *> *kvChain, int4 streamIndex)
	{
		StreamInfo *streamInfo;
		AutoStringA *key,*val;
		
		key = kvChain->End()->Previous()->GetValue();
		val = kvChain->End()->GetValue();
		
		streamInfo = &this->streams[streamIndex];

		if (*key == "codec_name")
			strcpy(streamInfo->codecName,val->c_str());
		else if (*key == "codec_type")
			strcpy(streamInfo->codecType,val->c_str());
		else if (*key == "codec_tag_string")
			strcpy(streamInfo->codecTag,val->c_str());
		else if (*key == "display_aspect_ratio")
			strcpy(streamInfo->aspectRatio,val->c_str());
		else if (*key == "width")
			streamInfo->width = val->ToInt32();
		else if (*key == "height")
			streamInfo->height = val->ToInt32();
		else if (*key == "bit_rate")
			streamInfo->bitRate = val->ToInt32();
		else if (*key == "nb_frames")
			streamInfo->numberOfFrames = val->ToInt32();
		else if (*key == "sample_rate")
			streamInfo->sampleRate = val->ToInt32();
		else if (*key == "channels")
			streamInfo->channels = val->ToInt32();
		else if (*key == "duration")
			DoubleToDuration(val->ToDouble(),&streamInfo->duration);
	}

	void SetFormatProperty(LinkedList<AutoStringA *> *kvChain)
	{
		AutoStringA *key,*val;
		
		key = kvChain->End()->Previous()->GetValue();
		val = kvChain->End()->GetValue();

		if (*key == "duration")
			DoubleToDuration(val->ToDouble(),&this->time);
		else if (*key == "size")
			this->size = val->ToInt64();

	}

	void SetInfo(LinkedList<AutoStringA *> *kvChain)
	{
		LinkedListNode<AutoStringA *> *node;
		AutoStringA *m;
		int4 si;

		if (kvChain->GetCount() < 2)
			return;
		
		node = kvChain->Begin();

		if (*node->GetValue() == "streams")
		{
			node = node->Next();

			if (*node->GetValue() == "stream")
			{
				si = node->Next()->GetValue()->ToInt32();

				SetStreamProperty(kvChain,si);
			}
		}
		else if (*node->GetValue() == "format")
		{
			SetFormatProperty(kvChain);
		}
		
	}

	void Initialize()
	{
		process = new ffmpegProcess(FFPROBE);
		LinkedList<AutoStringA *> *kvChain;

		wchar wcmdBuf[MAX_PATH];
		wsprintf(wcmdBuf,L"-print_format flat -show_format -show_streams \"%s\"",this->mediaFileName);

		process->OnLineReceived = MediaInfo::OnStdoutLineReceived;
		process->SetArg((wstring)wcmdBuf);
		process->Start(this);
		process->Wait(true);

		for (int i=0;i<this->ffprobeInfoList.GetCount(); i++)
		{
			kvChain = this->ffprobeInfoList[i];

			SetInfo(kvChain);

			delete kvChain;

		}

		this->ffprobeInfoList.ReleaseMemory();
	}

	static void OnStdoutLineReceived(vptr arg, LPCSTR line)
	{
		MediaInfo *_this = (MediaInfo *)arg;
		LinkedList<AutoStringA *> *infoKv = NULL;

		infoKv = FpParseInfo(line);
		
		if (infoKv != NULL)
			_this->ffprobeInfoList.Add(infoKv);
	}

public:
	MediaInfo(wstring mediaFile)
	{
		wcscpy(this->mediaFileName,mediaFile);

		Initialize();
	}

	bool GetMediaDuration(Duration *mt)
	{
		//TODO: Valitate
		memcpy(mt,&this->streams[0].duration,sizeof(Duration));
		return true;
	}

	uint4 GetBitrate(uint4 streamIndex) const
	{
		//TODO: Validate stream index
		return this->streams[streamIndex].bitRate;
	}




};