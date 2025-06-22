#pragma once
#include <SDL3/SDL_keycode.h>
#include <fastdelegate.h>

enum SDL_Scancode;

namespace text_editor
{

class	line_edit_control;

class base
{
public:
					base			();
	virtual			~base			();
	virtual void on_key_press(line_edit_control* const control) {}
}; // class base

// -------------------------------------------------------------------------------------------------

class callback_base : public base
{
private:
	typedef		fastdelegate::FastDelegate0<void>		Callback;

public:
	callback_base() = default;
	callback_base(Callback const& callback, const SDL_Keymod& mod);
	virtual ~callback_base() = default;
	virtual	void	on_key_press	( line_edit_control* const control );
	void SetPrevCallback(callback_base* prev_action);

protected:
    SDL_Keymod KeyMod;
	Callback	m_callback;
	callback_base* m_previous_action;

}; // class callback_base

} // namespace text_editor
