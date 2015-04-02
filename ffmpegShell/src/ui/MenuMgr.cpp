#include "Stdafx.h"
#include "ui\MenuMgr.h"

HMENU MeiCreateMenu()
{
	return CreatePopupMenu();
}

void MeiDestroyMenu(HMENU menu)
{
	DestroyMenu(menu);
}

bool MeiInsertMenuAsChild(HMENU parent, HMENU child, UINT pos, LPCWSTR menuStr)
{
	return (bool)InsertMenuW(parent,pos,MF_BYPOSITION | MF_POPUP,(UINT_PTR)child,menuStr);
}

bool MeiInsertMenuItem(HMENU menu, LPCWSTR menuStr, DWORD commandId, UINT pos)
{
	MENUITEMINFOW mii={0};

	mii.cbSize = sizeof(MENUITEMINFOW);
	mii.fMask = MIIM_ID | MIIM_TYPE;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED | MFS_DEFAULT;
	mii.wID = commandId;
	mii.dwTypeData = (LPWSTR)menuStr;
	mii.cch = wcslen(menuStr);

	return (bool)InsertMenuItemW(menu,pos,TRUE,&mii);
}

bool MeiAddSeperator(HMENU menu,UINT sepPos)
{
	MENUITEMINFOW mii={0};
	mii.cbSize = sizeof(MENUITEMINFOW);
	mii.fMask = MIIM_TYPE;
	mii.fType = MFT_SEPARATOR;
	mii.fState = MFS_ENABLED;

	return (bool)InsertMenuItemW(menu,sepPos,TRUE,&mii);
}


bool MeiFillMenuContainerInternal(MENUCONTAINERINTERNAL *intContainer, UINT cmdFirst, UINT cmdLast, UINT flag)
{
	intContainer->idCmdFirst = cmdFirst;
	intContainer->idCmdLast = cmdLast;
	intContainer->flag = flag;
	intContainer->items = new DynamicArray<MENUCONTAINERITEM *>(10);
	intContainer->menuHandle = MeiCreateMenu();

	if (!intContainer->menuHandle)
	{
		delete intContainer->items;
		return false;
	}

	return true;
}

void *MeiTraceRootContainer(MENUCONTAINERINTERNAL *container)
{
	return NULL;
}

MENUCONTAINERINTERNAL *MeiCreateMenuContainerInternal(void *root, uint4 cmdFirst, uint4 cmdLast, uint4 flag)
{
	MENUCONTAINERINTERNAL *intContainer = NULL;

	intContainer = ALLOCOBJECT(MENUCONTAINERINTERNAL);

	if (!intContainer)
		return NULL;

	if (!MeiFillMenuContainerInternal(intContainer,cmdFirst,cmdLast,flag))
	{
		FREEOBJECT(intContainer);
		return NULL;
	}

	intContainer->structSize = sizeof(MENUCONTAINERINTERNAL);
	intContainer->root = root;

	return intContainer;
}


MENUCONTAINER *MeCreateMenuContainer(UINT cmdFirst, UINT cmdLast, UINT flag)
{
	MENUCONTAINER *container = NULL;

	if (cmdLast - cmdFirst <= 0)
		return NULL;

	container = ALLOCOBJECT(MENUCONTAINER);

	if (!container)
		return NULL;


	if (!MeiFillMenuContainerInternal(&container->c,cmdFirst,cmdLast,flag))
	{
		FREEOBJECT(container);
		return NULL;
	}

	container->c.structSize = sizeof(MENUCONTAINER);
	container->c.root = (void *)container;
	container->orderedItems = new DynamicArray<MENUCONTAINERITEM *>(10);

	return container;
}



void MeDeleteMenuContainer(MENUCONTAINER *container)
{
	uint4 itemCount,i;
	MENUCONTAINERITEM *item;

	itemCount = container->c.items->GetCount();

	for (i=0;i<itemCount;i++)
	{
		item = (*container->c.items)[i];
		
		if (item->type == Slot)
		{
			MeDeleteMenuContainer(MC(item->subItems));
			delete item->subItems->items;
		}
		else if (item->type == Normal)
			FREEARRAY(item->menuStr);

		FREEOBJECT(item);
	}
}

