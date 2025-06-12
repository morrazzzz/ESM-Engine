#include "stdafx.h"
#include "xr_input.h"
#include "iinputreceiver.h"

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
	if (pInput->InputsScancodes)
	{
		for (i = 0; i < CountInputsScancode; i++)
		{
			if (IR_GetKeyState(i))
				IR_OnKeyboardRelease(i);
		}
	}

	pInput->MouseKeyRelease();
}

void IInputReceiver::IR_OnActivate(void)
{
}

BOOL IInputReceiver::IR_GetKeyState(int dik)
{
	VERIFY(pInput);
	return pInput->GetPressedKey(dik);
}
