#pragma once
#include "..\UI.h"

#define IMPL_CTRLBASE_CTOR(t) t(UIOBJECT *ui, uint4 ctrlId) : UiControlBase(ui,ctrlId) 

class UiControlBase
{
protected:
	UIOBJECT *ui;
	uint4 ctrlId;
	HWND ctrlHwnd;
public:
	UiControlBase()
	{
		this->ui = NULL;
		this->ctrlId = 0;
	}

	UiControlBase(UIOBJECT *ui, uint4 ctrlId)
	{
		this->ui = ui;
		this->ctrlId = ctrlId;

		this->ctrlHwnd = GetDlgItem(this->ui->hwnd,this->ctrlId);
	}

	virtual void OnInitControl()
	{
	}

};