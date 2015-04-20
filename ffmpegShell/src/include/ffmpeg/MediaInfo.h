#pragma once

#include "..\stdafx.h"
#include "..\ffmpegProcess.h"
#include <math.h>

typedef struct
{
	uint4 hours;
	uint4 minutes;
	uint4 seconds;
	uint4 milliseconds;
}ffmpegTime;

#define FFTIME_TO_SECONDS( fftime ) \
	((fftime)->hours * 60 * 60) + ((fftime)->minutes * 60) + ((fftime)->seconds)

typedef enum
{
	SrcVideo,
	SrcAudio
}MediaSourceType;

typedef struct
{
	char				codecName[16];
	char				codecType[16];
	char				codecTag[8];
	char				aspectRatio[6];
	ffmpegTime			duration;
	MediaSourceType		streamType;
	uint4				width;
	uint4				height;
	uint4				bitRate;
	uint4				numberOfFrames;
	uint4				sampleRate;
	uint4				channels;
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
	ffmpegTime time;
	uint8 size;
	MediaSourceType type;
	StreamInfo streams[2];
	uint4 streamCount;
	ffmpegProcess *process;
	wchar mediaFileName[MAX_PATH];
	DynamicArray<LinkedList<AutoStringA *> *> ffprobeInfoList;

	void DoubleToDuration(double d, ffmpegTime *dt)
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

	int4 CompareffmpegTime(ffmpegTime *t1, ffmpegTime *t2)
	{
		uint4 t1Total=0,t2Total=0;

		if (t1 == NULL)
		{
			if (t2 == NULL)
				return 0;

			return -1;
		}

		if (t2 == NULL)
		{
			if (t1 == NULL)
				return 0;

			return 1;
		}

		t1Total = FFTIME_TO_SECONDS(t1);
		t2Total = FFTIME_TO_SECONDS(t2);

		return (t1Total > t2Total) ? 1 : (t1Total < t2Total) ? -1 : 0;
	}

	void SetStreamProperty(LinkedList<AutoStringA *> *kvChain, int4 streamIndex)
	{
		StreamInfo *streamInfo;
		AutoStringA *key,*val;
		
		key = kvChain->End()->Previous()->GetValue();
		val = kvChain->End()->GetValue();
		
		streamInfo = &this->streams[streamIndex];

		if (streamIndex+1 > this->streamCount)
			this->streamCount = streamIndex+1;

		if (*key == "codec_name")
			strcpy(streamInfo->codecName,val->c_str());
		else if (*key == "codec_type")
		{
			strcpy(streamInfo->codecType,val->c_str());

			if (!strcmp(streamInfo->codecType,"video"))
			{
				this->type = SrcVideo;
				streamInfo->streamType = SrcVideo;
			}
			else if (!strcmp(streamInfo->codecType,"audio"))
			{
				streamInfo->streamType = SrcAudio;
				this->type = SrcAudio;
			}
		}
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
		process->SetArg((wnstring)wcmdBuf);
		process->Start(this);
		process->Wait(true);

		for (uint4 i=0;i<this->ffprobeInfoList.GetCount(); i++)
		{
			kvChain = this->ffprobeInfoList[i];

			SetInfo(kvChain);

			delete kvChain;

		}

		this->ffprobeInfoList.ReleaseMemory();
	}

	static void KvDisposer(void *obj)
	{
		AutoStringA *astr = (AutoStringA *)obj;
		delete astr;
	}

	static void OnStdoutLineReceived(vptr arg, LPCSTR line)
	{
		MediaInfo *_this = (MediaInfo *)arg;
		LinkedList<AutoStringA *> *infoKv = NULL;

		infoKv = FpParseInfo(line);
		
		if (infoKv != NULL)
		{
			infoKv->SetDisposer(KvDisposer);
			_this->ffprobeInfoList.Add(infoKv);
		}
	}

public:
	MediaInfo(wnstring mediaFile)
	{
		wcscpy(this->mediaFileName,mediaFile);
		this->streamCount=0;
		Initialize();
	}

	bool GetMediaDuration(ffmpegTime *mt)
	{
		ffmpegTime *maxTime=NULL;
		

		//TODO: Do this thing on the stream info initialization at once

		for (int4 i=0;i<this->streamCount;i++)
		{
			if (CompareffmpegTime(&this->streams[i].duration,maxTime) > 0)
				maxTime = &this->streams[i].duration;
		}

		memcpy(mt,maxTime,sizeof(ffmpegTime));

		return true;
	}

	uint4 GetBitrate(uint4 streamIndex) const
	{
		if (streamIndex >= this->streamCount)
			return 0;
		
		return this->streams[streamIndex].bitRate;
	}




};