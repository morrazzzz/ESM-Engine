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
callback_base::callback_base( Callback const& callback, const SDL_Keymod& mod)
{
	m_callback  = callback;
	KeyMod = mod;
}

bool callback_base::on_key_press( )
{
	if (KeyMod == SDL_KMOD_NONE || pInput->GetModState(KeyMod))
	{
		m_callback();
		return true;
	}

	return false;
}
}