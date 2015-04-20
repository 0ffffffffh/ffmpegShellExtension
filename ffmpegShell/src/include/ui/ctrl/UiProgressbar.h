#pragma once

#include "UiControlBase.h"

class UiProgressbar : public UiControlBase
{
private:
	uint4 currPos;
	uint4 rMin,rMax;
	HWND pbHwnd;

	void UpdatePos()
	{
		SendMessage(this->pbHwnd,PBM_SETPOS,this->currPos,0);
	}

public:

	IMPL_CTRLBASE_CTOR(UiProgressbar)
	{
	}

	UiProgressbar(void)
	{
	}

	~UiProgressbar(void)
	{
	}

	void OnInitControl()
	{
		this->pbHwnd = GetDlgItem(this->ui->hwnd,this->ctrlId);
	}

	void SetRange(uint4 min, uint4 max)
	{
		SendMessage(this->pbHwnd,PBM_SETRANGE32,min,max);
		this->rMin = min;
		this->rMax = max;
	}

	void Finish()
	{
		SetPos(this->rMax);
	}

	void SetPos(uint4 pos)
	{
		if (pos > this->rMax)
			pos = this->rMax;

		this->currPos=pos;

		UpdatePos();
	}

	void Increment()
	{
		this->currPos++;

		if (this->currPos > this->rMax)
			this->currPos=this->rMax;

		UpdatePos();
	}

	void Add(uint4 stepAmount)
	{
		this->currPos += stepAmount;

		if (this->currPos > this->rMax)
			this->currPos = this->rMax;

		UpdatePos();
	}
};

