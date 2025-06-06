#pragma once

#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_keycode.h>

constexpr int CountInputsScancode = 322;

#define DIRECTINPUT_VERSION 0x0800
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
public:
	enum {
		COUNT_MOUSE_BUTTONS			= 3,
		COUNT_MOUSE_AXIS			= 3,
	};
	struct sxr_mouse
	{
		DIDEVCAPS					capabilities;
		DIDEVICEINSTANCE			deviceInfo;
		DIDEVICEOBJECTINSTANCE		objectInfo;
		u32							mouse_dt;
	};
private:
	LPDIRECTINPUT8				pDI;			// The DInput object
	LPDIRECTINPUTDEVICE8		pMouse;			// The DIDevice7 interface
	LPDIRECTINPUTDEVICE8		pKeyboard;		// The DIDevice7 interface
	//----------------------
	u32							timeStamp	[COUNT_MOUSE_AXIS];
	u32							timeSave	[COUNT_MOUSE_AXIS];
	int 						offs		[COUNT_MOUSE_AXIS];
	BOOL						mouseState	[COUNT_MOUSE_BUTTONS];

	HRESULT						CreateInputDevice(	LPDIRECTINPUTDEVICE8* device, GUID guidDevice,
													const DIDATAFORMAT* pdidDataFormat, u32 dwFlags,
													u32 buf_size );

//	xr_stack<IInputReceiver*>	cbStack;
	xr_vector<IInputReceiver*>	cbStack;

	void						MouseUpdate					( );
	void						KeyUpdate					( );

public:
	sxr_mouse					mouse_property;
	u32							dwCurTime;

	void						SetAllAcquire				( BOOL bAcquire = TRUE );

	void						iCapture					( IInputReceiver *pc );
	void						iRelease					( IInputReceiver *pc );
	BOOL						iGetAsyncBtnState			( int btn );
	void						iGetLastMouseDelta			( Ivector2& p )	{ p.set(offs[0],offs[1]); }

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
	IInputReceiver* TextInputReceiver = nullptr;

	bool InputsScancodesPrev[CountInputsScancode];
public:
//morrazzzz: start
	const bool* InputsScancodes;
public:
	void InputBeforeNewFrame();
	void TextInputStart(IInputReceiver* receiver);
	void TextInputStop();
	void TextInputProcess(const char* text);
	bool GetPressedKey(int dik) const;
	bool GetModState(const SDL_Keymod& mode) const;
	const char* GetKeyName(SDL_Scancode dik);
};

extern ENGINE_API CInput *		pInput;

