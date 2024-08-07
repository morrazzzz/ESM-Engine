#include "stdafx.h"
#include "EngineAPI.h"

extern xr_token* vid_quality_token;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void __cdecl dummy		(void)	{
};
CEngineAPI::CEngineAPI	()
{
	hGame			= 0;
	hRender			= 0;
	hTuner			= 0;
	pCreate			= 0;
	pDestroy		= 0;
	tune_pause		= dummy	;
	tune_resume		= dummy	;
}

CEngineAPI::~CEngineAPI()
{
	// destroy quality token here
	if (vid_quality_token)
	{
		for (int i = 0; vid_quality_token[i].name; i++)
		{
			xr_free(vid_quality_token[i].name);
		}
		xr_free(vid_quality_token);
		vid_quality_token = NULL;
	}
}
extern u32 renderer_value; //con cmd
ENGINE_API int g_current_renderer = 0;

ENGINE_API bool is_enough_address_space_available()
{
	SYSTEM_INFO		system_info;
	GetSystemInfo(&system_info);

	return (*(size_t*)&system_info.lpMaximumApplicationAddress) > 0x90000000ull;
}


void CEngineAPI::Initialize(void)
{
	//////////////////////////////////////////////////////////////////////////
	// render
	LPCSTR			r1_name	= "xrRender_R1.dll";
	LPCSTR			r2_name	= "xrRender_R2.dll";

#ifndef DEDICATED_SERVER
	if (psDeviceFlags.test(rsR2) )	{
		// try to initialize R2
		Log				("Loading DLL:",	r2_name);
		hRender			= LoadLibrary		(r2_name);
		if (0==hRender)	{
			// try to load R1
			Msg			("...Failed - incompatible hardware.");
		}
	}
#endif

	if (0==hRender)		{
		// try to load R1
		psDeviceFlags.set	(rsR2,FALSE);
		renderer_value		= 0; //con cmd

		Log				("Loading DLL:",	r1_name);
		hRender			= LoadLibrary		(r1_name);
		if (0==hRender)	R_CHK				(GetLastError());
		R_ASSERT		(hRender);
	}

	Device.ConnectToRender();

	// game	
	{
		LPCSTR			g_name	= "xrGame.dll";
		Log				("Loading DLL:",g_name);
		hGame			= LoadLibrary	(g_name);
		if (0==hGame)	R_CHK			(GetLastError());
		R_ASSERT2		(hGame,"Game DLL raised exception during loading or there is no game DLL at all");
		pCreate			= (Factory_Create*)		GetProcAddress(hGame,"xrFactory_Create"		);	R_ASSERT(pCreate);
		pDestroy		= (Factory_Destroy*)	GetProcAddress(hGame,"xrFactory_Destroy"	);	R_ASSERT(pDestroy);
	}

	//////////////////////////////////////////////////////////////////////////
	// vTune
	tune_enabled		= FALSE;
	if (strstr(Core.Params,"-tune"))	{
		LPCSTR			g_name	= "vTuneAPI.dll";
		Log				("Loading DLL:",g_name);
		hTuner			= LoadLibrary	(g_name);
		if (0==hTuner)	R_CHK			(GetLastError());
		R_ASSERT2		(hTuner,"Intel vTune is not installed");
		tune_enabled	= TRUE;
		tune_pause		= (VTPause*)	GetProcAddress(hTuner,"VTPause"		);	R_ASSERT(tune_pause);
		tune_resume		= (VTResume*)	GetProcAddress(hTuner,"VTResume"	);	R_ASSERT(tune_resume);
	}
}

void CEngineAPI::Destroy	(void)
{
	if (hGame)				{ FreeLibrary(hGame);	hGame	= 0; }
	if (hRender)			{ FreeLibrary(hRender); hRender = 0; }
	pCreate					= 0;
	pDestroy				= 0;
	Engine.Event._destroy	();
	XRC.r_clear_compact		();
}

extern "C" {
	typedef bool __cdecl SupportsAdvancedRendering(void);
};

void CEngineAPI::CreateRendererList()
{
	if (vid_quality_token != NULL)
		return;

	bool bSupports_r2 = false;
	bool bSupports_r2_5 = false;

	LPCSTR			r2_name = "xrRender_R2.dll";

	if (strstr(Core.Params, "-perfhud_hack"))
	{
		bSupports_r2 = true;
		bSupports_r2_5 = true;
	}
	else
	{
		// try to initialize R2
		Log("Loading DLL:", r2_name);
		hRender = LoadLibrary(r2_name);
		if (hRender)
		{
			bSupports_r2 = true;
			SupportsAdvancedRendering* test_rendering = (SupportsAdvancedRendering*)GetProcAddress(hRender, "SupportsAdvancedRendering");
			R_ASSERT(test_rendering);
			bSupports_r2_5 = test_rendering();
			FreeLibrary(hRender);
		}
	}

	hRender = 0;

	xr_vector<LPCSTR>			_tmp;
	u32 i = 0;
	bool bBreakLoop = false;
	for (; i < 4; ++i)
	{
		switch (i)
		{
		case 1:
			if (!bSupports_r2)
				bBreakLoop = true;
			break;
		case 3:		//"renderer_r2.5"
			if (!bSupports_r2_5)
				bBreakLoop = true;
			break;
		default:;
		}

		if (bBreakLoop) break;
		_tmp.push_back(NULL);
		LPCSTR val = NULL;
		switch (i)
		{
		case 0: val = "renderer_r1";			break;
		case 1: val = "renderer_r2a";		break;
		case 2: val = "renderer_r2";			break;
		case 3: val = "renderer_r2.5";		break;
		}
		if (bBreakLoop) break;
		_tmp.back() = xr_strdup(val);
		u32 _cnt = _tmp.size() + 1;
		vid_quality_token = xr_alloc<xr_token>(_cnt);

		vid_quality_token[_cnt - 1].id = -1;
		vid_quality_token[_cnt - 1].name = NULL;

//#ifdef DEBUG
		Msg("Available render modes[%d]:", _tmp.size());
//#endif // DEBUG
		for (u32 i = 0; i < _tmp.size(); ++i)
		{
			vid_quality_token[i].id = i;
			vid_quality_token[i].name = _tmp[i];
#ifdef DEBUG
			Msg("[%s]", _tmp[i]);
#endif // DEBUG
		}
}

}