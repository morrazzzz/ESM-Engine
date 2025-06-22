#include "stdafx.h"
#include "xr_input.h"
#include "iinputreceiver.h"
#include "InputKeyEnum.h"

void	IInputReceiver::IR_Capture						(void)
{
	VERIFY(pInput);
	pInput->iCapture(this);
}

void	IInputReceiver::IR_Release						(void)
{
	VERIFY(pInput);
	pInput->iRelease(this);
}

void IInputReceiver::IR_OnDeactivate()
{
	int i;

	for (i = 0; i < CountInputsScancode; i++)
	{
		if (IR_GetKeyState(i))
			IR_OnKeyboardRelease(i);
	}

	for (int i = 0; i < CountMouseButton; i++)
	{
		if (pInput->GetPressedMouseKey(i))
			IR_OnMouseRelease(i + MOUSE_LEFT_BUTTON_IDX);
	}
}

void IInputReceiver::IR_OnActivate(void)
{
}

BOOL IInputReceiver::IR_GetKeyState(int dik)
{
	VERIFY(pInput);
	return pInput->GetPressedKey(dik);
}
