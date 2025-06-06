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

// -------------------------------------------------------------------------------------------------

class type_pair : public base
{
public:
	type_pair(const SDL_Scancode& dik, char c, char c_shift, bool b_translate);
	virtual	~type_pair();
	void init(const SDL_Scancode& dik, char c, char c_shift, bool b_translate);
	virtual	void on_key_press(line_edit_control* const control);

private:
	SDL_Scancode m_dik;
	bool	m_translate;
	char	m_char;
	char	m_char_shift;

}; // class type_pair

} // namespace text_editor