int4 _MeAddItem(MENUCONTAINER *container, MenuInvokeStyle style, MENU_ITEM_HANDLER menuHandler, vptr arg, MenuItemType type, wnstring menuString, va_list argList)
{
	MENUCONTAINERITEM *menuItem;
	wnstring str = NULL;
	void *rootContainer;

	if (type < Normal || type > Seperator)
		return -1;

	if (container->c.idCmdFirst + container->c.cmdIndex > container->c.idCmdLast)
		return -1;
	
	if (type != Seperator)
	{
		str = ALLOCARRAY(wchar, 128);

		if (!str)
			return -1;
	}

	menuItem = ALLOCOBJECT(MENUCONTAINERITEM);

	if (!menuItem)
	{
		FREEOBJECT(str);
		return -1;
	}

	if (type != Seperator)
	{
		_vsnwprintf(str,128,menuString, argList);
		menuItem->menuStr = str;
	}

	switch (type)
	{
	case Slot:
		{
			if (container->c.parent == NULL)
				rootContainer = (void *)container;
			else
				rootContainer = MCI(container->c.parent)->root;

			menuItem->subItems = MeiCreateMenuContainerInternal(rootContainer,container->c.idCmdFirst + container->c.cmdIndex,
													container->c.idCmdFirst + container->c.cmdIndex + 1,
													container->c.flag);
			menuItem->subItems->parent = &container->c;
		}
		break;
	case Normal:
		{
			menuItem->cmdId = container->c.idCmdFirst + container->c.cmdIndex;
			swprintf(menuItem->verb,L"Vrb%x",container->c.cmdIndex);
			menuItem->handler = menuHandler;
			menuItem->handlerArg = arg;
		}
		break;
	}

	menuItem->invokeStyle = style;
	menuItem->type = type;

	if (type == Normal)
	{
		container->c.cmdIndex++;
		MC(container->c.root)->orderedItems->Add(menuItem);
	}

	if (container->c.parent != NULL)
		MCI(container->c.parent)->cmdIndex++;

	
	return container->c.items->Add(menuItem);
}

MENUCONTAINER *MeAddSlot(MENUCONTAINER *container, wnstring menuString)
{
	int4 index = _MeAddItem(container,NonInvokable,NULL,NULL,Slot,menuString,NULL);

	if (index > 0)
		return MC((*(container->c.items))[index]->subItems);

	return NULL;
}

int4 MeAddItem(MENUCONTAINER *container, MENU_ITEM_HANDLER menuHandler, vptr arg, wnstring menuString, ...)
{
	int4 result;
	va_list argList;

#ifndef _DEBUG
	if (!menuHandler)
		return -1;
#endif

	va_start(argList,menuString);

	result = _MeAddItem(container, 
		ShortTimeHandler, 
		menuHandler,
		arg,
		Normal,
		menuString,
		argList);

	va_end(argList);

	return result;
}

int4 MeAddItem2(MENUCONTAINER *container, MenuInvokeStyle style, MENU_ITEM_HANDLER menuHandler, vptr arg, wnstring menuString, ...)
{
	int4 result;
	va_list argList;

#ifndef _DEBUG
	if (!menuHandler)
		return -1;
#endif

	va_start(argList,menuString);

	result = _MeAddItem(container, 
		style, 
		menuHandler,
		arg,
		Normal,
		menuString,
		argList);

	va_end(argList);

	return result;
}

bool MeAddSeperator(MENUCONTAINER *container)
{
	return _MeAddItem(container,NonInvokable,NULL,NULL,Seperator,NULL,NULL)>0;
}

bool _MeActivateMenu(MENUCONTAINERINTERNAL *container)
{
	uint4 pos=0,i=0;
	uint4 len = container->items->GetCount();
	MENUCONTAINERITEM *item;

	while (i < len)
	{
		item = (*container->items)[i];

		switch (item->type)
		{
		case Slot:
			{
				_MeActivateMenu(item->subItems);
				MeiInsertMenuAsChild(MCI(item->subItems->parent)->menuHandle,item->subItems->menuHandle,pos++,item->menuStr);
			}
			break;
		case Normal:
			MeiInsertMenuItem(container->menuHandle,item->menuStr,item->cmdId,pos++);
			break;
		case Seperator:
			MeiAddSeperator(container->menuHandle,pos++);
			break;
		}

		i++;
	}

	return true;
}

HRESULT MeActivateMenu(MENUCONTAINER *container,HMENU parentMenu)
{
	if (!container)
		return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_NULL, ERROR_INVALID_PARAMETER);

	if (!container->c.items->GetCount())
		return MAKE_HRESULT(SEVERITY_ERROR, FACILITY_NULL, ERROR_INVALID_PARAMETER);

	_MeActivateMenu(&container->c);

	MeiInsertMenuAsChild(parentMenu,container->c.menuHandle,0,L"ffmpeg Shell");

	return MAKE_HRESULT(SEVERITY_SUCCESS,FACILITY_NULL,container->c.idCmdFirst+container->c.cmdIndex - container->c.idCmdFirst + 1);
}

DWORD WINAPI __MenuInvokeWorker(PVOID arg)
{
	MENUCONTAINERITEM *menuItem;

	if (arg == NULL)
		return 1;
	
	menuItem = (MENUCONTAINERITEM *)arg;

	menuItem->handler(menuItem->handlerArg);

	return 0;
}

bool _MeiInvokeOnSeperateThread(MENUCONTAINERITEM *menuItem)
{
	DWORD tid;
	
	return CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)__MenuInvokeWorker,menuItem,0,&tid) !=
		INVALID_HANDLE_VALUE;
}

bool MeInvokeHandler(MENUCONTAINER *container, int4 cmdId)
{
	MENU_ITEM_HANDLER handler;
	MENUCONTAINERITEM *menuItem;

	menuItem = (*container->orderedItems)[cmdId];

	if (!menuItem)
		return false;

	if (menuItem->invokeStyle == NonInvokable)
		return false;

	if (menuItem->invokeStyle == LongTimeHandler)
		return _MeiInvokeOnSeperateThread(menuItem);

	handler = menuItem->handler;

	if (handler != NULL)
		return handler(menuItem->handlerArg);

	return false;
}