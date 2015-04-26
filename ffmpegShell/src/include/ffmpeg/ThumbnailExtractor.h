#pragma once

#include "..\stdafx.h"
#include "..\ffmpegProcess.h"
#include "MediaInfo.h"
#include "..\helper\DynamicArray.h"

class ThumbnailExtractor
{
private:
	wnstring fileName;
	DynamicArray<HANDLE> *bitMaps;
public:
	ThumbnailExtractor(wnstring fileName)
	{
		this->fileName = fileName;
	}

	~ThumbnailExtractor()
	{
		delete this->bitMaps;
	}

	HANDLE GetFrameBitmapByFrameId(int4 frameId)
	{
		if (frameId < 0)
			return NULL;

		if (frameId >= this->bitMaps->GetCount())
			return NULL;

		return (*this->bitMaps)[frameId];
	}

	bool ExtractFrame(int4 frameCount, ffmpegTime *totalTime)
	{
		int4 frameId;
		int8 secsPerFrame=0,secs=0;
		wchar outFile[MAX_PATH];
		wchar buf[MAX_PATH + (MAX_PATH / 2)];
		ffmpegProcess *ffProc = NULL;
		ffmpegTime frameTime;

		secsPerFrame = FFTIME_TO_SECONDS(totalTime) / (frameCount + 1);

		for (int4 i=1;i<=frameCount;i++)
		{
			ffProc = new ffmpegProcess(FFMPEG);

			memset(outFile,0,sizeof(outFile));
			GetTempPathW(MAX_PATH,outFile);
			

			secs += secsPerFrame;

			SECONDS_TO_FFTIME_INT(secs,&frameTime);

			wsprintf(buf,L"-ss %02d:%02d:%02d -i \"%s\" -r 1 -s cif -vframes 1 \"%s\\imgout_%d.bmp\"",
				frameTime.hours,frameTime.minutes,frameTime.seconds,this->fileName,
				outFile,i);

			ffProc->SetArg(buf);
			ffProc->Start(NULL);
			ffProc->Wait();

			delete ffProc;
		}	
			
		this->bitMaps = new DynamicArray<HANDLE>();

		for (frameId = 1; frameId <= frameCount; frameId++)
		{
			memset(buf,0,sizeof(buf));
			wsprintf(buf,L"%s\\imgout_%d.bmp",outFile,frameId);
			
			if (PathFileExistsW(buf))
			{
				bitMaps->Add(
					LoadImageW(
						ffhelper::Helper::GetAppInstance(),
						(LPCWSTR)buf,
						IMAGE_BITMAP,
						256,192,
						LR_LOADFROMFILE | LR_CREATEDIBSECTION));



			}
		}


		return this->bitMaps->GetCount() == frameCount;
	}

	bool ExtractFrame(ffmpegTime *startTime)
	{
		return ExtractFrame(1,startTime);
	}

};