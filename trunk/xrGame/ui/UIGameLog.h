#pragma once

#include "UIScrollView.h"

class CUIPdaMsgListItem;

class CUIGameLog: public CUIScrollView
{
public:
	CUIGameLog();
	virtual ~CUIGameLog();
	CUIPdaMsgListItem*		AddPdaMessage	(LPCSTR msg, float delay);
	virtual void			Update			();

	void					SetTextAtrib	(CGameFont* pFont, u32 color);
	u32						GetTextColor	();

private:
	xr_vector<CUIWindow*>		toDelList;
	u32							txt_color;
};

