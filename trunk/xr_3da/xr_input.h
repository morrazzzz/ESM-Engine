#pragma once

#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>

constexpr int CountInputsScancode = 322;
#include <dinput.h>

class	ENGINE_API				IInputReceiver;

class ENGINE_API CInput
#ifndef M_BORLAND
	:
	public pureFrame,
	public pureAppActivate,
	public pureAppDeactivate
#endif
{
private:
	//----------------------
//	xr_stack<IInputReceiver*>	cbStack;
	xr_vector<IInputReceiver*>	cbStack;

	void						MouseUpdate					( );
	void						KeyUpdate					( );

public:
	void						iCapture					( IInputReceiver *pc );
	void						iRelease					( IInputReceiver *pc );

	CInput						( BOOL bExclusive = true);
	~CInput						( );

	virtual void				OnFrame						(void);
	virtual void				OnAppActivate				(void);
	virtual void				OnAppDeactivate				(void);

	IInputReceiver*				CurrentIR					();

public:
			void				exclusive_mode				(const bool &exclusive);
			bool				get_exclusive_mode();

private:
	void MouseKeyHold();
	void MouseKeyPress();
private:
	IInputReceiver* TextInputReceiver = nullptr;

	bool InputsScancodesPrev[CountInputsScancode];
	u32 mouseStatePrev;
public:
	bool mouseMove = false;
    float mouseX{}, mouseY{};

	SDL_MouseButtonFlags mouseState;
	const bool* InputsScancodes;
public:
	void MouseKeyRelease();

	void TextInputStart(IInputReceiver* receiver);
	void TextInputStop();
	void TextInputProcess(const char* text);

	void SetInputRelativeMouseMode(bool mode);

	bool GetPressedKey(int dik) const;
	bool GetPressedMouseKey(u32 key);
	bool GetModState(const SDL_Keymod& mode) const;
	const char* GetKeyName(u16 dik);
};

extern ENGINE_API CInput *		pInput;

