////////////////////////////////////////////////////////////////////////////
//	Module 		: edit_actions.cpp
//	Created 	: 04.03.2008
//	Author		: Evgeniy Sokolov
//	Description : edit actions chars class implementation
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "edit_actions.h"
#include "line_edit_control.h"
#include "xr_input.h"
#include <locale.h>

namespace text_editor
{

base::base()
{
}

base::~base()
{
}

// -------------------------------------------------------------------------------------------------

callback_base::callback_base( Callback const& callback, const SDL_Keymod& mod)
{
	m_callback  = callback;
	KeyMod = mod;
	m_previous_action = nullptr;
}

void callback_base::SetPrevCallback(callback_base* prev_action)
{
	m_previous_action = prev_action;
}

void callback_base::on_key_press( line_edit_control* const control )
{
	if (KeyMod == SDL_KMOD_NONE || pInput->GetModState(KeyMod))
	{
		m_callback();
		return;
	}

	if (m_previous_action)
		m_previous_action->on_key_press(control);
}

// -------------------------------------------------------------------------------------------------

type_pair::type_pair(const SDL_Scancode& dik, char c, char c_shift, bool b_translate)
{
	init( dik, c, c_shift, b_translate );
}

type_pair::~type_pair()
{
}

void type_pair::init(const SDL_Scancode& dik, char c, char c_shift, bool b_translate )
{
	m_translate	= b_translate;
	m_dik = dik;
	m_char = c;
	m_char_shift = c_shift;
}


void type_pair::on_key_press( line_edit_control* const control )
{
	char c = 0;
	if( m_translate )
	{
		c				= m_char;
		char c_shift	= m_char_shift;
		string16 buff;
		buff[0]	= 0;
		
		/*
		//setlocale( LC_ALL, "" ); // User-default

		// The following 3 lines looks useless

		LPSTR			loc;
		STRCONCAT		( loc, ".", itoa( GetACP(), code_page, 10 ) );
		setlocale		( LC_ALL, loc );*/

		static _locale_t current_locale = _create_locale(LC_ALL, "");

		strcpy(buff, pInput->GetKeyName(m_dik));
		_strlwr_l(buff, current_locale);
		c = buff[0];

		if (pInput->GetModState(SDL_KMOD_CAPS) || pInput->GetModState(SDL_KMOD_SHIFT))
		{
			_strupr_l(buff, current_locale);
			c_shift = buff[0];
			c = c_shift;
		}
	}
	else
	{
		c = m_char;
		if (pInput->GetModState(SDL_KMOD_CAPS) || pInput->GetModState(SDL_KMOD_SHIFT))
			c = m_char_shift;
	}
	control->insert_character(c);
}

} // namespace text_editor
