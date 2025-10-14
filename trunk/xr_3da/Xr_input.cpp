#include "stdafx.h"
#include "xr_input.h"
#include "InputKeyEnum.h"
#include "IInputReceiver.h"
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>

CInput* pInput = nullptr;

ENGINE_API float psMouseSens = 1.f;
ENGINE_API float psMouseSensScale = 1.f;
ENGINE_API BOOL	psMouseInvert = 0;

CInput::CInput()
{
	Log("Starting INPUT device...");

	KeyboardState = nullptr;
	std::fill(KeyboardStatePrev, KeyboardStatePrev + std::size(KeyboardStatePrev), false);
	std::fill(mouseState, mouseState + std::size(mouseState), false);
	std::fill(mouseStatePrev, mouseStatePrev + std::size(mouseStatePrev), false);

#ifdef ENGINE_BUILD
	Device.seqAppActivate.Add		(this);
	Device.seqAppDeactivate.Add		(this);
	Device.seqFrame.Add				(this, REG_PRIORITY_HIGH);
#endif
}

CInput::~CInput()
{
#ifdef ENGINE_BUILD
	Device.seqFrame.Remove			(this);
	Device.seqAppDeactivate.Remove	(this);
	Device.seqAppActivate.Remove	(this);
#endif
}

void CInput::OnAppActivate()
{
	if (CurrentIR())
		CurrentIR()->IR_OnActivate();

	mouseMove = false;
	mouseX = mouseY = 0.0f;

	KeyboardState = nullptr;
	std::fill(KeyboardStatePrev, KeyboardStatePrev + std::size(KeyboardStatePrev), false);
	std::fill(mouseState, mouseState + std::size(mouseState), false);
	std::fill(mouseStatePrev, mouseStatePrev + std::size(mouseStatePrev), false);
}

void CInput::OnAppDeactivate()
{
	if (CurrentIR())
		CurrentIR()->IR_OnDeactivate();

	mouseMove = false;
	mouseX = mouseY = 0.0f;

	KeyboardState = nullptr;
	std::fill(KeyboardStatePrev, KeyboardStatePrev + std::size(KeyboardStatePrev), false);
	std::fill(mouseState, mouseState + std::size(mouseState), false);
	std::fill(mouseStatePrev, mouseStatePrev + std::size(mouseStatePrev), false);
}

void CInput::OnFrame()
{
	if (Device.getImGuiActivated())
		return;

	Device.Statistic->Input.Begin();
	KeyUpdate();
	MouseUpdate();
	Device.Statistic->Input.End();
}

//-------------------------------------------------------
void CInput::iCapture(IInputReceiver* p)
{
	R_ASSERT(p);

	// change focus
	if (!cbStack.empty())
		cbStack.back()->IR_OnDeactivate();
	cbStack.push_back(p);
	cbStack.back()->IR_OnActivate();
}

void CInput::iRelease(IInputReceiver* p)
{
	if (p == cbStack.back())
	{
		cbStack.back()->IR_OnDeactivate();
		cbStack.pop_back();

		if (cbStack.empty())
			return;

		IInputReceiver* ir = cbStack.back();
		ir->IR_OnActivate();
	}
	else {// we are not topmost receiver, so remove the nearest one
		u32 cnt = cbStack.size();
		for (; cnt > 0; --cnt)
			if (cbStack[cnt - 1] == p) {
				xr_vector<IInputReceiver*>::iterator it = cbStack.begin();
				std::advance(it, cnt - 1);
				cbStack.erase(it);
				break;
			}
	}
}

IInputReceiver* CInput::CurrentIR()
{
	if (cbStack.size())
		return cbStack.back();
	else
		return nullptr;
}

//-----------------------------------------------------------------------
void CInput::KeyUpdate()
{
	if (!CurrentIR())
		return;

	KeyboardState = SDL_GetKeyboardState(nullptr);

	for (int i = 0; i < CountInputsScancode; i++)
	{
		if (!CurrentIR())
			break;

		if (!KeyboardState[i] && !KeyboardStatePrev[i])
			continue;

		if (!KeyboardState[i] && KeyboardStatePrev[i])
		{
			CurrentIR()->IR_OnKeyboardRelease(i);
			KeyboardStatePrev[i] = KeyboardState[i];
			continue;
		}

		if (KeyboardState[i] && !KeyboardStatePrev[i])
		{
			CurrentIR()->IR_OnKeyboardPress(i);
			KeyboardStatePrev[i] = KeyboardState[i];
			continue;
		}

		CurrentIR()->IR_OnKeyboardHold(i);
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

	SDL_MouseButtonFlags mouseButtonFlags = SDL_GetMouseState(nullptr, nullptr);

	for (int i = 0; i < 5; i++)
	{
		if (!CurrentIR())
			break;

		mouseState[i] = mouseButtonFlags & BUTTON_MASK(i);

		if (!mouseState[i] && !mouseStatePrev[i])
			continue;

		if (!mouseState[i] && mouseStatePrev[i])
		{
			CurrentIR()->IR_OnMouseRelease(i + MOUSE_LEFT_BUTTON_IDX);
			mouseStatePrev[i] = mouseState[i];
			continue;
		}

		if (mouseState[i] && !mouseStatePrev[i])
		{
			CurrentIR()->IR_OnMousePress(i + MOUSE_LEFT_BUTTON_IDX);
			mouseStatePrev[i] = mouseState[i];
			continue;
		}

		CurrentIR()->IR_OnMouseHold(i + MOUSE_LEFT_BUTTON_IDX);
	}
}

bool CInput::GetModState(const SDL_Keymod& mode) const
{
	return SDL_GetModState() & mode;
}

bool CInput::GetPressedKey(int dik) const
{
	return KeyboardState && KeyboardState[dik];
}

bool CInput::GetPressedMouseKey(int key)
{
	return mouseState[key];
}

const char* CInput::GetKeyName(u16 dik)
{
	switch (dik)
	{
	case MOUSE_LEFT_BUTTON_IDX:
		return "LMB";
	case MOUSE_RIGHT_BUTTON_IDX:
		return "RMB";
	case MOUSE_MIDDLE_BUTTON_IDX:
		return "MMB";
	case MOUSE_BACK_BUTTON_IDX:
		return "Back MB";
	case MOUSE_FORWARD_BUTTON_IDX:
		return "Forward MB";
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
	Device.StartWindowTextInput();

	TextInputReceiver = receiver;
}

void CInput::TextInputStop()
{
	R_ASSERT2(TextInputReceiver != nullptr, "InputText was stopped or even have not started! Need delete this call TextInputStop() or call TextInputStart() for set receiver");
	Device.StopWindowTextInput();

	TextInputReceiver = nullptr;
}

void CInput::TextInputProcess(const char* text)
{
	const char* TextInput = UTF8ToANSI(text);

	TextInputReceiver->IR_OnTextInput(TextInput);
}

void CInput::SetInputRelativeMouseMode(bool mode)
{
	SDL_SetWindowRelativeMouseMode(Device.SDLWindow, mode);
}

void CInput::SetMouseMotion(float x, float y)
{
	mouseX += x;
	mouseY += y;
	mouseMove = true;
}