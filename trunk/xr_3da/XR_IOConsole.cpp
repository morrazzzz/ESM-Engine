// XR_IOConsole.cpp: implementation of the CConsole class.
// modify 15.05.2008 sea

#include "stdafx.h"
#include "XR_IOConsole.h"
#include "line_editor.h"

#include "igame_level.h"
#include "igame_persistent.h"

#include "xr_input.h"
#include "xr_ioc_cmd.h"
#include "GameFont.h"

#include "../Include/xrRender/UIRender.h"

static float const UI_BASE_WIDTH	= 1024.0f;
static float const UI_BASE_HEIGHT	= 768.0f;
static float const LDIST            = 0.05f;
static u32   const cmd_history_max  = 64;

static u32 const fps_not_calculated = color_rgba(182, 255, 0, 255);
static u32 const bad_fps_console_color = color_rgba(255, 0, 0, 255);
static u32 const normal_fps_console_color = color_rgba(255, 186, 39, 255);
static u32 const good_fps_console_color = color_rgba(29, 255, 0, 255);

static u32 const prompt_font_color  = color_rgba( 228, 228, 255, 255 );
static u32 const tips_font_color    = color_rgba( 230, 250, 230, 255 );
static u32 const cmd_font_color     = color_rgba( 138, 138, 245, 255 );
static u32 const cursor_font_color  = color_rgba( 255, 0, 0, 255 );
static u32 const total_font_color   = color_rgba( 250, 250,  15, 180 );
static u32 const default_font_color = color_rgba( 250, 250, 250, 250 );

static u32 const back_color         = color_rgba(  20,  20,  20, 200 );
static u32 const tips_back_color    = color_rgba(  20,  20,  20, 200 );
static u32 const tips_select_color  = color_rgba(  90,  90, 140, 230 );
static u32 const tips_word_color    = color_rgba(   5, 100,  56, 200 );
static u32 const tips_scroll_back_color  = color_rgba( 15, 15, 15, 230 );
static u32 const tips_scroll_pos_color   = color_rgba( 70, 70, 70, 240 );


ENGINE_API CConsole*		Console		=	NULL;

extern char const * const	ioc_prompt;
       char const * const	ioc_prompt	=	">>> ";

extern char const * const	ch_cursor;
       char const * const	ch_cursor	=	"|";

text_editor::line_edit_control& CConsole::ec()
{
	return m_editor->control();
}

u32 CConsole::get_mark_color( Console_mark type )
{
	u32 color = default_font_color;
	switch ( type )
	{
	case mark0:  color = color_rgba( 255, 255,   0, 255 ); break;
	case mark1:  color = color_rgba( 255,   0,   0, 255 ); break;
	case mark2:  color = color_rgba( 100, 100, 255, 255 ); break;
	case mark3:  color = color_rgba(   0, 222, 205, 155 ); break;
	case mark4:  color = color_rgba( 255,   0, 255, 255 ); break;
	case mark5:  color = color_rgba( 155,  55, 170, 155 ); break;
	case mark6:  color = color_rgba(  25, 200,  50, 255 ); break;
	case mark7:  color = color_rgba( 255, 255,   0, 255 ); break;
	case mark8:  color = color_rgba( 128, 128, 128, 255 ); break;
	case mark9:  color = color_rgba(   0, 255,   0, 255 ); break;
	case mark10: color = color_rgba(  55, 155, 140, 255 ); break;
	case mark11: color = color_rgba( 205, 205, 105, 255 ); break;
	case mark12: color = color_rgba( 128, 128, 250, 255 ); break;
	case no_mark:
	default: break;
	}
	return color;
}

