#include "StdAfx.h"
#include "UIMessagesWindow.h"
#include "UIGameLog.h"
#include "UIXmlInit.h"
#include "../game_news.h"
#include "UIPdaMsgListItem.h"

CUIMessagesWindow::CUIMessagesWindow(){
	m_pGameLog = nullptr;
	Init(0, 0, UI_BASE_WIDTH, UI_BASE_HEIGHT);
}

CUIMessagesWindow::~CUIMessagesWindow(){
	
}

void CUIMessagesWindow::Init(float x, float y, float width, float height){

	CUIXml		 xml;
	u32			color;
	CGameFont*	pFont;

	xml.Init(CONFIG_PATH, UI_PATH, "messages_window.xml");

	m_pGameLog = xr_new<CUIGameLog>();m_pGameLog->SetAutoDelete(true);
	m_pGameLog->Show(true);
	AttachChild(m_pGameLog);
	CUIXmlInit::InitScrollView(xml, "sp_log_list", 0, m_pGameLog);
}

void CUIMessagesWindow::AddIconedPdaMessage(GAME_NEWS_DATA* news)
{
	CUIPdaMsgListItem *pItem			= m_pGameLog->AddPdaMessage(news->SingleLineText(), float(news->show_time));
	pItem->SetTextComplexMode			(true);
	pItem->UIIcon.InitTexture(news->texture_name.c_str());

//	news->tex_rect.rb.add(news->tex_rect.lt);
	pItem->UIIcon.SetTextureRect(news->tex_rect);
	pItem->UIMsgText.SetWndPos			(pItem->UIIcon.GetWidth(), pItem->UIMsgText.GetWndPos().y);
	pItem->UIMsgText.AdjustHeightToText	();

	float h1 = _max(pItem->UIIcon.GetHeight(), pItem->UIMsgText.GetWndPos().y + pItem->UIMsgText.GetHeight());
	pItem->SetHeight(h1 + 3.0f);
	m_pGameLog->SendMessage(pItem,CHILD_CHANGED_SIZE);
}

void CUIMessagesWindow::Update()
{
	CUIWindow::Update();
}