#pragma once

#include "uiwindow.h"
#include "../UIDialogHolder.h"

class CUIDialogWnd : public CUIWindow  
{
private:
	typedef CUIWindow inherited;
	CDialogHolder*	m_pHolder;
public:
	bool										m_bWorkInPause;
	CUIDialogWnd								();
	virtual ~ CUIDialogWnd						();

	virtual void Show							();
	virtual void Hide							();
	virtual void Update							();

	virtual bool OnKeyboardAction						(int dik, EUIMessages keyboard_action);
	virtual bool OnKeyboardHold					(int dik);

	CDialogHolder* GetHolder					()					{return m_pHolder;};
			void SetHolder						(CDialogHolder* h)	{m_pHolder = h;};
	virtual bool StopAnyMove					(){return true;}
	virtual bool NeedCursor						()const {return true;}
	virtual bool NeedCenterCursor				()const							{return true;}
	virtual bool WorkInPause					()const {return m_bWorkInPause;}
	virtual bool Dispatch						(int cmd, int param)				{return true;}
			void ShowDialog						(bool bDoHideIndicators);
			void HideDialog						();

    virtual bool IR_process						();
};
