#pragma once

#include <SDL3/SDL_keycode.h>

enum SDL_Scancode;

namespace text_editor
{

void remove_spaces( PSTR str ); // in & out
void split_cmd( PSTR first, PSTR second, LPCSTR str );

class callback_base;

enum init_mode
{
	im_standart = 0,
	im_number_only,
	im_read_only,
	im_file_name_mode, // not "/\\:*?\"<>|^()[]%" 

	im_count
};// init_mode


class ENGINE_API line_edit_control
{
	typedef  fastdelegate::FastDelegate0<void>	Callback;
public:
					line_edit_control	( u32 str_buffer_size );
			void	init				( u32 str_buffer_size, init_mode mode = im_standart );
					~line_edit_control	();

			void	clear_states		();
			void	on_key_press		( int dik );
			void	on_key_hold			( int dik );
			void	on_key_release		( int dik );
			void	on_frame			();

	void assign_callback(const SDL_Scancode& key, Callback const& callback, const SDL_Keymod& state = SDL_KMOD_NONE);
//	void assign_callback(const SDL_Scancode& key, text_editor::callback_base* const main_callback, 
//		std::initializer_list<const text_editor::callback_base*> callbacks);

	inline text_editor::callback_base* createCallbackBase(Callback const& callback, const SDL_Keymod& state = SDL_KMOD_NONE);

	IC	bool		cursor_view			()	const	{ return m_cursor_view; }

	IC	LPCSTR		str_edit			()	const	{ return m_edit_str; }

		void		set_edit			( LPCSTR str );
		void		set_selected_mode	( bool status )		{ m_unselected_mode = !status; }
		bool		get_selected_mode	() const			{ return !m_unselected_mode; }


	void InputConsoleText(const char* text);

	int	m_cur_pos;
	xr_string lineEditString{};
	bool needUpdateCurPos{};
private:
	line_edit_control(line_edit_control const&);

	void xr_stdcall	undo_buf			();
	void xr_stdcall	select_all_buf		();
	void xr_stdcall flip_insert_mode	();

	void xr_stdcall	copy_to_clipboard	();
	void xr_stdcall	paste_from_clipboard();
	void xr_stdcall cut_to_clipboard	();

	void xr_stdcall	move_pos_home		();
	void xr_stdcall	move_pos_end		();
	void xr_stdcall move_pos_left		();
	void xr_stdcall	move_pos_right		();
	void xr_stdcall move_pos_left_word	();
	void xr_stdcall	move_pos_right_word	();

	void xr_stdcall	delete_selected_back();
	void xr_stdcall delete_selected_forward();
	void xr_stdcall	delete_word_back	();
	void xr_stdcall	delete_word_forward	();
	void xr_stdcall SwitchKL			();

//			void	add_inserted_text	();

			void	delete_selected		( bool back );
			void	compute_positions	();
private:
	xr_unordered_map<SDL_Scancode, const text_editor::callback_base*> m_actions;

	char*			m_edit_str;
	char*			m_undo_buf;
	char*			m_inserted;
	int				m_buffer_size;

	int				m_select_start;
	int				m_p1;
	int				m_p2;

	float			m_accel;
	float			m_cur_time;
	float			m_rep_time;
	float			m_last_key_time;
	u32				m_last_frame_time;

	bool			m_hold_mode;
	bool			m_insert_mode;
	bool			m_repeat_mode;
	bool			m_mark;
	bool			m_cursor_view;
	bool			m_need_update;
	bool			m_unselected_mode;
}; // class line_edit_control

} // namespace text_editor

