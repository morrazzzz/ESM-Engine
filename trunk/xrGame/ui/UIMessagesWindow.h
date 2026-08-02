#pragma once

#include "UIWindow.h"
#include "../pda_space.h"
#include "../InfoPortionDefs.h"

class CUIGameLog;
struct GAME_NEWS_DATA;

class CUIMessagesWindow : public CUIWindow {
public:
						CUIMessagesWindow				();
	virtual				~CUIMessagesWindow				();

	void				AddIconedPdaMessage				(GAME_NEWS_DATA* news);

	virtual void		Update();


protected:
	virtual void Init(float x, float y, float width, float height);

	CUIGameLog*			m_pGameLog;
//	Frect				m_ListPos2;
};