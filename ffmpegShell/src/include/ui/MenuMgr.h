#pragma once

#include "stdafx.h"
#include "helper\DynamicArray.h"

struct __MENUCONTAINERINTERNAL;

typedef bool (*MENU_ITEM_HANDLER)(vptr);

typedef enum
{
	Normal,
	Slot,
	Seperator
}MenuItemType;

typedef struct 
{
	uint4 cmdId;
	MenuItemType type;
	wstring menuStr;
	wchar verb[8];
	MENU_ITEM_HANDLER handler;
	vptr handlerArg;
	__MENUCONTAINERINTERNAL *subItems;
}MENUCONTAINERITEM;

#define MCI(ptr) ((MENUCONTAINERINTERNAL *)ptr)
#define MC(ptr) ((MENUCONTAINER *)ptr)

typedef struct __MENUCONTAINERINTERNAL
{
	uint4 structSize;
	uint4 idCmdFirst;
	uint4 idCmdLast;
	uint4 flag;
	uint4 cmdIndex;
	HMENU menuHandle;
	void *parent;
	void *root;
	DynamicArray<MENUCONTAINERITEM *> *items;
}MENUCONTAINERINTERNAL;

typedef struct __MENUCONTAINER
{
	MENUCONTAINERINTERNAL c;
	DynamicArray<MENUCONTAINERITEM *> *orderedItems;
}MENUCONTAINER;

MENUCONTAINER *MeCreateMenuContainer(UINT cmdFirst, UINT cmdLast, UINT flag);

void MeDeleteMenuContainer(MENUCONTAINER *container);

MENUCONTAINER *MeAddSlot(MENUCONTAINER *container, wstring menuString);

int4 MeAddItem(MENUCONTAINER *container, MENU_ITEM_HANDLER menuHandler, vptr arg, wstring menuString, ...);

bool MeAddSeperator(MENUCONTAINER *container);

HRESULT MeActivateMenu(MENUCONTAINER *container, HMENU parentMenu);

bool MeInvokeHandler(MENUCONTAINER *container, int4 cmdId);