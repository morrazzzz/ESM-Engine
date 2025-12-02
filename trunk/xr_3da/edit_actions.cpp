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
	callback_base::callback_base(Callback const& callback, const SDL_Keymod& mod)
	{
		m_callback = callback;
		keyModCurrentCallback = mod;
	}

	callback_base::~callback_base()
	{
		if (additionalCallbacks.empty())
			return;

		for (u32 i = 0; i < additionalCallbacks.size(); i++)
			delete additionalCallbacks[i];

		additionalCallbacks.clear();
	}

	void callback_base::AddAdditionalCallback(const callback_base* prev_action)
	{
		additionalCallbacks.emplace_back(prev_action);
	}

	bool callback_base::on_key_press() const
	{
		if (pInput->GetModState(keyModCurrentCallback) 
			|| keyModCurrentCallback == SDL_KMOD_NONE && !pInput->GetModState(allKeyModsAdditionalCallback))
		{
			m_callback();
			return true;
		}

		for (u32 i = 0; i < additionalCallbacks.size(); i++)
		{
			auto callback = additionalCallbacks[i];

			if (callback->keyModCurrentCallback == SDL_KMOD_NONE && pInput->GetModState(allKeyModsAdditionalCallback))
				continue;

			if (callback->on_key_press())
				break;
		}

		return false;
	}
}