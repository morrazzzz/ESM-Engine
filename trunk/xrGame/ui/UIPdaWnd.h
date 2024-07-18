#pragma once

#include "UIDialogWnd.h"
#include "UIPdaAux.h"
#include "../encyclopedia_article_defs.h"

class CInventoryOwner;
class CUIFrameLineWnd;
class CUIButton;
class CUITabControl;
class CUIStatic;
class CUIMapWnd;
class CUIEncyclopediaWnd;
class CUIDiaryWnd;
class CUIActorInfoWnd;
class CUIStalkersRankingWnd;
class CUIEventsWnd;
class CUIPdaContactsWnd;

 

class CUIPdaWnd: public CUIDialogWnd
{
private:
	typedef CUIDialogWnd	inherited;
protected:
	//�������� ������������� ����������
	CUIFrameLineWnd*		UIMainButtonsBackground;
	CUIFrameLineWnd*		UITimerBackground;

	// ������ PDA
	CUITabControl*			UITabControl;

	// ���������� ������� �����
	void					UpdateDateTime					();
	void					DrawUpdatedSections				();
protected:
	// ���������
	CUIStatic*				UIMainPdaFrame;
	CUIStatic*				m_updatedSectionImage;
	CUIStatic*				m_oldSectionImage;

	// ������� �������� ������
	CUIWindow*				m_pActiveDialog;
	EPdaTabs				m_pActiveSection;
	xr_vector<Fvector2>		m_sign_places_main;

public:
	// ���������� PDA
	CUIMapWnd*				UIMapWnd;
	CUIPdaContactsWnd*		UIPdaContactsWnd;
	CUIEncyclopediaWnd*		UIEncyclopediaWnd;
	CUIDiaryWnd*			UIDiaryWnd;
	CUIActorInfoWnd*		UIActorInfo;
	CUIStalkersRankingWnd*	UIStalkersRanking;
	CUIEventsWnd*			UIEventsWnd;
	virtual void			Reset				();
public:
							CUIPdaWnd			();
	virtual					~CUIPdaWnd			();

	virtual void 			Init				();

	virtual void 			SendMessage			(CUIWindow* pWnd, s16 msg, void* pData = NULL);

	virtual void 			Draw				();
	virtual void 			Update				();
	virtual void 			Show				();
	virtual void 			Hide				();
	virtual bool			OnMouseAction				(float x, float y, EUIMessages mouse_action) {CUIDialogWnd::OnMouseAction(x,y,mouse_action);return true;} //always true because StopAnyMove() == false
	
	void					SetActiveSubdialog	(EPdaTabs section);
	virtual bool			StopAnyMove			(){return false;}

			void			PdaContentsChanged	(pda_section::part type);
};
