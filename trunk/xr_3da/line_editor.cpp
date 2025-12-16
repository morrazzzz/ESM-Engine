////////////////////////////////////////////////////////////////////////////
//	Module 		: line_editor.cpp
//	Created 	: 22.02.2008
//	Author		: Evgeniy Sokolov
//	Description : line editor class implementation
////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "line_editor.h"

namespace text_editor
{

line_editor::line_editor( u32 str_buffer_size )
	: m_control( str_buffer_size )
{
}

line_editor::~line_editor()
{
}

void line_editor::on_frame()
{
	m_control.on_frame();
}

void line_editor::lineEditorKeyPress( int dik )
{
	m_control.on_key_press( dik );
}

void line_editor::lineEditorKeyHold( int dik )
{
	m_control.on_key_hold( dik );
}

void line_editor::lineEditorKeyRelease( int dik )
{
	m_control.on_key_release( dik );
}

void line_editor::lineEditorTextInput(const char* text)
{
	m_control.InputConsoleText(text);
}

} // namespace text_editor
