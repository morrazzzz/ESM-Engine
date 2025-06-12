#pragma once

class ENGINE_API	IInputReceiver
{
public:
	BOOL			IR_GetKeyState					(int dik);
	void			IR_Capture						(void);
	void			IR_Release						(void);

	virtual void	IR_OnDeactivate					(void);
	virtual void	IR_OnActivate					(void);

	virtual void IR_OnMousePress(int btn) {}
	virtual void IR_OnMouseRelease(int btn)	{}
	virtual void IR_OnMouseHold(int btn) {}
	virtual void IR_OnMouseWheel(int direction) {}
	virtual void IR_OnMouseMove(float x, float y) {}

	virtual void IR_OnKeyboardPress(int dik) {}
	virtual void IR_OnKeyboardRelease(int dik) {}
	virtual void IR_OnKeyboardHold(int dik) {}

	virtual void IR_OnTextInput(const char* text) {}
};

ENGINE_API extern float			psMouseSens;
ENGINE_API extern float			psMouseSensScale;
ENGINE_API extern Flags32		psMouseInvert;

