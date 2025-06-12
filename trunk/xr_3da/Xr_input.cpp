#include "stdafx.h"
#include "xr_input.h"
#include "InputKeyEnum.h"
#include "IInputReceiver.h"
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>

CInput *	pInput	= nullptr;

ENGINE_API float	psMouseSens			= 1.f;
ENGINE_API float	psMouseSensScale	= 1.f;
ENGINE_API Flags32	psMouseInvert		= {FALSE};

#define MOUSEBUFFERSIZE			64

static bool g_exclusive	= true;
void on_error_dialog			(bool before)
{
	if (!pInput || !g_exclusive)
		return;

	pInput->exclusive_mode		(!before);
}

CInput::CInput(BOOL bExclusive)
{
	g_exclusive							= !!bExclusive;

	Log("Starting INPUT device...");

	/*
	ZeroMemory							( mouseState,	sizeof(mouseState) );
	ZeroMemory							( timeStamp,	sizeof(timeStamp) );
	ZeroMemory							( timeSave,		sizeof(timeStamp) );
	ZeroMemory							( offs,			sizeof(offs) );
	*/

	Debug.set_on_dialog				(&on_error_dialog);

	InputsScancodes = nullptr;
	for (int i = 0; i < CountInputsScancode; i++)
		InputsScancodesPrev[i] = false;

//	InputsScancodesPrev.resize(CountInputsScancode, false);

#ifdef ENGINE_BUILD
	Device.seqAppActivate.Add		(this);
	Device.seqAppDeactivate.Add		(this);
	Device.seqFrame.Add				(this, REG_PRIORITY_HIGH);
#endif
}

CInput::~CInput(void)
{
#ifdef ENGINE_BUILD
	Device.seqFrame.Remove			(this);
	Device.seqAppDeactivate.Remove	(this);
	Device.seqAppActivate.Remove	(this);
#endif
}

//-----------------------------------------------------------------------
void CInput::KeyUpdate()
{
	if (cbStack.empty())
		return;

	InputsScancodes = SDL_GetKeyboardState(nullptr);

	for (int i = 0; i < CountInputsScancode; i++)
	{
		if (!InputsScancodes[i] && !InputsScancodesPrev[i])
			continue;

		if (!InputsScancodes[i] && InputsScancodesPrev[i])
		{
			cbStack.back()->IR_OnKeyboardRelease(i);
			InputsScancodesPrev[i] = InputsScancodes[i];
			continue;
		}

		if (InputsScancodes[i] && InputsScancodesPrev[i])
		{
			cbStack.back()->IR_OnKeyboardHold(i);
			continue;
		}

		cbStack.back()->IR_OnKeyboardPress(i);
		InputsScancodesPrev[i] = InputsScancodes[i];
	}
}

void CInput::MouseUpdate( )
{
	if (cbStack.empty())
		return;

   	if (CurrentIR() && mouseMove)
	{
		CurrentIR()->IR_OnMouseMove(mouseX, mouseY);
		mouseMove = false;

		mouseX = mouseY = 0.0f;
	}

	mouseState = SDL_GetMouseState(nullptr, nullptr);

	MouseKeyRelease();
	MouseKeyHold();
	MouseKeyPress();

	mouseStatePrev = mouseState;
}

//-------------------------------------------------------
void CInput::iCapture(IInputReceiver *p)
{
	VERIFY(p);

    // change focus
	if (!cbStack.empty())
		cbStack.back()->IR_OnDeactivate();
	cbStack.push_back(p);
	cbStack.back()->IR_OnActivate();
}

void CInput::iRelease(IInputReceiver *p)
{
	if (p == cbStack.back())
	{
		cbStack.back()->IR_OnDeactivate();
		cbStack.pop_back();

		if (cbStack.empty())
			return;

		IInputReceiver * ir = cbStack.back();
		ir->IR_OnActivate();
	}else{// we are not topmost receiver, so remove the nearest one
		u32 cnt = cbStack.size();
		for(;cnt>0;--cnt)
			if( cbStack[cnt-1] == p ){
				xr_vector<IInputReceiver*>::iterator it = cbStack.begin();
				std::advance	(it,cnt-1);
				cbStack.erase	(it);
				break;
			}
	}
}

void CInput::OnAppActivate		(void)
{
	if (CurrentIR())
		CurrentIR()->IR_OnActivate();

	mouseMove = false;
	mouseX = mouseY = 0.0f;

	InputsScancodes = nullptr;
	std::fill(InputsScancodesPrev, InputsScancodesPrev + std::size(InputsScancodesPrev), false);

}

void CInput::OnAppDeactivate	(void)
{
	if (CurrentIR())
		CurrentIR()->IR_OnDeactivate();

	mouseMove = false;
	mouseX = mouseY = 0.0f;

	InputsScancodes = nullptr;
	std::fill(InputsScancodesPrev, InputsScancodesPrev + std::size(InputsScancodesPrev), false);
}

void CInput::OnFrame			(void)
{
	Device.Statistic->Input.Begin			();
	KeyUpdate();
	MouseUpdate();
	Device.Statistic->Input.End				();
}

