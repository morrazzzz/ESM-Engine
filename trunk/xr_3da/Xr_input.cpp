#include "stdafx.h"
#include "xr_input.h"
#include "IInputReceiver.h"
#include <SDL3/SDL_keyboard.h>

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

	pDI 								=	NULL;
	pMouse								=	NULL;
	pKeyboard							=	NULL;

	//=====================Mouse
	mouse_property.mouse_dt				=	25;

	ZeroMemory							( mouseState,	sizeof(mouseState) );
	ZeroMemory							( timeStamp,	sizeof(timeStamp) );
	ZeroMemory							( timeSave,		sizeof(timeStamp) );
	ZeroMemory							( offs,			sizeof(offs) );

	if (!pDI) CHK_DX(DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&pDI, NULL ));

	// MOUSE
	CHK_DX(CreateInputDevice(
	&pMouse,		GUID_SysMouse,		&c_dfDIMouse2,
	((bExclusive)?DISCL_EXCLUSIVE:DISCL_NONEXCLUSIVE) | DISCL_FOREGROUND | DISCL_NOWINKEY,
	MOUSEBUFFERSIZE ));

	Debug.set_on_dialog				(&on_error_dialog);

	InputsScancodes = nullptr;
	for (int i = 0; i < CountInputsScancode; i++)
		InputsScancodesPrev[i] = false;

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
	//_______________________

	// Unacquire and release the device's interfaces
	if( pMouse ){
		pMouse->Unacquire();
		_RELEASE	(pMouse);
	}

	_SHOW_REF	("Input: ",pDI);
	_RELEASE	(pDI);
}

//-----------------------------------------------------------------------------
// Name: CreateInputDevice()
// Desc: Create a DirectInput device.
//-----------------------------------------------------------------------------
HRESULT CInput::CreateInputDevice( LPDIRECTINPUTDEVICE8* device, GUID guidDevice, const DIDATAFORMAT* pdidDataFormat, u32 dwFlags, u32 buf_size )
{
	// Obtain an interface to the input device
//.	CHK_DX( pDI->CreateDeviceEx( guidDevice, IID_IDirectInputDevice8, (void**)device, NULL ) );
	CHK_DX( pDI->CreateDevice( guidDevice, /*IID_IDirectInputDevice8,*/ device, NULL ) );

	// Set the device data format. Note: a data format specifies which
	// controls on a device we are interested in, and how they should be
	// reported.
	CHK_DX((*device)->SetDataFormat( pdidDataFormat ) );

	// Set the cooperativity level to let DirectInput know how this device
	// should interact with the system and with other DirectInput applications.
	HRESULT _hr = (*device)->SetCooperativeLevel( Device.m_hWnd, dwFlags );
	if (FAILED(_hr) && (_hr==E_NOTIMPL)) Msg("! INPUT: Can't set coop level. Emulation???");
	else R_CHK(_hr);

	// setup the buffer size for the keyboard data
	DIPROPDWORD				dipdw;
	dipdw.diph.dwSize		= sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj		= 0;
	dipdw.diph.dwHow		= DIPH_DEVICE;
	dipdw.dwData			= buf_size;

	CHK_DX( (*device)->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) );

	return S_OK;
}

//-----------------------------------------------------------------------

void CInput::SetAllAcquire( BOOL bAcquire )
{
	if (pMouse)		bAcquire ? pMouse->Acquire() 	: pMouse->Unacquire();
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
			continue;
		}

		if (InputsScancodes[i] && InputsScancodesPrev[i])
		{
			cbStack.back()->IR_OnKeyboardHold(i);
			continue;
		}

		cbStack.back()->IR_OnKeyboardPress(i);
	}

	pInput->InputBeforeNewFrame();
}

BOOL CInput::iGetAsyncBtnState( int btn )
{
	return !!mouseState[btn];
}

