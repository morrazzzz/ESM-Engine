#pragma once

#pragma todo("Refactor this!!!!")

#if defined(USE_DX11) || defined(USE_DX10)

#	include "..\xrRenderDX10\DXCommonTypes.h"

#else	//	USE_DX10

typedef	IDirect3DVertexShader9	ID3DVertexShader;
typedef	IDirect3DPixelShader9	ID3DPixelShader;
typedef	IDirect3DQuery9			ID3DQuery;
typedef	IDirect3DTexture9		ID3DTexture2D;
typedef	IDirect3DSurface9		ID3DRenderTargetView;
typedef	IDirect3DSurface9		ID3DDepthStencilView;
typedef	IDirect3DBaseTexture9	ID3DBaseTexture;
typedef	D3DSURFACE_DESC			D3D_TEXTURE2D_DESC;
typedef IDirect3DVertexBuffer9	ID3DVertexBuffer;
typedef IDirect3DIndexBuffer9	ID3DIndexBuffer;
typedef	IDirect3DStateBlock9	ID3DState;

#endif	//	USE_DX10
