#include "stdafx.h"
#include "UIStatic.h"

using namespace luabind;

#pragma optimize("s",on)
void CUIStatic::script_register(lua_State *L)
{
	module(L)
	[
		class_<CUILines>("CUILines")
		.def("SetFont",				&CUILines::SetFont)
		.def("SetText",				&CUILines::SetText)
		.def("SetTextST",			&CUILines::SetTextST)
		.def("GetText",				&CUILines::GetText)
		.def("SetElipsis",			&CUILines::SetEllipsis)
		.def("SetTextColor",		&CUILines::SetTextColor),


		class_<CUIStatic, CUIWindow>("CUIStatic")
		.def(						constructor<>())
		.def("TextControl",			&CUIStatic::TextItemControl)

		.def("SetText",				(void (CUIStatic::*)(LPCSTR)) (&CUIStatic::SetText) )
		.def("SetTextST",			(void (CUIStatic::*)(LPCSTR)) (&CUIStatic::SetTextST) )
		.def("GetText",				&CUIStatic::GetText)
		
		.def("SetColor",			&CUIStatic::SetColor)
		.def("GetColor",			&CUIStatic::GetColor)
		.def("SetTextColor",		&CUIStatic::SetTextColor_script)
		.def("Init",				(void(CUIStatic::*)(float,float,float,float))&CUIStatic::Init )
		.def("Init",				(void(CUIStatic::*)(LPCSTR,float,float,float,float))&CUIStatic::Init )
		.def("InitTexture",			&CUIStatic::InitTexture )
		.def("SetTextureOffset",	&CUIStatic::SetTextureOffset )

		.def("SetStretchTexture",	&CUIStatic::SetStretchTexture)
		.def("GetStretchTexture",	&CUIStatic::GetStretchTexture)

		.def("SetTextAlign",		&CUIStatic::SetTextAlign_script)
		.def("GetTextAlign",		&CUIStatic::GetTextAlign_script)

		.def("SetHeading",			&CUIStatic::SetHeading)
		.def("GetHeading",			&CUIStatic::GetHeading),

		class_<CUITextWnd, CUIWindow>("CUITextWnd")
		.def(						constructor<>())
		.def("AdjustHeightToText",	&CUITextWnd::AdjustHeightToText)
		.def("AdjustWidthToText",	&CUITextWnd::AdjustWidthToText)
		.def("SetText",				&CUITextWnd::SetText)
		.def("SetTextST",			&CUITextWnd::SetTextST)
		.def("GetText",				&CUITextWnd::GetText)
		.def("SetFont",				&CUITextWnd::SetFont)
		.def("GetFont",				&CUITextWnd::GetFont)
		.def("SetTextColor",		&CUITextWnd::SetTextColor)
		.def("GetTextColor",		&CUITextWnd::GetTextColor)
		.def("SetTextComplexMode",	&CUITextWnd::SetTextComplexMode)
		.def("SetTextAlignment",	&CUITextWnd::SetTextAlignment)
		.def("SetVTextAlignment",	&CUITextWnd::SetVTextAlignment)
		.def("SetEllipsis",			&CUITextWnd::SetEllipsis)
		.def("SetTextOffset",		&CUITextWnd::SetTextOffset)
//		.def("",					&CUITextWnd::)
	];
}