#pragma once

#include "ui_defs.h"

enum EUIMirroring{
	tmNone,
	tmMirrorHorisontal,
	tmMirrorVertical,
	tmMirrorBoth
};

class CUICustomItem
{
protected:
	enum {
		flValidRect				=0x0001,
		flValidOriginalRect		=0x0002,
		flValidHeadingPivot		=0x0004,
		flFixedLTWhileHeading =0x0008,
	};

	//�������������(� ��������) 
	//����. ������  �� ������� ������������ �������� � ����������� ������������ iOriginalRect
	Frect			iVisRect;

	//����� �������� � �������� ���. 0/0
	Frect			iOriginalRect;

	// �����, ������������ ������� ��������� �������
	Fvector2		iHeadingPivot;

	u32				uFlags;
	u32				uAlign;
	EUIMirroring	eMirrorMode;

public:
					CUICustomItem			();
	virtual			~CUICustomItem			();
	IC void			SetRect					(float x1, float y1, float x2, float y2){iVisRect.set(x1,y1,x2,y2); uFlags|=flValidRect; }
	IC void			SetRect					(const Frect& r){iVisRect.set(r); uFlags|=flValidRect; }
	  void			SetOriginalRect			(float x, float y, float width, float height);

	IC Frect		GetRect					() {return iVisRect;}
	   Frect		GetOriginalRect			() const;
	   Frect		GetOriginalRectScaled	();
	
	   IC bool		GetFixedLTWhileHeading() const { return true; };
	   void			SetHeadingPivot			(const Fvector2& p)		{iHeadingPivot=p; uFlags|=flValidHeadingPivot;}
	   Fvector2		GetHeadingPivot			()						{return iHeadingPivot;}
	   

	void Render(const Fvector2& pos, u32 color, float x1, float y1, float x2, float y2);
	
	void Render(const Fvector2& pos, u32 color);
	void Render(const Fvector2& pos, u32 color, float angle);

	IC void			SetAlign				(u32 align)					{uAlign=align;};
	IC u32			GetAlign				()							{return uAlign;}

	IC void			SetMirrorMode			(EUIMirroring m)			{ eMirrorMode = m; }
	IC EUIMirroring GetMirrorMode			()							{ return eMirrorMode; }
};
