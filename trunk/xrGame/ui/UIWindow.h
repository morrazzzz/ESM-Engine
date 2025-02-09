#pragma once
#include "../xr_level_controller.h"
class CUIWindow;

template <typename T, typename allocator = xalloc<T>>
using ui_list = std::list<T, allocator>;

using WINDOW_LIST = ui_list<CUIWindow*>;
using WINDOW_LIST_it = WINDOW_LIST::iterator;

#define DEF_UILIST(N,T)		typedef ui_list< T > N;			typedef N::iterator N##_it;

//////////////////////////////////////////////////////////////////////////

#include "UIMessages.h"
#include "../script_export_space.h"
#include "uiabstract.h"


class CUIWindow  : public CUISimpleWindow
{
public:
	using CUISimpleWindow::Init;

				CUIWindow						();
	virtual		~CUIWindow						();

	////////////////////////////////////
	//�������������
	virtual void			Init				(Frect* pRect);

	////////////////////////////////////
	//������ � ��������� � ������������� ������
	virtual void			AttachChild			(CUIWindow* pChild);
	virtual void			DetachChild			(CUIWindow* pChild);
	virtual bool			IsChild				(CUIWindow* pChild) const;
	virtual void			DetachAll			();
	size_t					GetChildNum			()								{return m_ChildWndList.size();} 

	void					SetParent			(CUIWindow* pNewParent);
	CUIWindow*				GetParent			()	const							{return m_pParentWnd;}
	
	//�������� ���� ������ �������� ������
	CUIWindow*				GetTop				()								{if(m_pParentWnd == NULL) return  this; 
																				else return  m_pParentWnd->GetTop();}
	CUIWindow*				GetCurrentMouseHandler();
	CUIWindow*				GetChildMouseHandler();


	//������� �� ������� ������ ��������� �������� ����
	bool					BringToTop			(CUIWindow* pChild);

	//������� �� ������� ������ ���� ��������� ���� � ��� ������
	void					BringAllToTop		();
	


	virtual bool 			OnMouseAction(float x, float y, EUIMessages mouse_action);
	virtual void 			OnMouseMove			();
	virtual void 			OnMouseScroll		(float iDirection);
	virtual bool 			OnDbClick			();
	virtual bool 			OnMouseDown			(int mouse_btn);
	virtual void 			OnMouseUp			(int mouse_btn);
	virtual void 			OnFocusReceive		();
	virtual void 			OnFocusLost			();
			bool 			HasChildMouseHandler();

	//���������/���������� ���� �����
	//��������� ���������� �������� ����� �������������
	void					SetCapture			(CUIWindow* pChildWindow, bool capture_status);
	CUIWindow*				GetMouseCapturer	()													{return m_pMouseCapturer;}

	//������, �������� ������������ ���������,
	//���� NULL, �� ���� �� GetParent()
	void					SetMessageTarget	(CUIWindow* pWindow)								{m_pMessageTarget = pWindow;}
	CUIWindow*				GetMessageTarget	();

	//������� �� ����������
	virtual bool			OnKeyboardAction			(int dik, EUIMessages keyboard_action);
	virtual bool			OnKeyboardHold		(int dik);
	virtual void			SetKeyboardCapture	(CUIWindow* pChildWindow, bool capture_status);

	
	
	//��������� ��������� �� �������������� ������������ �������������
	//�-��� ������ ����������������
	//pWnd - ��������� �� ����, ������� ������� ���������
	//pData - ��������� �� �������������� ������, ������� ����� ������������
	virtual void			SendMessage			(CUIWindow* pWnd, s16 msg, void* pData = NULL);
	
	

	//����������/���������� �� ���� � ����������
	virtual void			Enable				(bool status)									{m_bIsEnabled=status;}
	virtual bool			IsEnabled			()												{return m_bIsEnabled;}

