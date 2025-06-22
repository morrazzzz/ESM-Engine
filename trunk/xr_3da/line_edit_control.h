#pragma once

#include <SDL3/SDL_keycode.h>

enum SDL_Scancode;

namespace text_editor
{

void remove_spaces( PSTR str ); // in & out
void split_cmd( PSTR first, PSTR second, LPCSTR str );

class base;
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
private:
	typedef  text_editor::base						Base;
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
	void assign_callback(const SDL_Scancode& key, text_editor::callback_base* first, text_editor::callback_base* second = nullptr);

			void	insert_character	( char c );

	IC	bool		cursor_view			()	const	{ return m_cursor_view; }
	IC	bool		need_update			()	const	{ return m_need_update; }

	IC	LPCSTR		str_edit			()	const	{ return m_edit_str; }
	IC	LPCSTR		str_before_cursor	()	const	{ return m_buf0; }
	IC	LPCSTR		str_before_mark		()	const	{ return m_buf1; }
	IC	LPCSTR		str_mark			()	const	{ return m_buf2; }
	IC	LPCSTR		str_after_mark		()	const	{ return m_buf3; }

		void		set_edit			( LPCSTR str );
		void		set_selected_mode	( bool status )		{ m_unselected_mode = !status; }
		bool		get_selected_mode	() const			{ return !m_unselected_mode; }


	void InputConsoleText(const char* text);
private:
					line_edit_control	( line_edit_control const& );
	line_edit_control const& operator=	( line_edit_control const& );

			void	update_bufs			();

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

			void	clear_inserted		();
			bool	empty_inserted		();

			void	add_inserted_text	();

			void	delete_selected		( bool back );
			void	compute_positions	();
			void	clamp_cur_pos		();
private:
	xr_unordered_map<SDL_Scancode, Base*> m_actions;

	char*			m_edit_str;
	char*			m_undo_buf;
	char*			m_inserted;
	char*			m_buf0;
	char*			m_buf1;
	char*			m_buf2;
	char*			m_buf3;

	enum			{ MIN_BUF_SIZE = 8, MAX_BUF_SIZE = 4096 };
	int				m_buffer_size;

	int				m_cur_pos;
	int				m_select_start;
	int				m_p1;
	int				m_p2;

	float			m_accel;
	float			m_cur_time;
	float			m_rep_time;
	float			m_last_key_time;
	u32				m_last_frame_time;
	u32				m_last_changed_frame;

	bool			m_hold_mode;
	bool			m_insert_mode;
	bool			m_repeat_mode;
	bool			m_mark;
	bool			m_cursor_view;
	bool			m_need_update;
	bool			m_unselected_mode;
}; // class line_edit_control

} // namespace text_editor