void CInput::MouseUpdate( )
{
	if (cbStack.empty())
		return;

	HRESULT hr;
	DWORD dwElements	= MOUSEBUFFERSIZE;
	DIDEVICEOBJECTDATA	od[MOUSEBUFFERSIZE];

	VERIFY(pMouse);

	hr = pMouse->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), &od[0], &dwElements, 0 );
	if (( hr == DIERR_INPUTLOST )||( hr == DIERR_NOTACQUIRED )){
		hr = pMouse->Acquire();
		if ( hr != S_OK ) return;
		hr = pMouse->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), &od[0], &dwElements, 0 );
		if ( hr != S_OK ) return;
	};
	BOOL				mouse_prev[COUNT_MOUSE_BUTTONS];

	mouse_prev[0]		= mouseState[0];
	mouse_prev[1]		= mouseState[1];
	mouse_prev[2]		= mouseState[2];

	offs[0] = offs[1] = offs[2] = 0;
	for (u32 i = 0; i < dwElements; i++){
		switch (od[i].dwOfs){
		case offsetof(DIMOUSESTATE, lX):
			offs[0]	+= od[i].dwData;
			timeStamp[0] = od[i].dwTimeStamp;
			break;
		case offsetof(DIMOUSESTATE, lY):	
			offs[1]	+= od[i].dwData;
			timeStamp[1] = od[i].dwTimeStamp;
			break;
		case offsetof(DIMOUSESTATE, lZ):	
			offs[2]	+= od[i].dwData;
			timeStamp[2] = od[i].dwTimeStamp;
			break;
		case DIMOFS_BUTTON0:
			if ( od[i].dwData & 0x80 )	
			{ mouseState[0] = TRUE;				cbStack.back()->IR_OnMousePress(0);		}
			if ( !(od[i].dwData & 0x80))
			{ mouseState[0] = FALSE;			cbStack.back()->IR_OnMouseRelease(0);	}
			break;
		case DIMOFS_BUTTON1:
			if ( od[i].dwData & 0x80 )	
			{ mouseState[1] = TRUE;				cbStack.back()->IR_OnMousePress(1);		}
			if ( !(od[i].dwData & 0x80))
			{ mouseState[1] = FALSE;			cbStack.back()->IR_OnMouseRelease(1);	}
			break;
		case DIMOFS_BUTTON2:
			if ( od[i].dwData & 0x80 )	
			{ mouseState[2] = TRUE;				cbStack.back()->IR_OnMousePress(2);		}
			if ( !(od[i].dwData & 0x80))
			{ mouseState[2] = FALSE;			cbStack.back()->IR_OnMouseRelease(2);	}
			break;
		case DIMOFS_BUTTON3:
			if ( od[i].dwData & 0x80 )	
			{ mouseState[2] = TRUE;				cbStack.back()->IR_OnKeyboardPress(0xED + 103);		}
			if ( !(od[i].dwData & 0x80))
			{ mouseState[2] = FALSE;			cbStack.back()->IR_OnKeyboardRelease(0xED + 103);	}
			break;
		case DIMOFS_BUTTON4:
			if ( od[i].dwData & 0x80 )	
			{ mouseState[2] = TRUE;				cbStack.back()->IR_OnKeyboardPress(0xED + 104);		}
			if ( !(od[i].dwData & 0x80))
			{ mouseState[2] = FALSE;			cbStack.back()->IR_OnKeyboardRelease(0xED + 104);	}
			break;
		case DIMOFS_BUTTON5:
			if ( od[i].dwData & 0x80 )	
			{ mouseState[2] = TRUE;				cbStack.back()->IR_OnKeyboardPress(0xED + 105);		}
			if ( !(od[i].dwData & 0x80))
			{ mouseState[2] = FALSE;			cbStack.back()->IR_OnKeyboardRelease(0xED + 105);	}
			break;
		case DIMOFS_BUTTON6:
			if ( od[i].dwData & 0x80 )	
			{ mouseState[2] = TRUE;				cbStack.back()->IR_OnKeyboardPress(0xED + 106);		}
			if ( !(od[i].dwData & 0x80))
			{ mouseState[2] = FALSE;			cbStack.back()->IR_OnKeyboardRelease(0xED + 106);	}
			break;
		case DIMOFS_BUTTON7:
			if ( od[i].dwData & 0x80 )	
			{ mouseState[2] = TRUE;				cbStack.back()->IR_OnKeyboardPress(0xED + 107);		}
			if ( !(od[i].dwData & 0x80))
			{ mouseState[2] = FALSE;			cbStack.back()->IR_OnKeyboardRelease(0xED + 107);	}
			break;
		}
	}

	if (mouseState[0] && mouse_prev[0])
	{
		cbStack.back()->IR_OnMouseHold(0);
	}

	if (mouseState[1] && mouse_prev[1])		
	{
		cbStack.back()->IR_OnMouseHold(1);
	}

	if (mouseState[2] && mouse_prev[2])		
	{
		cbStack.back()->IR_OnMouseHold(2);
	}
	if ( dwElements ){
		if (offs[0] || offs[1]) cbStack.back()->IR_OnMouseMove	( offs[0], offs[1] );
		if (offs[2])			cbStack.back()->IR_OnMouseWheel	( offs[2] );
	} else {
		if (timeStamp[1] && ((dwCurTime-timeStamp[1])>=mouse_property.mouse_dt))	cbStack.back()->IR_OnMouseStop(DIMOFS_Y, timeStamp[1] = 0);
		if (timeStamp[0] && ((dwCurTime-timeStamp[0])>=mouse_property.mouse_dt))	cbStack.back()->IR_OnMouseStop(DIMOFS_X, timeStamp[0] = 0);
	}
}

