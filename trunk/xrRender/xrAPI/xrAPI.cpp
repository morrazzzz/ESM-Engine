// xrAPI.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "../../Include/xrApi/xrAPI.h"

//XRAPI_API IRender_interface*	Render = NULL;
XRAPI_API IRenderFactory*	RenderFactory = NULL;
XRAPI_API CDUInterface*	DU = NULL;
//XRAPI_API xr_token*	vid_mode_token = NULL;
XRAPI_API IUIRender* UIRender = NULL;
//XRAPI_API CSound_manager_interface*	Sound = NULL;
XRAPI_API CGameMtlLibrary*	PGMLib = NULL;

#ifdef DEBUG
	XRAPI_API IDebugRender*	DRender = NULL;
#endif // DEBUG