IInputReceiver*	 CInput::CurrentIR()
{
	if(cbStack.size())
		return cbStack.back();
	else
		return NULL;
}

void CInput::exclusive_mode			(const bool &exclusive)
{
	g_exclusive = exclusive;
}

bool CInput::get_exclusive_mode()
{
	return g_exclusive;
}

bool CInput::GetModState(const SDL_Keymod& mode) const
{
	return SDL_GetModState() & mode;
}

bool CInput::GetPressedKey(int dik) const
{
	return InputsScancodes && InputsScancodes[dik];
}

bool CInput::GetPressedMouseKey(u32 key)
{
	return mouseState & key;
}

const char* CInput::GetKeyName(u16 dik)
{
	switch (dik)
	{
	case MOUSE_LEFT:
		return "LMB";
	case MOUSE_RIGHT:
		return "RMB";
	case MOUSE_MIDDLE:
		return "MMB";
	case MOUSE_X1:
	case MOUSE_X2:
		VERIFY(!"NotImplement! Need name for x1, x2 mouse key");
		break;
	default:
		break;
	}

	SDL_Scancode key_scancode = static_cast<SDL_Scancode>(dik);
	SDL_Keycode key = SDL_GetKeyFromScancode(key_scancode, SDL_KMOD_NONE, false);
	const char* name = SDL_GetKeyName(key);

	return UTF8ToANSI(name);
}

void CInput::TextInputStart(IInputReceiver* receiver)
{
	R_ASSERT2(TextInputReceiver == nullptr, "InputText was started, but not stopped! Need call TextInputStop() for this receiver, and set new receiver!");
	SDL_StartTextInput(Device.SDLWindow);

	TextInputReceiver = receiver;
}

void CInput::TextInputStop()
{
	R_ASSERT2(TextInputReceiver != nullptr, "InputText was stopped or even have not started! Need delete this call TextInputStop() or call TextInputStart() for set receiver");
	SDL_StopTextInput(Device.SDLWindow);

	TextInputReceiver = nullptr;
}

void CInput::TextInputProcess(const char* text)
{
	const char* TextInput = UTF8ToANSI(text);

	TextInputReceiver->IR_OnTextInput(TextInput);
}

void CInput::MouseKeyRelease()
{
	bool NeedReleaseMouse = mouseStatePrev & SDL_BUTTON_LMASK;

	if (!GetPressedMouseKey(SDL_BUTTON_LMASK) && NeedReleaseMouse)
		cbStack.back()->IR_OnMouseRelease(MOUSE_LEFT);

	NeedReleaseMouse = mouseStatePrev & SDL_BUTTON_RMASK;
	if (!GetPressedMouseKey(SDL_BUTTON_RMASK) && NeedReleaseMouse)
		cbStack.back()->IR_OnMouseRelease(MOUSE_RIGHT);

	NeedReleaseMouse = mouseStatePrev & SDL_BUTTON_MMASK;
	if (!GetPressedMouseKey(SDL_BUTTON_MMASK) && NeedReleaseMouse)
		cbStack.back()->IR_OnMouseRelease(MOUSE_MIDDLE);
}

void CInput::MouseKeyHold()
{
	bool NeedHoldMouse = mouseStatePrev & SDL_BUTTON_LMASK;

	if (GetPressedMouseKey(SDL_BUTTON_LMASK) && NeedHoldMouse)
		cbStack.back()->IR_OnMouseHold(MOUSE_LEFT);

	NeedHoldMouse = mouseStatePrev & SDL_BUTTON_RMASK;
	if (GetPressedMouseKey(SDL_BUTTON_RMASK) && NeedHoldMouse)
		cbStack.back()->IR_OnMouseHold(MOUSE_RIGHT);

	NeedHoldMouse = mouseStatePrev & SDL_BUTTON_MMASK;
	if (GetPressedMouseKey(SDL_BUTTON_MMASK) && NeedHoldMouse)
		cbStack.back()->IR_OnMouseHold(MOUSE_MIDDLE);
}

void CInput::MouseKeyPress()
{
	bool NeedPressMouse = !(mouseStatePrev & SDL_BUTTON_LMASK);

	if (GetPressedMouseKey(SDL_BUTTON_LMASK) && NeedPressMouse)
		cbStack.back()->IR_OnMousePress(MOUSE_LEFT);

	NeedPressMouse = !(mouseStatePrev & SDL_BUTTON_RMASK);
	if (GetPressedMouseKey(SDL_BUTTON_RMASK) && NeedPressMouse)
		cbStack.back()->IR_OnMousePress(MOUSE_RIGHT);

	NeedPressMouse = !(mouseStatePrev & SDL_BUTTON_MMASK);
	if (GetPressedMouseKey(SDL_BUTTON_MMASK) && NeedPressMouse)
		cbStack.back()->IR_OnMousePress(MOUSE_MIDDLE);

}

void CInput::SetInputRelativeMouseMode(bool mode)
{
	SDL_SetWindowRelativeMouseMode(Device.SDLWindow, mode);
}