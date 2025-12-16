////////////////////////////////////////////////////////////////////////////
//	Module 		: line_edit_control.cpp
//	Created 	: 21.02.2008
//	Author		: Evgeniy Sokolov
//	Description : line edit control class implementation
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "line_edit_control.h"

#include "../xrCore/os_clipboard.h"
#include "../xrGame/object_broker.h"
#include "xr_input.h"

#include "edit_actions.h"
#include <SDL3/SDL_scancode.h>

ENGINE_API float g_console_sensitive = 0.15f;

namespace text_editor
{

static bool terminate_char(char c, bool check_space = false)
{
	switch (c)
	{
	case ' ':
		return check_space;
	case '(':
	case ')':
	case '{':	case '}':
	case '[':	case ']':
	case '<':	case '>':
	case '\'':	case '\"':
	case '=':	case '+':	case '-':	case '*':	case '\\':
	case '/':	case '&':	case '|':
	case '!':	case '@':	case '#':	case '~':	case '`':
	case '$':	case '%':	case '^':
	case ':':	case ';':
	case '?':	case ',':	case '.':
	case '_':
		return true;
	}
	return false;
}

// -------------------------------------------------------------------------------------------------

line_edit_control::line_edit_control( u32 str_buffer_size )
{
	m_edit_str	= NULL;
	m_inserted	= NULL;
	m_undo_buf	= NULL;

	init( str_buffer_size );
}

line_edit_control::~line_edit_control()
{
	xr_free( m_edit_str );
	xr_free( m_inserted );
	xr_free( m_undo_buf );

	for (const auto& action : m_actions)
		delete action.second;

	m_actions.clear();
}

void line_edit_control::clear_states()
{
	m_edit_str[0]	= 0;
	m_undo_buf[0]	= 0;

	lineEditString.clear();

	m_cur_pos		= 0;
	m_select_start	= 0;
	m_p1			= 0;
	m_p2			= 0;

	m_accel				= 1.0f;
	m_cur_time			= 0.0f;
	m_rep_time			= 0.0f;
	m_last_frame_time	= 0;
	m_last_key_time		= 0.0f;

	m_hold_mode			= false;
	m_insert_mode		= false;
	m_repeat_mode		= false;
	m_mark				= false;
	m_cursor_view		= false;
	m_unselected_mode	= false;
}

void line_edit_control::init( u32 str_buffer_size, init_mode mode )
{
	m_buffer_size = str_buffer_size;
	clamp(m_buffer_size, 8, 4096);

	xr_free( m_edit_str );	m_edit_str = (LPSTR)xr_malloc( m_buffer_size * sizeof(char) );
	xr_free( m_inserted );	m_inserted = (LPSTR)xr_malloc( m_buffer_size * sizeof(char) );
	xr_free( m_undo_buf );	m_undo_buf = (LPSTR)xr_malloc( m_buffer_size * sizeof(char) );

	lineEditString.reserve(m_buffer_size);

	clear_states();

	assign_callback(SDL_SCANCODE_A, Callback(this, &line_edit_control::select_all_buf), SDL_KMOD_CTRL);
	assign_callback(SDL_SCANCODE_C, Callback(this, &line_edit_control::copy_to_clipboard), SDL_KMOD_CTRL);
	assign_callback(SDL_SCANCODE_INSERT, Callback(this, &line_edit_control::copy_to_clipboard), SDL_KMOD_CTRL);

	assign_callback(SDL_SCANCODE_HOME, Callback(this, &line_edit_control::move_pos_home));
	assign_callback(SDL_SCANCODE_END, Callback(this, &line_edit_control::move_pos_end));
	assign_callback(SDL_SCANCODE_LEFT, Callback(this, &line_edit_control::move_pos_left));
	assign_callback(SDL_SCANCODE_RIGHT, Callback(this, &line_edit_control::move_pos_right));
	assign_callback(SDL_SCANCODE_LEFT, Callback(this, &line_edit_control::move_pos_left_word), SDL_KMOD_CTRL);
	assign_callback(SDL_SCANCODE_RIGHT, Callback(this, &line_edit_control::move_pos_right_word), SDL_KMOD_CTRL);

	if (mode != im_read_only)
	{
		assign_callback(SDL_SCANCODE_INSERT, Callback(this, &line_edit_control::flip_insert_mode));
		assign_callback(SDL_SCANCODE_Z, Callback(this, &line_edit_control::undo_buf), SDL_KMOD_CTRL);
		assign_callback(SDL_SCANCODE_V, Callback(this, &line_edit_control::paste_from_clipboard), SDL_KMOD_CTRL);
		assign_callback(SDL_SCANCODE_X, Callback(this, &line_edit_control::cut_to_clipboard), SDL_KMOD_CTRL);

		assign_callback(SDL_SCANCODE_INSERT, Callback(this, &line_edit_control::paste_from_clipboard), SDL_KMOD_SHIFT);

		assign_callback(SDL_SCANCODE_BACKSPACE, Callback(this, &line_edit_control::delete_selected_back));
		assign_callback(SDL_SCANCODE_BACKSPACE, Callback(this, &line_edit_control::delete_word_back), SDL_KMOD_CTRL);
		
		assign_callback(SDL_SCANCODE_DELETE, Callback(this, &line_edit_control::delete_selected_forward));
		assign_callback(SDL_SCANCODE_DELETE, Callback(this, &line_edit_control::delete_word_forward), SDL_KMOD_CTRL);
		assign_callback(SDL_SCANCODE_DELETE, Callback(this, &line_edit_control::cut_to_clipboard), SDL_KMOD_SHIFT);

#pragma todo("morrazzzz: Why LSHIFT + CTRL this switch keyboard language? delete?")
		assign_callback(SDL_SCANCODE_LSHIFT, Callback(this, &line_edit_control::SwitchKL), SDL_KMOD_CTRL);
		assign_callback(SDL_SCANCODE_LSHIFT, Callback(this, &line_edit_control::SwitchKL), SDL_KMOD_ALT);
	}
}

void line_edit_control::assign_callback(const SDL_Scancode& key, Callback const& callback, const SDL_Keymod& state)
{
	auto callbackKey = createCallbackBase(callback, state);
	auto it = m_actions.find(key);
	
	if (it != m_actions.end())
	{
		auto nonConstCallback = const_cast<text_editor::callback_base*>(it->second);

		nonConstCallback->allKeyModsAdditionalCallback |= state;
		nonConstCallback->AddAdditionalCallback(callbackKey);

		return;
	}

	m_actions.emplace(key, callbackKey);
}

/*
void line_edit_control::assign_callback(const SDL_Scancode& key, text_editor::callback_base* const main_callback, 
	std::initializer_list<const text_editor::callback_base*> callbacks)
{
	for (auto& const main : callbacks)
		main_callback->AddAdditionalCallback(main);

	m_actions.emplace(key, const_cast<const text_editor::callback_base*>(main_callback));
}
*/

text_editor::callback_base* line_edit_control::createCallbackBase(Callback const& callback, const SDL_Keymod& state)
{
	return new text_editor::callback_base(callback, state);
}

void line_edit_control::set_edit( LPCSTR str )
{
	lineEditString = str;

	m_cur_pos = lineEditString.size();
	m_select_start = m_cur_pos;
	m_accel        = 1.0f;
}

void line_edit_control::InputConsoleText(const char* text)
{
//	clamp_cur_pos();
//	compute_positions();

	u32 text_size = xr_strlen(text) + lineEditString.size();

	if (text_size == (m_buffer_size - 1))
		return;

	bool needUpdateCurPos = m_cur_pos == lineEditString.size();

	if (!m_insert_mode)
		lineEditString.insert(m_cur_pos, text);
	else
		lineEditString.replace(m_cur_pos, 1, text);

	strncat_s(m_edit_str, m_buffer_size,  text, text_size);
	m_edit_str[text_size] = 0;

	if (needUpdateCurPos)
		m_cur_pos = text_size;
	else
		move_pos_right();

	m_accel = 1.0f;
}

// ========================================================

void line_edit_control::on_key_press( int dik )
{
	if ( !m_hold_mode )
	{
		m_last_key_time = 0.0f;
		m_accel = 1.0f;
	}
	m_mark = true;

	auto it = m_actions.find((SDL_Scancode)dik);

	if (it != m_actions.end())
		it->second->on_key_press();

	// ===========
	if ( dik == DIK_LCONTROL || dik == DIK_RCONTROL )
	{
		m_mark = false;	
	}
	
	m_edit_str[m_buffer_size-1] = 0;

	if ( m_mark && (!pInput->GetModState(SDL_KMOD_SHIFT)))
	{
		m_select_start = m_cur_pos;
	}
	compute_positions();

	m_repeat_mode = false;
	m_rep_time    = 0.0f;
}

// -------------------------------------------------------------------------------------------------

void line_edit_control::on_key_hold( int dik )
{
	switch ( dik )
	{
	case DIK_TAB:
//	case DIK_LSHIFT:   case DIK_RSHIFT:
	case DIK_LCONTROL: case DIK_RCONTROL:
	case DIK_LALT:     case DIK_RALT:
		return;
		break;
	}

	if ( m_repeat_mode && m_last_key_time > 5.0f * g_console_sensitive )
	{
		float buf_time = m_rep_time;
		m_hold_mode    = true;
		
		on_key_press( dik );
		
		m_hold_mode    = false;
		m_rep_time     = buf_time;
	}
}

void line_edit_control::on_key_release( int dik )
{
	m_accel         = 1.0f;
	m_rep_time      = 0.0f;
	m_last_key_time = 0.0f;
}

void line_edit_control::on_frame()
{
	u32   fr_time = Device.dwTimeContinual;
	float dt      = (fr_time - m_last_frame_time) * 0.001f;
	if ( dt > 0.06666f )
	{
		dt = 0.06666f;
	}
	m_last_frame_time = fr_time;
	m_cur_time += dt;

	m_cursor_view = true;
	if ( m_cur_time > 0.3f ) { m_cursor_view = false; }
	if ( m_cur_time > 0.4f ) { m_cur_time = 0.0f; }

	m_rep_time += dt * m_accel;
	if ( m_rep_time > g_console_sensitive )//0.2
	{
		m_rep_time    = 0.0f;
		m_repeat_mode = true;
		m_accel       += 0.2f;
	}
	m_last_key_time += dt;
	
	/*if ( Device.dwFrame % 100 == 0 )
	{
	Msg( " cur_time=%.2f  re=%d  acc=%.2f   rep_time=%.2f", cur_time, bRepeat, fAccel, rep_time );
	}*/
}

void line_edit_control::copy_to_clipboard()
{
	if ( m_p1 >= m_p2 )
	{
		return;
	}
	u32 edit_len = xr_strlen( m_edit_str );
	PSTR buf = (PSTR)_alloca( (edit_len + 1) * sizeof(char) );
	strncpy_s( buf, edit_len + 1, m_edit_str + m_p1, m_p2 - m_p1 );
	buf[edit_len] = 0;
	os_clipboard::copy_to_clipboard( buf );
	m_mark = false;
}

void line_edit_control::paste_from_clipboard()
{
	os_clipboard::paste_from_clipboard( m_inserted, m_buffer_size-1 );
}

void line_edit_control::cut_to_clipboard()
{
	copy_to_clipboard();
	delete_selected_forward();
}

// =================================================================================================

void line_edit_control::undo_buf()
{
	xr_strcpy( m_inserted, m_buffer_size, m_undo_buf );
	m_undo_buf[0] = 0;
}

void line_edit_control::select_all_buf()
{
	m_select_start = 0;
	m_cur_pos = (int)xr_strlen( m_edit_str );
	m_mark = false;
}

void line_edit_control::flip_insert_mode()
{
	m_insert_mode = !m_insert_mode;
}


void line_edit_control::delete_selected_back()
{
	delete_selected( true );
}

void line_edit_control::delete_selected_forward()
{
	delete_selected( false );
}

void line_edit_control::delete_selected( bool back )
{
	if (m_cur_pos <= 0)
		return;

	lineEditString.erase(lineEditString.begin() + m_cur_pos - 1);
	move_pos_left();
}

void line_edit_control::delete_word_back()
{
	move_pos_left_word		( );
	compute_positions		( );
	delete_selected			( true );
}

void line_edit_control::delete_word_forward()
{
	move_pos_right_word();
	compute_positions();
	delete_selected( false );
}

void line_edit_control::move_pos_home()
{
	m_cur_pos = 0;
}

void line_edit_control::move_pos_end()
{
	m_cur_pos = (int)xr_strlen( m_edit_str );
}

void line_edit_control::move_pos_left()
{
	if (m_cur_pos - 1 < 0)
		return;

	--m_cur_pos;
}

void line_edit_control::move_pos_right()
{
	if (m_cur_pos + 1 > lineEditString.size())
		return;

	++m_cur_pos;
}

void line_edit_control::move_pos_left_word()
{
	int i = m_cur_pos;

	xr_string subString(std::string_view(lineEditString).substr(0, m_cur_pos));
	for (auto it = subString.rbegin(); it != subString.rend(); ++it)
	{
		if (*it == ' ')
		{
			--i;
			continue;
		}

		if (!terminate_char(*it, true))
		{
			--i;
			continue;
		}

		break;
	}

	m_cur_pos = i - 1;
}

void line_edit_control::move_pos_right_word()
{
	int edit_len = (int)xr_strlen( m_edit_str );
	int i = m_cur_pos + 1;
	while( i < edit_len && !terminate_char( m_edit_str[i], true ) )	{ ++i; }
	//while( i < edit_len && terminate_char( m_edit_str[i] ) )		{ ++i; }
	while( i < edit_len && m_edit_str[i] == ' ' )					{ ++i; }
	m_cur_pos = i;
}

void line_edit_control::compute_positions()
{
	m_p1 = m_cur_pos;
	m_p2 = m_cur_pos;
	if ( m_unselected_mode )
	{
		return;
	}

	if( m_cur_pos > m_select_start )
	{
		m_p1 = m_select_start;
	}
	else if( m_cur_pos < m_select_start )
	{
		m_p2 = m_select_start;
	}
}

void line_edit_control::SwitchKL()
{
	ActivateKeyboardLayout( (HKL)HKL_NEXT, 0 );
}

// -------------------------------------------------------------------------------------------------

void remove_spaces( PSTR str ) // in & out
{
	u32 str_size = xr_strlen( str );
	if ( str_size < 1 )
	{
		return;
	}
	PSTR new_str = (PSTR)_alloca( (str_size + 1) * sizeof(char) );
	new_str[0] = 0;

	u32 a = 0, b = 0, i = 0;
	while ( b < str_size )
	{
		a = b;
		while ( a < str_size && str[a] == ' ' ) { ++a; }
		b = a;
		while ( b < str_size && str[b] != ' ' ) { ++b; }
		strncpy_s( new_str + i, str_size+1 - i, str + a, b - a );
		i += (b-a);
		if ( i < str_size ) 
		{
			new_str[i] = ' ';
		}
		++b;
		++i;
	}
	--i;
	if ( i < str_size )
	{
		strncpy_s( str, str_size, new_str, i );
	}
}

void split_cmd( PSTR first, PSTR second, LPCSTR str )
{
	first[0] = 0;
	second[0] = 0;
	u32 str_size = xr_strlen( str );
	if ( str_size < 1 )
	{
		return;
	}

	// split into =>>(cmd) (params)
	u32 a = 0;
	while ( a < str_size && str[a] != ' ' ) { ++a; }
	strncpy_s( first, str_size+1, str, a );
	if ( a < str_size )		{ first[a] = 0; }	else	{ first[str_size] = 0; }
	++a;
	if ( a < str_size )
	{
		strncpy_s( second, str_size+1, str + a, str_size - a );
		second[str_size - a] = 0;
	}
}

} // namespace text_editor