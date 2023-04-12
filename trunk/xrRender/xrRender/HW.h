// HW.h: interface for the CHW class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "hwcaps.h"
#include "stats_manager.h"

class CHW
{
	HINSTANCE 				hD3D9;
public:
	IDirect3D9* 			pD3D;		// D3D
	IDirect3DDevice9*		pDevice;	// render device

	IDirect3DSurface9*		pBaseRT;
	IDirect3DSurface9*		pBaseZB;

	CHWCaps					Caps;

	UINT					DevAdapter;
	D3DDEVTYPE				DevT;
	D3DPRESENT_PARAMETERS	DevPP;

	stats_manager			stats_manager;

	CHW()
	{
    	hD3D9		= nullptr;
		pD3D		= nullptr;
		pDevice		= nullptr;
		pBaseRT		= nullptr;
		pBaseZB		= nullptr;
	};

	void					CreateD3D				();
	void					DestroyD3D				();
	void					CreateDevice(HWND hw, bool move_window);
	void					DestroyDevice			();

	void					Reset					(HWND hw);

	void					selectResolution		(u32 &dwWidth, u32 &dwHeight, BOOL bWindowed);
	D3DFORMAT				selectDepthStencil		(D3DFORMAT);
	u32						selectPresentInterval	();
	u32						selectGPU				();
	u32						selectRefresh			(u32 dwWidth, u32 dwHeight, D3DFORMAT fmt);
	void					updateWindowProps		(HWND hw);
	BOOL					support					(D3DFORMAT fmt, DWORD type, DWORD usage);

#ifdef DEBUG
	void	Validate(void)	{	VERIFY(pDevice); VERIFY(pD3D); };
#else
	void	Validate(void)	{};
#endif
private:
	bool m_move_window;
};

extern ECORE_API CHW HW;
