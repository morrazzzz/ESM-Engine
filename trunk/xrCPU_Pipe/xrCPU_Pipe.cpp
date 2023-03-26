// xrCPU_Pipe.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#pragma hdrstop

#pragma comment(lib,"xr_3DA")

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    return TRUE;
}

extern xrSkin1W			xrSkin1W_x86;
extern xrSkin2W			xrSkin2W_x86;
extern xrSkin3W			xrSkin3W_x86;
extern xrSkin4W			xrSkin4W_x86;

extern xrSkin1W			xrSkin1W_SSE;
extern xrSkin2W			xrSkin2W_SSE;
extern xrSkin3W			xrSkin3W_SSE;
extern xrSkin4W			xrSkin4W_SSE;

extern xrSkin4W			xrSkin4W_thread;

xrSkin4W* skin4W_func = nullptr;

extern xrM44_Mul		xrM44_Mul_x86;
extern xrM44_Mul		xrM44_Mul_3DNow;
extern xrM44_Mul		xrM44_Mul_SSE;
extern xrTransfer		xrTransfer_x86;
extern xrMemFill_32b	xrMemFill32_MMX;


extern "C" {
	__declspec(dllexport) void	__cdecl	xrBind_PSGP	(xrDispatchTable* T, DWORD dwFeatures)
	{
		// analyze features
		// DWORD dwFeatures = CPU::ID.feature & CPU::ID.os_support;

		if(strstr(strlwr(GetCommandLine()),"-x86"))	dwFeatures &= ~(_CPU_FEATURE_SSE+_CPU_FEATURE_3DNOW);

		// generic
		T->skin1W	= xrSkin1W_x86;
		T->skin2W	= xrSkin2W_x86;
		skin4W_func = xrSkin4W_x86;
		// T->blerp	= xrBoneLerp_x86;
		T->m44_mul	= xrM44_Mul_x86;
		T->transfer = xrTransfer_x86;
		T->memFill	= NULL;
		T->memFill32= xrMemFill32_MMX;

		// Init helper threads
		ttapi_Init();

		if (ttapi_GetWorkersCount() > 1)
			T->skin4W = xrSkin4W_thread;
	};
};
