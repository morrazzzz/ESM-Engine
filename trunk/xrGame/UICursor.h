#pragma once

#include "ui_base.h"
#include "UIStaticItem.h"

class CUIStatic;

class CUICursor:public pureRender
{
	bool			bVisible;
	Fvector2		vPos;
	Fvector2		vPrevPos;
	bool			m_b_use_win_cursor;
	CUIStatic*		m_static;
	void			InitInternal	();
public:
					CUICursor		();
	virtual			~CUICursor		();
	virtual void	OnRender		();
	
	Fvector2		GetCursorPositionDelta();

	Fvector2		GetCursorPosition		();
	void			SetUICursorPosition		(Fvector2 pos);
	void			UpdateCursorPosition		(int _dx, int _dy);

	bool			IsVisible		() {return bVisible;}
	void			Show			() {bVisible = true;}
	void			Hide			() {bVisible = false;}
};
