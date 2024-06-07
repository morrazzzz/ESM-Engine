#include "stdafx.h"
#include "UIVotingCategory.h"
#include "UIXmlInit.h"
#include "UI3tButton.h"
#include "../game_cl_teamdeathmatch.h"
#include "UIKickPlayer.h"
#include "UIChangeMap.h"
#include "UIChangeWeather.h"
#include "UITextVote.h"

#include "../game_sv_mp_vote_flags.h"


CUIVotingCategory::CUIVotingCategory()
{
	xml_doc			= NULL;
	kick			= NULL;
	change_weather	= NULL;
	change_map		= NULL;
	text_vote		= NULL;

	bkgrnd			= xr_new<CUIStatic>(); bkgrnd->SetAutoDelete(true); AttachChild(bkgrnd);
	header			= xr_new<CUIStatic>(); header->SetAutoDelete(true);	AttachChild(header);
	btn_cancel		= xr_new<CUI3tButton>();btn_cancel->SetAutoDelete(true); AttachChild(btn_cancel);

	for (int i = 0; i<7; i++)
	{
		btn[i] = xr_new<CUI3tButton>();
		btn[i]->SetAutoDelete(true);
		AttachChild(btn[i]);


		txt[i] = xr_new<CUIStatic>();
		txt[i]->SetAutoDelete(true);
		AttachChild(txt[i]);
	}
	Init();
}

CUIVotingCategory::~CUIVotingCategory()
{
	xr_delete(kick);
	xr_delete(change_map);
	xr_delete(change_weather);
	xr_delete(text_vote);

	xr_delete(xml_doc);
}

void CUIVotingCategory::Init()
{
	if (!xml_doc)
		xml_doc = xr_new<CUIXml>();

	xml_doc->Init(CONFIG_PATH, UI_PATH, "voting_category.xml");

	CUIXmlInit::InitWindow(*xml_doc, "category", 0, this);

	CUIXmlInit::InitStatic(*xml_doc, "category:header", 0, header);
	CUIXmlInit::InitStatic(*xml_doc, "category:background", 0, bkgrnd);

	string256 _path;
	for (int i = 0; i<7; i++){
		sprintf_s(_path, "category:btn_%d", i + 1);
		CUIXmlInit::Init3tButton(*xml_doc, _path, 0, btn[i]);
		sprintf_s(_path, "category:txt_%d", i + 1);
		CUIXmlInit::InitStatic(*xml_doc, _path, 0, txt[i]);
	}

	CUIXmlInit::Init3tButton(*xml_doc, "category:btn_cancel", 0, btn_cancel);
}

void CUIVotingCategory::SendMessage(CUIWindow* pWnd, s16 msg, void* pData)
{
	if (BUTTON_CLICKED == msg)
	{
		if (btn_cancel == pWnd)
			OnBtnCancel();
		for (int i=0; i<7; i++){
			if (btn[i] == pWnd){
				OnBtn(i);
				return;
			}
		}
	}
}

#include <dinput.h>

bool CUIVotingCategory::OnKeyboardAction(int dik, EUIMessages keyboard_action)
{
	CUIDialogWnd::OnKeyboardAction(dik, keyboard_action);
	
	if (WINDOW_KEY_PRESSED == keyboard_action)
	{
		if (DIK_ESCAPE == dik)
		{
			OnBtnCancel();
			return true;
		}
		if (dik >= DIK_1 && dik <= DIK_7)
			OnBtn(dik - DIK_1);
			return true;
	}
	return false;
}

#include "../../xr_3da/xr_ioconsole.h"

void CUIVotingCategory::OnBtn(int i)
{
}

void CUIVotingCategory::OnBtnCancel()
{
}

void CUIVotingCategory::Update				()
{
	//check buttons state, based on voting mask
	for (int i = 0; i<7; i++)
	{
		u16 flag = 1<<(u16(i+1) & 0xff);
		
		btn[i]->Enable((i==6)?false:Game().IsVotingEnabled(flag));
		txt[i]->Enable((i==6)?false:Game().IsVotingEnabled(flag));		
	}

	inherited::Update();
}
