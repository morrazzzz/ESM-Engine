#pragma once

#include "UIWindow.h"
#include "../pda_space.h"
#include "../InfoPortionDefs.h"

class CUIGameLog;

class CUIMessagesWindow : public CUIWindow {
public:
						CUIMessagesWindow				();
	virtual				~CUIMessagesWindow				();

	void				AddIconedPdaMessage				(LPCSTR textureName, Frect originalRect, LPCSTR message, int iDelay);

	virtual void		Update();


protected:
	virtual void Init(float x, float y, float width, float height);

	CUIGameLog*			m_pGameLog;
//	Frect				m_ListPos2;
};