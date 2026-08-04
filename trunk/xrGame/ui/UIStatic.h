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
	friend class CUI3tButton;
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
			void			SetXformLightAnim(LPCSTR lanim, bool bCyclic);
			void			ResetXformAnimation();

			virtual void		DrawTexture();
			virtual void		DrawText();

			void AdjustHeightToText();
			void AdjustWidthToText();

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
	virtual void			SetFont					(CGameFont* pFont);
	virtual CGameFont*		GetFont					();
	virtual void			SetTextAlignment		(ETextAlignment alignment);
	virtual ETextAlignment	GetTextAlignment		();

	// text additional
			void	SetTextComplexMode			(bool md);
			void	SetTextAlign_script			(u32 align);
			u32		GetTextAlign_script			();
			void	SetTextColor_script			(int a, int r, int g, int b){SetTextColor(color_argb(a,r,g,b));}
			u32&	GetTextColorRef				();
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

	virtual void			ColorAnimationSetTextureColor(u32 color, bool only_alpha);
	virtual void			ColorAnimationSetTextColor(u32 color, bool only_alpha);

	// Анализируем текст на помещаемость его по длинне в заданную ширину, и если нет, то всталяем 
	// "\n" реализуем таким образом wordwrap
//	static void PreprocessText				(STRING &str, float width, CGameFont *pFont);
	enum EElipsisPosition
	{
		eepNone,
		eepBegin,
		eepEnd,
		eepCenter
	};

	void SetElipsis							(EElipsisPosition pos, int indent);

	// will be need by CUI3tButton
	// Don't change order!!!!!
	typedef enum {
		E, // enabled
		D, // disabled
		T, // touched
		H  // highlighted
	} E4States;

	void SetTextColor(u32 color, E4States state);
protected:
	CUILines* m_pTextControl;

	// this array of color will be useful in CUI3tButton class
	// but we really need to declare it directly there because it must be initialized in CUIXmlInit::InitStatic
	u32  m_dwTextColor[4];
	bool m_bUseTextColor[4]; // note: 0 index will be ignored

	bool m_bStretchTexture;
	bool m_bTextureEnable;
	CUIStaticItem m_UIStaticItem;

	float			m_fHeading;
	bool			m_bHeading;

    // Для вывода текстуры с обрезанием по маске используем CUIFrameWindow
	Fvector2		m_TextureOffset;

	// Обрезка надписи
	Frect	m_ClipRect;
	EElipsisPosition	m_ElipsisPos;
	void Elipsis(const Frect &rect, EElipsisPosition elipsisPos);
	int	m_iElipsisIndent;
private:
	Frect	m_xxxRect; // need by RescaleRelative2Rect(Frect& r). it is initializes only once in Init(x,y,width,height)

public:
	CUILines* TextItemControl();
	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CUIStatic)
#undef script_type_list
#define script_type_list save_type_list(CUIStatic)
