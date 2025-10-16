#pragma once

#include <SDL3/SDL_keycode.h>

constexpr int CountInputsScancode = 322;
constexpr int CountMouseButton = 5;

#define BUTTON_MASK(button) (1u << button);

#include <dinput.h>

class ENGINE_API IInputReceiver;

class ENGINE_API CInput
#ifndef M_BORLAND
	:
	public pureFrame,
	public pureAppActivate,
	public pureAppDeactivate
#endif
{

	xr_vector<IInputReceiver*> cbStack;

	const bool* KeyboardState;
	IInputReceiver* TextInputReceiver = nullptr;

	float mouseX{}, mouseY{};
	float mouseWheelY{};

	bool KeyboardStatePrev[CountInputsScancode];
	bool mouseState[CountMouseButton];
	bool mouseStatePrev[CountMouseButton];

	bool mouseMove{};
	bool mouseWheel{};
public:
	CInput();
	~CInput();

	void OnFrame() override;
    void OnAppActivate() override;
	void OnAppDeactivate() override;

	void iCapture(IInputReceiver* pc);
	void iRelease(IInputReceiver* pc);

	void TextInputStart(IInputReceiver* receiver);
	void TextInputStop();
	void TextInputProcess(const char* text);

	void SetInputRelativeMouseMode(bool mode);
	void SetMouseMotion(float x, float y);
	void SetMouseWheel(float x);

	bool GetPressedKey(int dik) const;
	bool GetPressedMouseKey(int key);
	bool GetModState(const SDL_Keymod& mode) const;
	const char* GetKeyName(u16 dik);

	IInputReceiver* CurrentIR();
private:
	void MouseUpdate();
	void KeyUpdate();
};

extern ENGINE_API CInput *		pInput;