	//������/�������� ���� � ��� �������� ����
	virtual void			Show				(bool status)									{SetVisible(status); Enable(status); }
	IC		bool			IsShown				()												{return this->GetVisible();}
			void			ShowChildren		(bool show);
	
	//���������� ����������
	IC void					GetAbsoluteRect		(Frect& r) ;
	IC void					GetAbsolutePos		(Fvector2& p) 	{Frect abs; GetAbsoluteRect(abs); p.set(abs.x1,abs.y1);}


			void			SetWndRect_script(float x, float y, float width, float height)		{CUISimpleWindow::SetWndRect(x,y,width,height);}
			void			SetWndRect_script(Frect rect)										{CUISimpleWindow::SetWndRect(rect);}

	//���������� ����
	virtual void			Draw				();
	virtual void			Draw				(float x, float y);
	//���������� ���� ����������������
	virtual void			Update				();


			void			SetPPMode			();
			void			ResetPPMode			();
	IC		bool			GetPPMode			()		{return m_bPP;};
	//��� �������� ���� � �������� � �������� ���������
	virtual void			Reset				();
			void			ResetAll			();


	//��������!!!! (� ����� ��� � ���)
	virtual void			SetFont				(CGameFont* pFont)			{ m_pFont = pFont;}
	CGameFont*				GetFont				()							{if(m_pFont) return m_pFont;
																				if(m_pParentWnd== NULL)	
																					return  m_pFont;
																				else
																					return  m_pParentWnd->GetFont();}

	DEF_UILIST				(WINDOW_LIST, CUIWindow*);
	WINDOW_LIST&			GetChildWndList		()							{return m_ChildWndList; }


	IC bool					IsAutoDelete		()							{return m_bAutoDelete;}
	IC void					SetAutoDelete		(bool auto_delete)			{m_bAutoDelete = auto_delete;}

	// Name of the window
	const shared_str		WindowName			() const					{ return m_windowName; }
	void					SetWindowName		(LPCSTR wn)					{ m_windowName = wn; }
	LPCSTR					WindowName_script	()							{return *m_windowName;}
	CUIWindow*				FindChild			(const shared_str name);

	IC bool					CursorOverWindow	() const					{ return m_bCursorOverWindow; }

protected:
	IC void					SafeRemoveChild(CUIWindow* child)				{WINDOW_LIST_it it = std::find(m_ChildWndList.begin(),m_ChildWndList.end(),child); if(it!=m_ChildWndList.end())m_ChildWndList.erase(it);};

	shared_str				m_windowName;
	//������ �������� ����
	WINDOW_LIST				m_ChildWndList;
	
	//��������� �� ������������ ����
	CUIWindow*				m_pParentWnd;

	//�������� ���� �������, ��������� ���� ����
	CUIWindow*				m_pMouseCapturer;
	
	//��� ���������� ����������
	//������ ������, ������ �� ������
	//����� ���� ����� � ����������
	CUIWindow*				m_pOrignMouseCapturer;

	//�������� ���� �������, ��������� ���� ����������
	CUIWindow*				m_pKeyboardCapturer;

	//���� ���� ���������
	CUIWindow*				m_pMessageTarget;


	CGameFont*				m_pFont;

	// ��������� ������� �����
	Fvector2 cursor_pos;

	//����� �������� ����� �����
	//��� ����������� DoubleClick
	u32						m_dwLastClickTime;
	u32						m_dwFocusReceiveTime;

#ifdef DEBUG
	Flags32					m_dbg_flag;
	int m_dbg_id;
#endif

	//���� ��������������� �������� �� ����� ������ �����������
	bool					m_bAutoDelete;

	// ���� �����������/����������� ��������� ���������
	bool					m_bPP;
	//�������� �� ���� ������������
	bool					m_bIsEnabled;

	// ���� ������ ��� �����
	bool					m_bCursorOverWindow;
	bool					m_bClickable;
public:
	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CUIWindow)
#undef script_type_list
#define script_type_list save_type_list(CUIWindow)
