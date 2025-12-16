////////////////////////////////////////////////////////////////////////////
//	Module 		: XR_IOConsole_callback.cpp
//	Created 	: 17.05.2008
//	Author		: Evgeniy Sokolov
//	Description : Console`s callback functions class implementation
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XR_IOConsole.h"

#include "line_editor.h"
#include "edit_actions.h"
#include "xr_ioc_cmd.h"
#include "xr_input.h"

void CConsole::Register_callbacks()
{
	ec().assign_callback(SDL_SCANCODE_RETURN, Callback(this, &CConsole::Execute_cmd));
	ec().assign_callback(SDL_SCANCODE_KP_ENTER, Callback(this, &CConsole::Execute_cmd));
	
	ec().assign_callback(SDL_SCANCODE_ESCAPE, Callback(this, &CConsole::Hide_cmd_esc));
	ec().assign_callback(SDL_SCANCODE_GRAVE, Callback(this, &CConsole::Hide_cmd));

	ec().assign_callback(SDL_SCANCODE_HOME, Callback(this, &CConsole::Begin_tips), SDL_KMOD_ALT);
	ec().assign_callback(SDL_SCANCODE_END, Callback(this, &CConsole::End_tips), SDL_KMOD_ALT);

	ec().assign_callback(SDL_SCANCODE_UP, Callback(this, &CConsole::Prev_tip));
	ec().assign_callback(SDL_SCANCODE_UP, Callback(this, &CConsole::Prev_cmd), SDL_KMOD_CTRL);

	ec().assign_callback(SDL_SCANCODE_DOWN, Callback(this, &CConsole::Next_tip));
	ec().assign_callback(SDL_SCANCODE_DOWN, Callback(this, &CConsole::Next_cmd), SDL_KMOD_CTRL);

	ec().assign_callback(SDL_SCANCODE_PRIOR, Callback(this, &CConsole::Prev_log));
	ec().assign_callback(SDL_SCANCODE_PRIOR, Callback(this, &CConsole::Begin_log), SDL_KMOD_CTRL);
	ec().assign_callback(SDL_SCANCODE_PRIOR, Callback(this, &CConsole::PageUp_tips), SDL_KMOD_ALT);

	ec().assign_callback(SDL_SCANCODE_PAGEDOWN, Callback(this, &CConsole::Next_log));
	ec().assign_callback(SDL_SCANCODE_PAGEDOWN, Callback(this, &CConsole::End_log), SDL_KMOD_CTRL);
	ec().assign_callback(SDL_SCANCODE_PAGEDOWN, Callback(this, &CConsole::PageDown_tips), SDL_KMOD_ALT);

	ec().assign_callback(SDL_SCANCODE_TAB, Callback(this, &CConsole::Find_cmd));
	ec().assign_callback(SDL_SCANCODE_TAB, Callback(this, &CConsole::Find_cmd_back), SDL_KMOD_SHIFT);
	ec().assign_callback(SDL_SCANCODE_TAB, Callback(this, &CConsole::GamePause), SDL_KMOD_ALT); //Need me??
}

void CConsole::Prev_log() // DIK_PRIOR=PAGE_UP
{
	scroll_delta++;
	if ( scroll_delta > int(LogFile->size())-1 )
	{
		scroll_delta = LogFile->size()-1;
	}
}

void CConsole::Next_log() // DIK_NEXT=PAGE_DOWN
{
	scroll_delta--;
	if ( scroll_delta < 0 )
	{
		scroll_delta = 0;
	}
}

void CConsole::Begin_log() // PAGE_UP+Ctrl
{
	scroll_delta = LogFile->size()-1;
}

void CConsole::End_log() // PAGE_DOWN+Ctrl
{
	scroll_delta = 0;
}

void CConsole::Find_cmd() // DIK_TAB
{
	shared_str out_str;
		
	IConsole_Command* cc = find_next_cmd(ec().lineEditString.c_str(), out_str );
	if ( cc && out_str.size() )
	{
		ec().set_edit( out_str.c_str() );
	}
}

void CConsole::Find_cmd_back() // DIK_TAB+shift
{
	LPCSTR edt = ec().lineEditString.c_str();

	vecCMD_IT it = Commands.lower_bound(edt);
	if ( it != Commands.begin() )
	{
		--it;
		IConsole_Command& cc = *(it->second);
		ec().set_edit(cc.Name());
	}
}

void CConsole::Prev_cmd() // DIK_UP + Ctrl
{
	prev_cmd_history_idx();
	SelectCommand();
}

void CConsole::Next_cmd() // DIK_DOWN + Ctrl
{
	next_cmd_history_idx();
	SelectCommand();
}

void CConsole::Prev_tip() // DIK_UP
{
	if (!updateTipsInput && ec().lineEditString.empty())
	{
		prev_cmd_history_idx();
		SelectCommand();
		return;
	}
	prev_selected_tip();
}

void CConsole::Next_tip() // DIK_DOWN
{
	if (!updateTipsInput && ec().lineEditString.empty())
	{
		next_cmd_history_idx();
		SelectCommand();
		return;
	}
	next_selected_tip();
}

void CConsole::Begin_tips()
{
	m_select_tip = 0;
	m_start_tip = 0;
}

void CConsole::End_tips()
{
	m_select_tip = m_tips.size() - 1;
	m_start_tip = m_select_tip - VIEW_TIPS_COUNT + 1;
	check_next_selected_tip();
}

void CConsole::PageUp_tips()
{
	m_select_tip -= VIEW_TIPS_COUNT;
	check_prev_selected_tip();
}

void CConsole::PageDown_tips()
{
	m_select_tip += VIEW_TIPS_COUNT;
	check_next_selected_tip();
}

void CConsole::Execute_cmd() // DIK_RETURN, DIK_NUMPADENTER
{
	if (0 <= m_select_tip && m_select_tip < (int)m_tips.size())
	{
		xr_string tempString{};
		shared_str const& str = m_tips[m_select_tip].text;
		if ( m_tips_mode == 1 )
		{
			tempString = str.c_str();
			tempString += " ";
		}
		else if ( m_tips_mode == 2 )
		{
			tempString = m_cur_cmd.c_str();
			tempString += " ";
			tempString += str.c_str();
		}

		ec().set_edit(tempString.c_str());
		reset_selected_tip();

		m_tips.clear();
		m_temp_tips.clear();
	}
	else
	{
		ExecuteCommand( ec().lineEditString.c_str(), !pInput->GetPressedKey(SDL_SCANCODE_F1));
	}
	m_disable_tips = false;
}

void CConsole::Show_cmd()
{
	Show();
}

void CConsole::Hide_cmd()
{
	Hide();
}

void CConsole::Hide_cmd_esc()
{
	if ( 0 <= m_select_tip && m_select_tip < (int)m_tips.size() )
	{
		m_disable_tips = true;
		return;
	}
	Hide();
}

void CConsole::GamePause()
{

}