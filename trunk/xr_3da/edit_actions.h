#pragma once
#include <SDL3/SDL_keycode.h>
#include <fastdelegate.h>

enum SDL_Scancode;

namespace text_editor
{

class	line_edit_control;

// -------------------------------------------------------------------------------------------------

class callback_base
{
private:
	typedef		fastdelegate::FastDelegate0<void>		Callback;

public:
	callback_base() = default;
	callback_base(Callback const& callback, const SDL_Keymod& mod);
	virtual ~callback_base() = default;
	bool	on_key_press();
	
	bool CheckNoKeyMode() const { return KeyMod == SDL_KMOD_NONE; }
protected:
    SDL_Keymod KeyMod;
	Callback	m_callback;
}; // class callback_base

} // namespace text_editor