//-------------------------------------------------------
void CInput::iCapture(IInputReceiver *p)
{
	VERIFY(p);
	if (pMouse) 	MouseUpdate();
    if (pKeyboard) 	KeyUpdate();

    // change focus
	if (!cbStack.empty())
		cbStack.back()->IR_OnDeactivate();
	cbStack.push_back(p);
	cbStack.back()->IR_OnActivate();

	// prepare for _new_ controller
	ZeroMemory			( timeStamp,	sizeof(timeStamp) );
	ZeroMemory			( timeSave,		sizeof(timeStamp) );
	ZeroMemory			( offs,			sizeof(offs) );
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

	SetAllAcquire	( true );
	ZeroMemory		( mouseState,	sizeof(mouseState) );
	ZeroMemory		( timeStamp,	sizeof(timeStamp) );
	ZeroMemory		( timeSave,		sizeof(timeStamp) );
	ZeroMemory		( offs,			sizeof(offs) );
}

void CInput::OnAppDeactivate	(void)
{
	if (CurrentIR())
		CurrentIR()->IR_OnDeactivate();

	SetAllAcquire	( false );
	ZeroMemory		( mouseState,	sizeof(mouseState) );
	ZeroMemory		( timeStamp,	sizeof(timeStamp) );
	ZeroMemory		( timeSave,		sizeof(timeStamp) );
	ZeroMemory		( offs,			sizeof(offs) );
}

void CInput::OnFrame			(void)
{
	Device.Statistic->Input.Begin			();
	dwCurTime		= Device.TimerAsync_MMT	();
	KeyUpdate();
	if (pMouse)		MouseUpdate				();
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
	pKeyboard->SetCooperativeLevel	(
		Device.m_hWnd, 
		(exclusive ? DISCL_EXCLUSIVE : DISCL_NONEXCLUSIVE) | DISCL_FOREGROUND
	);

	pMouse->SetCooperativeLevel		(
		Device.m_hWnd, 
		(exclusive ? DISCL_EXCLUSIVE : DISCL_NONEXCLUSIVE) | DISCL_FOREGROUND | DISCL_NOWINKEY
	);
}

bool CInput::get_exclusive_mode()
{
	return g_exclusive;
}

void CInput::InputBeforeNewFrame()
{
	for (int i = 0; i < CountInputsScancode; ++i)
		InputsScancodesPrev[i] = InputsScancodes[i];
}

bool CInput::GetModState(const SDL_Keymod& mode) const
{
	return SDL_GetModState() & mode;
}

bool CInput::GetPressedKey(int dik) const
{
	return InputsScancodes[dik];
}

const char* CInput::GetKeyName(SDL_Scancode dik)
{
	SDL_Keycode key = SDL_GetKeyFromScancode(dik, SDL_KMOD_NONE, false);
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