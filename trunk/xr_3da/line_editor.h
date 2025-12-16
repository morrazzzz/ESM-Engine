#pragma once

#include "IInputReceiver.h"
#include "line_edit_control.h"

namespace text_editor
{
	class line_editor
	{
	public:
		line_editor(u32 str_buffer_size);
		virtual ~line_editor();

		IC line_edit_control& control() { return m_control; }

		void on_frame();

		void lineEditorKeyPress(int dik);
		void lineEditorKeyHold(int dik);
		void lineEditorKeyRelease(int dik);
		void lineEditorTextInput(const char* text);

	private:
		line_edit_control		m_control;

	}; // class line_editor

} // namespace text_editor
