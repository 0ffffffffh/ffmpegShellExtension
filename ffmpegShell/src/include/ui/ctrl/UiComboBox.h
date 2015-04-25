#pragma once

#include "UiControlBase.h"

class UiComboBox : public UiControlBase
{
private:

public:

	IMPL_CTRLBASE_CTOR(UiComboBox)
	{
	}

	bool AddItem(wnstring itemStr)
	{
		return SendMessageW(this->ctrlHwnd,CB_ADDSTRING,NULL,(LPARAM)itemStr) > -1;
	}

	int4 GetSelectedIndex()
	{
		return SendMessageW(this->ctrlHwnd,CB_GETCURSEL,NULL,NULL);
	}

	bool SetSelectedIndex(int4 index)
	{
		return SendMessageW(this->ctrlHwnd,CB_SETCURSEL,(WPARAM)index,NULL) == index;
	}
};