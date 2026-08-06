#pragma once

#include "UILine.h"
#include "uiabstract.h"

class CUILines : public IUITextControl,
				 public CDeviceResetNotifier {
	 friend class CUICustomEdit;
public:
	CUILines();
	virtual ~CUILines();

	// IUITextControl methods
	virtual void			SetText(const char* text);
			void			SetTextST(const char* text);
	virtual const char*		GetText();

	virtual void			SetTextColor(u32 color);
	virtual u32				GetTextColor()								{return m_dwTextColor;}
	virtual void			SetFont(CGameFont* pFont);
	virtual CGameFont*		GetFont()									{return m_pFont;}
	virtual void			SetTextAlignment(ETextAlignment al)			{m_eTextAlign = al;}
	virtual ETextAlignment	GetTextAlignment()							{return m_eTextAlign;}
			void			SetVTextAlignment(EVTextAlignment al)		{m_eVTextAlign = al;}
			EVTextAlignment GetVTextAlignment()							{return m_eVTextAlign;}

			void			SetTextComplexMode							(bool mode = true);
			void			SetPasswordMode								(bool mode = true);
			bool			IsPasswordMode								() {return !!uFlags.test(flPasswordMode);};

			void			SetColoringMode								(bool mode);
			void			SetCutWordsMode								(bool mode);
			void			SetUseNewLineMode							(bool mode);
			void			SetEllipsis									(bool mode);

			void			Draw										(float x, float y);


    // CDeviceResetNotifier methods
	virtual void			OnDeviceReset								();

	// own methods
			void			Reset										();
			void			ParseText									(bool force=false);
			float			GetVisibleHeight							();

		Fvector2			m_TextOffset;
		Fvector2			m_wndSize;
		Fvector2			m_wndPos;
protected:
				// %c[255,255,255,255]
		u32					GetColorFromText							(const xr_string& str)							const;
		float				GetIndentByAlign							()												const;
		float				GetVIndentByAlign							();
		void				CutFirstColoredTextEntry					(xr_string& entry, u32& color,xr_string& text)	const;
	CUILine*				ParseTextToColoredLine						(const xr_string& str);

	// IUITextControl data
	typedef xr_string						Text;
	typedef xr_vector<CUILine>				LinesVector;
	typedef LinesVector::iterator			LinesVector_it;
	LinesVector				m_lines;	// parsed text

	Text					m_text;

	ETextAlignment			m_eTextAlign;
	EVTextAlignment			m_eVTextAlign;
	u32						m_dwTextColor;

	CGameFont*				m_pFont;

	enum {
		flNeedReparse		= (1<<0),
		flComplexMode		= (1<<1),
		flPasswordMode		= (1<<2),
		flColoringMode		= (1<<3),
		flCutWordsMode		= (1<<4),
		flRecognizeNewLine	= (1<<5),
		flEllipsis			= (1<<6)
	};	
private:
	Flags8					uFlags;
};

class CUILinesOwner : public IUITextControl {
public:
	virtual					~CUILinesOwner() {}

	// IUIFontControl{
	virtual void			SetTextColor					(u32 color)						{m_lines.SetTextColor(color);}
	virtual u32				GetTextColor					()								{return m_lines.GetTextColor();}
	virtual void			SetFont							(CGameFont* pFont)				{m_lines.SetFont(pFont);}
	virtual CGameFont*		GetFont							()								{return m_lines.GetFont();}
	virtual void			SetTextAlignment				(ETextAlignment alignment)		{m_lines.SetTextAlignment(alignment);}
	virtual ETextAlignment	GetTextAlignment				()								{return m_lines.GetTextAlignment();}

	// IUITextControl : public IUIFontControl{
	virtual void			SetText							(const char* text)				{m_lines.SetText(text);}
	virtual const char*		GetText							()								{return m_lines.GetText();}

	// own
	virtual void			SetTextPosX						(float x)						{m_textPos.x = x;}
	virtual void			SetTextPosY						(float y)						{m_textPos.y = y;}

protected:
	Fvector2				m_textPos;
	CUILines				m_lines;
};
