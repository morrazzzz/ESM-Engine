#pragma once

#include "UILanimController.h"
#include "../uistaticitem.h"
#include "../script_export_space.h"
#include "uilines.h"

class CUIFrameWindow;
class CLAItem;
class CUIXml;

struct lanim_cont{
	CLAItem*				m_lanim;
	float					m_lanim_start_time;
	float					m_lanim_delay_time;
	Flags8					m_lanimFlags;
	void					set_defaults		();
};

struct lanim_cont_xf :public lanim_cont{
	Fvector2				m_origSize;
	void					set_defaults		();
};

class CUIStatic : public CUIWindow, public ITextureOwner, public CUILightAnimColorConrollerImpl, public IUITextControl
{
	friend class CUIXmlInit;
private:
	typedef CUIWindow inherited;
	lanim_cont_xf			m_lanim_xform;
	void					EnableHeading_int		(bool b)				{m_bHeading = b;}
public:
	using CUISimpleWindow::SetWndRect;

					CUIStatic				();
	virtual			~CUIStatic				();

	virtual void	Init					(float x, float y, float width, float height);
	virtual void	Draw					();
	virtual void	Update					();	

	virtual void		CreateShader				(const char* tex, const char* sh = "hud\\default");
	virtual ui_shader& GetShader					();

	virtual void		SetTextureColor				(u32 color);
	virtual u32			GetTextureColor				() const;
	virtual void		SetTextureRect				(const Frect& r)			{m_UIStaticItem.SetTextureRect(r);}
	virtual const Frect& GetTextureRect() const { return m_UIStaticItem.GetTextureRect(); }

	virtual void		InitTexture(LPCSTR tex_name);
	virtual void		InitTextureEx(LPCSTR tex_name, LPCSTR sh_name = "hud\\default");
	CUIStaticItem*		GetStaticItem				()							{return &m_UIStaticItem;}
	//void			SetTextureRect_script(Frect* pr) { m_UIStaticItem.SetTextureRect(*pr); }
	//const	Frect* GetTextureRect_script()

	void			SetHeadingPivot(const Fvector2& p, const Fvector2& offset, bool fixedLT) { m_UIStaticItem.SetHeadingPivot(p, offset, fixedLT); }
	//void			ResetHeadingPivot() { m_UIStaticItem.ResetHeadingPivot(); }
	virtual void		SetTextureOffset(float x, float y) { m_TextureOffset.set(x, y); }
	Fvector2	GetTextureOffeset() const { return m_TextureOffset; }
	void		TextureOn() { m_bTextureEnable = true; }
	void		TextureOff() { m_bTextureEnable = false; }

			void		SetVTextAlignment(EVTextAlignment al);
	virtual void		SetColor					(u32 color)					{ m_UIStaticItem.SetTextureColor(color);		}
	u32					GetColor					() const					{ return m_UIStaticItem.GetTextureColor();		}

	// own
			void			SetXformLightAnim		(LPCSTR lanim, bool bCyclic);
			void			ResetXformAnimation		();

	virtual void			DrawTexture				();
	virtual void			DrawText				();

			void 			AdjustHeightToText		();
			void 			AdjustWidthToText		();

	virtual void		Init						(LPCSTR tex_name, float x, float y, float width, float height);	
			void		InitEx						(LPCSTR tex_name, LPCSTR sh_name, float x, float y, float width, float height);

	virtual void		OnFocusReceive				();
	virtual void		OnFocusLost					();

	//IUITextControl
	virtual void			SetText					(LPCSTR str);
			void			SetTextST				(LPCSTR str_id);
	virtual LPCSTR			GetText					();
	virtual void			SetTextColor			(u32 color);
	virtual u32				GetTextColor			();
//	virtual void			SetFont					(CGameFont* pFont);
//	virtual CGameFont*		GetFont					();
//	virtual void			SetTextAlignment		(ETextAlignment alignment);
//	virtual ETextAlignment	GetTextAlignment		();

	// text additional
			void	SetTextComplexMode			(bool md);
			void	SetTextAlign_script			(u32 align);
			u32		GetTextAlign_script			();
			void	SetTextColor_script			(int a, int r, int g, int b){SetTextColor(color_argb(a,r,g,b));}
//#pragma todo("Satan->Satan : delete next two functions")
//	virtual void			SetTextAlign		(CGameFont::EAligment align);
//	CGameFont::EAligment	GetTextAlign		();
	
	void			SetShader				(const ui_shader& sh);
	CUIStaticItem&	GetUIStaticItem			()						{return m_UIStaticItem;}

	void		SetStretchTexture			(bool stretch_texture)	{m_bStretchTexture = stretch_texture;}
	bool		GetStretchTexture			()						{return m_bStretchTexture;}

	void			SetHeading(float f) { m_fHeading = f; };
	float			GetHeading() { return m_fHeading; }
	bool			Heading() { return m_bHeading; }
	void			EnableHeading(bool b) { m_bHeading = b; }

	void			SetConstHeading			(bool b)				{m_bConstHeading = b;};
	bool			GetConstHeading			()						{return m_bConstHeading;}

	virtual void			ColorAnimationSetTextureColor(u32 color, bool only_alpha);
	virtual void			ColorAnimationSetTextColor(u32 color, bool only_alpha);
protected:
	CUILines* m_pTextControl;

	bool m_bStretchTexture;
	bool m_bTextureEnable;
	CUIStaticItem m_UIStaticItem;

	bool			m_bHeading;
	bool			m_bConstHeading;
	float			m_fHeading;

	Fvector2		m_TextureOffset;
public:
	CUILines* TextItemControl();
	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CUIStatic)
#undef script_type_list
#define script_type_list save_type_list(CUIStatic)

class CUITextWnd :public CUIWindow, public CUILightAnimColorConrollerImpl
{
	typedef CUIWindow	inherited;
	CUILines			m_lines;
public:
						CUITextWnd				();
	virtual				~CUITextWnd				(){};
	virtual void		Draw					();
	virtual void		Update					();

			void 		AdjustHeightToText		();
			void 		AdjustWidthToText		();

			void		SetText					(LPCSTR txt)				{TextItemControl().SetText(txt);}
			void		SetTextST				(LPCSTR txt)				{TextItemControl().SetTextST(txt);}
			LPCSTR		GetText					()							{return TextItemControl().GetText();}
			void		SetFont					(CGameFont* F)				{TextItemControl().SetFont(F);}
			CGameFont*	GetFont					()							{return TextItemControl().GetFont();}
			void		SetTextColor			(u32 color)					{TextItemControl().SetTextColor(color);}
			u32			GetTextColor			()							{return TextItemControl().GetTextColor();}
			void		SetTextComplexMode		(bool mode = true)			{TextItemControl().SetTextComplexMode(mode);}
			void		SetTextAlignment		(ETextAlignment al)			{TextItemControl().SetTextAlignment(al);}
			void		SetVTextAlignment		(EVTextAlignment al)		{TextItemControl().SetVTextAlignment(al);}
			void		SetEllipsis				(bool mode)					{TextItemControl().SetEllipsis(mode);}
			void		SetCutWordsMode			(bool mode)					{TextItemControl().SetCutWordsMode(mode);}
			void		SetTextOffset			(float x, float y)			{TextItemControl().m_TextOffset.x = x; TextItemControl().m_TextOffset.y = y;}

	virtual void		ColorAnimationSetTextColor(u32 color, bool only_alpha);

	CUILines&			TextItemControl			()							{return m_lines;}
};