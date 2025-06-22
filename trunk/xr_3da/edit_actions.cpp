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
}