bool CConsole::is_mark( Console_mark type )
{
	switch ( type )
	{
	case mark0:  case mark1:  case mark2:  case mark3:
	case mark4:  case mark5:  case mark6:  case mark7:
	case mark8:  case mark9:  case mark10: case mark11:	case mark12:
		return true;
		break;
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

CConsole::CConsole()
:m_hShader_back(NULL)
{
	m_editor = new text_editor::line_editor((u32)CONSOLE_BUF_SIZE);
	m_cmd_history_max = cmd_history_max;
	m_disable_tips    = false;
	Register_callbacks();
	Device.seqResolutionChanged.Add(this);
}

void CConsole::Initialize()
{
	scroll_delta	= 0;
	bVisible		= false;
	pFont			= NULL;
	pFont2			= NULL;

	m_last_cmd		= NULL;
	
	m_cmd_history.reserve( m_cmd_history_max + 2 );
	m_cmd_history.clear_not_free();
	reset_cmd_history_idx();

	m_tips.reserve( MAX_TIPS_COUNT + 1 );
	m_tips.clear_not_free();
	m_temp_tips.reserve( MAX_TIPS_COUNT + 1 );
	m_temp_tips.clear_not_free();

	m_tips_mode		= 0;
	m_prev_length_str = 0;
	m_cur_cmd		= NULL;
	reset_selected_tip();

	FPSInConsole = 30.0f;

	// Commands
	extern void CCC_Register();
	CCC_Register();
}

CConsole::~CConsole()
{
	xr_delete( m_hShader_back );
	xr_delete( m_editor );
	Destroy();
	Device.seqResolutionChanged.Remove(this);
}

void CConsole::Destroy()
{
	xr_delete( pFont );
	xr_delete( pFont2 );
	Commands.clear();
}

void CConsole::AddCommand( IConsole_Command* cc )
{
	Commands[cc->Name()] = cc;
}

void CConsole::RemoveCommand( IConsole_Command* cc )
{
	vecCMD_IT it = Commands.find( cc->Name() );
	if ( Commands.end() != it )
	{
		Commands.erase(it);
	}
}

void CConsole::OnFrame()
{
	m_editor->on_frame();
	
	//INFO: Not update for empty line. Otherwise update_tips clearing m_tips vector and return in update_tips
	if (updateTipsInput && ec().lineEditString.empty())
		return; 

	if (Device.dwFrame % 10 == 0)
	{
		if (updateTipsInput && !ec().lineEditString.empty())
			updateTipsInput = false;

		update_tips();
	}
}

void CConsole::OutFont( LPCSTR text, float& pos_y )
{
	float str_length = pFont->SizeOf_( text );
	float scr_width  = 1.98f * (Device.dwWidth * 0.5f);
	if( str_length > scr_width ) //1024.0f
	{
		float f	= 0.0f;
		int sz	= 0;
		int ln	= 0;
		PSTR one_line = (PSTR)_alloca( (CONSOLE_BUF_SIZE + 1) * sizeof(char) );
		
		while( text[sz] && (ln + sz < CONSOLE_BUF_SIZE-5) )// перенос строк
		{
			one_line[ln+sz]   = text[sz];
			one_line[ln+sz+1] = 0;
			
			float t	= pFont->SizeOf_( one_line + ln );
			if ( t > scr_width )
			{
				OutFont		( text + sz + 1, pos_y );
				pos_y		-= LDIST;
				pFont->OutI	( -1.0f, pos_y, "%s", one_line + ln );
				ln			= sz + 1;
				f			= 0.0f;
			}
			else
			{
				f = t;
			}

			++sz;
		}
	}
	else
	{
		pFont->OutI( -1.0f, pos_y, "%s", text );
	}
}

void CConsole::OnScreenResolutionChanged()
{
	xr_delete( pFont );
	xr_delete( pFont2 );
}

void CConsole::OnRender()
{
	if ( !bVisible )
	{
		return;
	}

	if ( !m_hShader_back )
	{
		m_hShader_back = xr_new< FactoryPtr<IUIShader> >();
		(*m_hShader_back)->create( "hud\\default", "ui\\ui_console" ); // "ui\\ui_empty"
	}
	
	if ( !pFont )
	{
		pFont = xr_new<CGameFont>( "hud_font_di", CGameFont::fsDeviceIndependent );
		pFont->SetHeightI(  0.025f );
	}
	if( !pFont2 )
	{
		pFont2 = xr_new<CGameFont>( "hud_font_di2", CGameFont::fsDeviceIndependent );
		pFont2->SetHeightI( 0.025f );
	}
	
	float fMaxY = 0.0f;
	float ypos = fMaxY - LDIST * 1.1f;
	float dwHalfWidth = static_cast<float>(Device.dwWidth * 0.5f);

	float scr_width = 1.9f * dwHalfWidth;
	float ioc_d = pFont->SizeOf_(ioc_prompt);
	float d1 = pFont->SizeOf_("_");

	std::string_view stringView = std::string_view(ec().lineEditString);
	xr_string stringBeforeCurPos(stringView.substr(0, ec().m_cur_pos));
	float sizeOf = !stringView.empty() ? pFont->SizeOf_(stringBeforeCurPos.c_str()) : 0.0f;

	float str_length = ioc_d + sizeOf;
	float out_pos = 0.0f;
	if (str_length > scr_width)
	{
		out_pos -= (str_length - scr_width);
		str_length = scr_width;
	}

	float scr_x = 1.0f / dwHalfWidth;
	float tipsX = -1.0f + (str_length + pFont->SizeOf_(ch_cursor)) * scr_x;

	DrawBackgrounds(-6.0f + str_length + pFont->SizeOf_(ch_cursor));

	//---------------------------------------------------------------------------------

	pFont->SetColor( prompt_font_color );
	pFont->OutI( -1.0f + out_pos * scr_x, ypos, "%s", ioc_prompt );
	out_pos += ioc_d + 2.5f;

	if (!m_disable_tips && m_tips.size())
	{
		pFont->SetColor(tips_font_color);

		/*
		float shift_x = 0.0f;
		switch (m_tips_mode)
		{
		case 0: shift_x = scr_x * 1.0f;			break;
		case 1: shift_x = scr_x * out_pos;		break;
		case 2: shift_x = scr_x * (ioc_d + pFont->SizeOf_(m_cur_cmd.c_str()) + d1);	break;
		case 3: shift_x = scr_x * str_length;	break;
		}
		*/

		vecTipsEx::iterator itb = m_tips.begin() + m_start_tip;
		vecTipsEx::iterator ite = m_tips.end();
		for (u32 i = 0; itb != ite; ++itb, ++i) // tips
		{
			pFont->OutI(tipsX, fMaxY + i * LDIST, "%s", (*itb).text.c_str());
			if (i >= VIEW_TIPS_COUNT - 1)
				break;
		}
	}

	// ===== ==============================================
	pFont->SetColor ( cmd_font_color );
	pFont2->SetColor( cmd_font_color );

	if (!ec().lineEditString.empty())
	{
		pFont->OutI(-1.0f + out_pos * scr_x, ypos, "%s", stringBeforeCurPos.c_str());
		if (ec().m_cur_pos != ec().lineEditString.size())
		{
			out_pos += sizeOf;// + 5.0f;//2.0f;
			xr_string stringAfterCursor(stringView.substr(ec().m_cur_pos));
			pFont->OutI(-1.0f + out_pos * scr_x, ypos, "%s", stringAfterCursor.c_str());
		}
	}

	if( ec().cursor_view() )
	{
		pFont->SetColor( cursor_font_color );
		pFont->OutI( -1.0f + str_length * scr_x, ypos, "%s", ch_cursor );
	}
	
	// ---------------------
	u32 log_line = LogFile->size()-1;
	ypos -= LDIST;
	for( int i = log_line - scroll_delta; i >= 0; --i ) 
	{
		ypos -= LDIST;
		if ( ypos < -1.0f )
		{
			break;
		}
		LPCSTR ls = ((*LogFile)[i]).c_str();
		
		if ( !ls )
		{
			continue;
		}
		Console_mark cm = (Console_mark)ls[0];
		pFont->SetColor( get_mark_color( cm ) );
		//u8 b = (is_mark( cm ))? 2 : 0;
		//OutFont( ls + b, ypos );
		OutFont( ls, ypos );
	}
	
	string16 q;
	itoa( log_line, q, 10 );
	u32 qn = xr_strlen( q );

	if (Device.fTimeDelta > EPS_S)
	{
		float fps = 1.f / Device.fTimeDelta;
		float fOne = 0.3f;
		float fInv = 1.f - fOne;
		FPSInConsole = fInv * FPSInConsole + fOne * fps;

		if (FPSInConsole <= 35.f)
			pFont->SetColor(bad_fps_console_color);
		else if (FPSInConsole <= 60.f)
			pFont->SetColor(normal_fps_console_color);
		else
			pFont->SetColor(good_fps_console_color);
	}
	else
		pFont->SetColor(fps_not_calculated);

	pFont->OutI(0.82f, -0.96f, "FPS: [%3.1f]", FPSInConsole);
	pFont->SetColor(total_font_color);
	pFont->OutI( 0.95f - 0.03f * qn, fMaxY - 2.0f * LDIST, "[%d]", log_line );
		
	pFont->OnRender();
	pFont2->OnRender();
}

void CConsole::DrawBackgrounds(float tipsX)
{
	Frect r;
	r.set(0.0f, 0.0f, float(Device.dwWidth), 0.5f * float(Device.dwHeight));

	UIRender->SetShader(**m_hShader_back);
	// 6 = back, 12 = tips, (VIEW_TIPS_COUNT+1)*6 = highlight_words, 12 = scroll
	UIRender->StartPrimitive(6 + 12 + (VIEW_TIPS_COUNT + 1) * 6 + 12, IUIRender::ptTriList, IUIRender::pttTL);

	DrawRect(r, back_color);

	if (m_tips.size() == 0 || m_disable_tips)
	{
		UIRender->FlushPrimitive();
		return;
	}

	LPCSTR max_str = "xxxxx";
	vecTipsEx::iterator itb = m_tips.begin();
	vecTipsEx::iterator ite = m_tips.end();
	for ( ; itb != ite; ++itb )
	{
		if ( pFont->SizeOf_( (*itb).text.c_str() ) > pFont->SizeOf_( max_str ) )
		{
			max_str = (*itb).text.c_str();
		}
	}

	float w1        = pFont->SizeOf_(ch_cursor);
	float cur_cmd_w = 0.0f;
	if (m_cur_cmd._get())
		cur_cmd_w = pFont->SizeOf_(m_cur_cmd.c_str());
	cur_cmd_w		+= (cur_cmd_w > 0.01f) ? w1 : 0.0f;

	float list_w    = pFont->SizeOf_( max_str ) + 2.0f * w1;

	float font_h    = pFont->CurrentHeight_();
	float tips_h    = _min( m_tips.size(), (u32)VIEW_TIPS_COUNT ) * font_h;
	tips_h			+= ( m_tips.size() > 0 )? 5.0f : 0.0f;

	Frect pr, sr;
	pr.x1 = tipsX /* + cur_cmd_w*/;
	pr.x2 = pr.x1 + list_w;

	pr.y1 = UI_BASE_HEIGHT * 0.5f;
	pr.y1 *= float(Device.dwHeight)/UI_BASE_HEIGHT;

	pr.y2 = pr.y1 + tips_h;

	float select_y = 0.0f;
	float select_h = 0.0f;
	
	if ( m_select_tip >= 0 && m_select_tip < (int)m_tips.size() )
	{
		int sel_pos = m_select_tip - m_start_tip;

		select_y = sel_pos * font_h;
		select_h = font_h; //1 string
	}
	
	sr.x1 = pr.x1;
	sr.y1 = pr.y1 + select_y;

	sr.x2 = pr.x2;
	sr.y2 = sr.y1 + select_h;

	DrawRect( pr, tips_back_color );
	DrawRect( sr, tips_select_color );

	// --------------------------- highlight words --------------------

	if ( m_select_tip < (int)m_tips.size() )
	{
		Frect rFrect;

		vecTipsEx::iterator itbNew = m_tips.begin() + m_start_tip;
		vecTipsEx::iterator iteNew = m_tips.end();
		for ( u32 i = 0; itbNew != iteNew; ++itbNew, ++i ) // tips
		{
			TipString const& ts = (*itbNew);
			if ( (ts.HL_start < 0) || (ts.HL_finish < 0) || (ts.HL_start > ts.HL_finish) )
			{
				continue;
			}
			int    str_size = (int)ts.text.size();
			if ( (ts.HL_start >= str_size) || (ts.HL_finish > str_size) )
			{
				continue;
			}

			if (ts.HL_start == 0 && ts.HL_finish == 0)
				continue;

			rFrect.null();

			std::string_view viewEdit = std::string_view(ts.text.c_str());
			xr_string stringStartHL(viewEdit.substr(0, ts.HL_start));

			constexpr float IndentLeftToRight = 6.f;

			rFrect.x1 = pr.x1 + IndentLeftToRight + pFont->SizeOf_(stringStartHL.c_str());
			rFrect.y1 = pr.y1 + i * font_h;

			xr_string stringFinishHL(viewEdit.substr(0, ts.HL_finish));
			rFrect.x2 = pr.x1 + IndentLeftToRight + pFont->SizeOf_(stringFinishHL.c_str());
			rFrect.y2 = rFrect.y1 + font_h;
//			VERIFY(rFrect.x1 < rFrect.x2);

			DrawRect( rFrect, tips_word_color );

			if ( i >= VIEW_TIPS_COUNT-1 )
			{
				break; // for itb
			}
		}// for itb
	} // if

	// --------------------------- scroll bar --------------------

	u32 tips_sz = m_tips.size();
	if ( tips_sz > VIEW_TIPS_COUNT )
	{
		Frect rb, rs;
		
		rb.x1 = pr.x2;
		rb.y1 = pr.y1;
		rb.x2 = rb.x1 + 2 * w1;
		rb.y2 = pr.y2;
		DrawRect( rb, tips_scroll_back_color );

		VERIFY( rb.y2 - rb.y1 >= 1.0f );
		float back_height = rb.y2 - rb.y1;
		float u_height = (back_height * static_cast<int>(VIEW_TIPS_COUNT))/ float(tips_sz);
		if ( u_height < 0.5f * font_h )
		{
			u_height = 0.5f * font_h;
		}

		//float u_pos = (back_height - u_height) * float(m_start_tip) / float(tips_sz);
		float u_pos = back_height * float(m_start_tip) / float(tips_sz);
		
		//clamp( u_pos, 0.0f, back_height - u_height );
		
		rs = rb;
		rs.y1 = pr.y1 + u_pos;
		rs.y2 = rs.y1 + u_height;
		DrawRect( rs, tips_scroll_pos_color );
	}

	UIRender->FlushPrimitive();
}

void CConsole::DrawRect( Frect const& r, u32 color )
{
	UIRender->PushPoint( r.x1, r.y1, 0.0f, color, 0.0f, 0.0f );
	UIRender->PushPoint( r.x2, r.y1, 0.0f, color, 1.0f, 0.0f );
	UIRender->PushPoint( r.x2, r.y2, 0.0f, color, 1.0f, 1.0f );

	UIRender->PushPoint( r.x1, r.y1, 0.0f, color, 0.0f, 0.0f );
	UIRender->PushPoint( r.x2, r.y2, 0.0f, color, 1.0f, 1.0f );
	UIRender->PushPoint( r.x1, r.y2, 0.0f, color, 0.0f, 1.0f );
}

void CConsole::ExecuteCommand( LPCSTR cmd_str, bool record_cmd )
{
	u32  str_size = xr_strlen( cmd_str );
	PSTR edt   = (PSTR)_alloca( (str_size + 1) * sizeof(char) );
	PSTR first = (PSTR)_alloca( (str_size + 1) * sizeof(char) );
	PSTR last  = (PSTR)_alloca( (str_size + 1) * sizeof(char) );
	
	xr_strcpy( edt, str_size+1, cmd_str );
	edt[str_size] = 0;

	scroll_delta	= 0;
	reset_cmd_history_idx();
	reset_selected_tip();

	text_editor::remove_spaces( edt );
	if ( edt[0] == 0 )
	{
		return;
	}
	if ( record_cmd )
	{
		char c[2];
		c[0] = mark2;
		c[1] = 0;

		if ( m_last_cmd.c_str() == 0 || xr_strcmp( m_last_cmd, edt ) != 0 )
		{
			Log( c, edt );
			add_cmd_history( edt );
			m_last_cmd = edt;
		}
	}
	text_editor::split_cmd( first, last, edt );

	// search
	vecCMD_IT it = Commands.find( first );
	if ( it != Commands.end() )
	{
		IConsole_Command* cc = it->second;
		if ( cc && cc->bEnabled )
		{
			if ( cc->bLowerCaseArgs && (pInput->GetModState(SDL_KMOD_CAPS) || pInput->GetModState(SDL_KMOD_SHIFT)))
			{
				strlwr( last );
			}
			if ( last[0] == 0 )
			{
				if ( cc->bEmptyArgsHandled )
				{
					cc->Execute( last );
				}
				else
				{
					IConsole_Command::TStatus stat;
					cc->Status( stat );
					Msg( "- %s %s", cc->Name(), stat );
				}
			}
			else
			{
				cc->Execute( last );
				if ( record_cmd )
				{
					cc->add_to_LRU( (LPCSTR)last );
				}
			}
		}
		else
		{
			Msg("! Disabled command: [%s]", first);
		}
	}
	else
	{
		Msg("! Unknown command: [%s]", first);
	}

	if ( record_cmd )
	{
		ec().clear_states();
	}
}

void CConsole::Show()
{
	if ( bVisible )
	{
		return;
	}
	bVisible = true;

	ec().clear_states();
	scroll_delta	= 0;
	reset_cmd_history_idx();
	reset_selected_tip();
	update_tips();

	IR_Capture();
	pInput->TextInputStart(this);

	Device.seqRender.Add( this, 1 );
	Device.seqFrame.Add( this );
}

void CConsole::Hide()
{
	if ( !bVisible )
	{
		return;
	}
	if ( g_pGamePersistent && g_dedicated_server )
	{
		return;
	}
//	if  ( g_pGameLevel || 
//		( g_pGamePersistent && g_pGamePersistent->m_pMainMenu && g_pGamePersistent->m_pMainMenu->IsActive() ))

	bVisible = false;
	reset_selected_tip();
	update_tips();

	pInput->TextInputStop();

	Device.seqFrame.Remove( this );
	Device.seqRender.Remove( this );
	IR_Release();
}

void CConsole::IR_OnKeyboardPress(int dik)
{
	if (dik == SDL_SCANCODE_SPACE && pInput->GetModState(SDL_KMOD_CTRL))
	{
		updateTipsInput = true;
		update_tips();
		return;
	}

	m_editor->lineEditorKeyPress(dik);
}	

void CConsole::IR_OnKeyboardHold(int dik)
{
	m_editor->lineEditorKeyHold(dik);
}

void CConsole::IR_OnKeyboardRelease(int dik)
{
	m_editor->lineEditorKeyRelease(dik);
}

void CConsole::IR_OnTextInput(const char* text)
{
	if (pInput->GetModState(SDL_KMOD_CTRL))
	{
//		updateTipsInput = false;
		return;
	}

	m_editor->lineEditorTextInput(text);
}

void CConsole::IR_OnMouseWheel(int direction)
{
	if (direction > 0)
		Prev_log();
	else
		Next_log();
}

void CConsole::FindConsole(std::string_view data_string_to_find)
{
	u32 LogSize_ = LogFile->size() - 1 - scroll_delta;
	bool ResultFind_ = false;
	u32 TotalResultFind_ = 0;

	for (u32 i = 0; i < LogSize_; ++i)
	{
		LPCSTR T = ((*LogFile)[i]).c_str();

		if (!T)
			continue;

		// There is no need to react to past search results and the command entered by the user.
		if (strstr(T, "find_in_console") || strstr(T, "Finding"))
			continue;

		if (strstr(T, data_string_to_find.data()))
		{
			Msg("@ [Finding]: %s", T);
			TotalResultFind_++;

			if (!ResultFind_)
				ResultFind_ = true;
		}
	}

	if (!ResultFind_)
	{
		Msg("@ [Finding]: Nothing was found by by the search: %s ", data_string_to_find.data());
		return;
	}

	Msg("@ [Finding]: Total`s result find: [%d] count string`s finded, search: [%s]", TotalResultFind_, data_string_to_find.data());
}

void CConsole::SelectCommand()
{
	if ( m_cmd_history.empty() )
	{
		return;
	}
	VERIFY( 0 <= m_cmd_history_idx && m_cmd_history_idx < (int)m_cmd_history.size() );
		
	vecHistory::reverse_iterator	it_rb = m_cmd_history.rbegin() + m_cmd_history_idx;
	ec().set_edit( (*it_rb).c_str() );
	reset_selected_tip();
}

void CConsole::Execute( LPCSTR cmd )
{
	ExecuteCommand( cmd, false );
}

void CConsole::ExecuteScript( LPCSTR str )
{
	u32  str_size = xr_strlen( str );
	PSTR buf = (PSTR)_alloca( (str_size + 10) * sizeof(char) );
	xr_strcpy( buf, str_size + 10, "cfg_load " );
	xr_strcat( buf, str_size + 10, str );
	Execute( buf );
}

// -------------------------------------------------------------------------------------------------

IConsole_Command* CConsole::find_next_cmd( LPCSTR in_str, shared_str& out_str )
{
	LPSTR t2;
	STRCONCAT( t2, in_str, " " );

	vecCMD_IT it = Commands.lower_bound( t2 );
	if ( it != Commands.end() )
	{
		IConsole_Command* cc = it->second;
		LPCSTR name_cmd      = cc->Name();
		u32    name_cmd_size = xr_strlen( name_cmd );
		PSTR   new_str       = (PSTR)_alloca( (name_cmd_size + 2) * sizeof(char) );

		xr_strcat( new_str, name_cmd_size + 2, name_cmd );

		out_str._set( (LPCSTR)new_str );
		return cc;
	}
	return NULL;
}

void CConsole::add_internal_cmds( std::string_view in_str, vecTipsEx& out_v )
{
	u32 cur_count = out_v.size();
	if ( cur_count >= MAX_TIPS_COUNT )
		return;

	if (in_str.empty())
	{
		for (auto& it : Commands)
			out_v.emplace_back(it.first, 0, 0);

		return;
	}

	size_t in_sz = in_str.size();

	// word in internal
	for (auto& it: Commands)
	{
		std::string_view name = it.first;
		auto position = name.find(in_str);

		if (position != std::string_view::npos)
		{
			if (std::find(out_v.begin(), out_v.end(), name.data()) == out_v.end())
				out_v.emplace_back(name.data(), position, position + in_sz);
		}
	} // for

	std::sort(out_v.begin(), out_v.end(), [](const TipString& a, const TipString& b) -> bool {
		return a.HL_start < b.HL_start;
		});
}

void CConsole::	update_tips()
{
	m_temp_tips.clear_not_free();
	m_tips.clear_not_free();

	if (!bVisible)
		return;

	u32 cur_length = ec().lineEditString.size();

	if (!updateTipsInput && cur_length == 0)
	{
		m_prev_length_str = 0;
		return;
	}

	LPCSTR cur = ec().lineEditString.c_str();

	if ( m_prev_length_str != cur_length )
	{
		reset_selected_tip();
	}
	m_prev_length_str = cur_length;

	PSTR first = (PSTR)_alloca( (cur_length + 1) * sizeof(char) );
	PSTR last  = (PSTR)_alloca( (cur_length + 1) * sizeof(char) );
	text_editor::split_cmd( first, last, cur );
	
	u32 first_lenght = xr_strlen(first);
	
	if ( (first_lenght > 2) && (first_lenght + 1 <= cur_length) ) // param
	{
		if ( cur[first_lenght] == ' ' )
		{
			if ( m_tips_mode != 2 )
			{
				reset_selected_tip();
			}

			vecCMD_IT it = Commands.find( first );
			if ( it != Commands.end() )
			{
				IConsole_Command* cc = it->second;
				
				u32 mode = 0;
				if ( (first_lenght + 2 <= cur_length) && (cur[first_lenght] == ' ') && (cur[first_lenght+1] == ' ') )
				{
					mode = 1;
					last += 1; // fake: next char
				}

				cc->fill_tips( m_temp_tips, mode );
				m_tips_mode = 2;
				m_cur_cmd._set( first );
				select_for_filter( last, m_temp_tips, m_tips );

				if ( m_tips.size() == 0 )
				{
					m_tips.push_back( TipString( "(empty)" ) );
				}
				if ( (int)m_tips.size() <= m_select_tip )
				{
					reset_selected_tip();
				}
				return;
			}
		}
	}

	// cmd name
	{
		add_internal_cmds( cur, m_tips );
		//add_next_cmds( cur, m_tips );
		m_tips_mode = 1;
	}

	if ( m_tips.size() == 0 )
	{
		m_tips_mode = 0;
		reset_selected_tip();
	}
	if ( (int)m_tips.size() <= m_select_tip )
	{
		reset_selected_tip();
	}
}

void CConsole::select_for_filter( LPCSTR filter_str, vecTips& in_v, vecTipsEx& out_v )
{
	out_v.clear_not_free();
	u32 in_count = in_v.size();
	if ( in_count == 0 || !filter_str )
	{
		return;
	}

	bool all = ( xr_strlen(filter_str) == 0 );

	vecTips::iterator itb = in_v.begin();
	vecTips::iterator ite = in_v.end();
	for ( ; itb != ite ; ++itb )
	{
		shared_str const& str = (*itb);
		if ( all )
		{
			out_v.push_back( TipString( str ) );
		}
		else
		{
			LPCSTR fd_str = strstr( str.c_str(), filter_str );
			if ( fd_str )
			{
				int   fd_sz = str.size() - xr_strlen( fd_str );
				TipString ts( str, fd_sz, fd_sz + xr_strlen(filter_str) );
				out_v.push_back( ts );
			}
		}
	}//for
}
