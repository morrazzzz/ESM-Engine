#pragma once

#include "UIWindow.h"
class CUIStatic;
class CUITextWnd;
struct GAME_NEWS_DATA;

class CUINewsItemWnd :public CUIWindow
{
typedef	CUIWindow		inherited;

	CUITextWnd* m_UITextDate;
//	CUITextWnd* m_UICaption;
	CUITextWnd* m_UIText;
	CUIStatic*				m_UIImage;

public:
					CUINewsItemWnd					();
	virtual			~CUINewsItemWnd					();
			void	Init							(LPCSTR xml_name, LPCSTR start_from);
			void	Setup							(GAME_NEWS_DATA& news_data);
};