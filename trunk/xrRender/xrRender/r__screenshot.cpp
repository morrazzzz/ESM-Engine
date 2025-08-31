#include "stdafx.h"
#include "../xrRender/tga.h"
#include "../../xr_3da/xrImage_Resampler.h"

#define	GAMESAVE_SIZE	128

#define SM_FOR_SEND_WIDTH 640
#define SM_FOR_SEND_HEIGHT 480

#if defined(USE_DX10) || defined(USE_DX11)
void CRender::ScreenshotImpl	(ScreenshotMode mode, LPCSTR name, CMemoryWriter* memory_writer)
{
	ID3DResource		*pSrcTexture;
	HW.pBaseRT->GetResource(&pSrcTexture);

	VERIFY(pSrcTexture);

	// Save
	switch (mode)	
	{
		case IRender_interface::SM_FOR_GAMESAVE:
			{
				ID3DTexture2D		*pSrcSmallTexture;
	
				D3D_TEXTURE2D_DESC desc;
				ZeroMemory( &desc, sizeof(desc) );
				desc.Width = GAMESAVE_SIZE;
				desc.Height = GAMESAVE_SIZE;
				desc.MipLevels = 1;
				desc.ArraySize = 1;
				desc.Format = DXGI_FORMAT_BC1_UNORM;
				desc.SampleDesc.Count = 1;
				desc.Usage = D3D_USAGE_DEFAULT;
				desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
				CHK_DX( HW.pDevice->CreateTexture2D( &desc, NULL, &pSrcSmallTexture ) );

				//	D3DX10_TEXTURE_LOAD_INFO *pLoadInfo

#ifdef USE_DX11
				CHK_DX(D3DX11LoadTextureFromTexture(HW.pContext, pSrcTexture,
					NULL, pSrcSmallTexture ));
#else
				CHK_DX(D3DX10LoadTextureFromTexture( pSrcTexture,
					NULL, pSrcSmallTexture ));
#endif

				// save (logical & physical)
				ID3DBlob*		saved	= 0;
#ifdef USE_DX11
				HRESULT hr = D3DX11SaveTextureToMemory(HW.pContext, pSrcSmallTexture, D3DX11_IFF_DDS, &saved, 0);
#else
				HRESULT hr					= D3DX10SaveTextureToMemory( pSrcSmallTexture, D3DX10_IFF_DDS, &saved, 0);
				//HRESULT hr					= D3DXSaveTextureToFileInMemory (&saved,D3DXIFF_DDS,texture,0);
#endif
				if(hr==D3D_OK)
				{
					IWriter*			fs		= FS.w_open	(name); 
					if (fs)				
					{
						fs->w				(saved->GetBufferPointer(),(u32)saved->GetBufferSize());
						FS.w_close			(fs);
					}
				}
				_RELEASE			(saved);

				// cleanup
				_RELEASE			(pSrcSmallTexture);
			}
			break;
		case IRender_interface::SM_NORMAL:
		{
			ID3DBlob* saved = nullptr;
			string64 t_stemp{};
			string_path	buf{};

			if (strstr(Core.Params, "-ss_jpg"))
			{
				xr_sprintf(buf, sizeof(buf), "ss_%s_%s_(%s).jpg", Core.UserName, timestamp(t_stemp), (g_pGameLevel) ? g_pGameLevel->name().c_str() : "mainmenu");
#ifdef USE_DX11
				CHK_DX(D3DX11SaveTextureToMemory(HW.pContext, pSrcTexture, D3DX11_IFF_JPG, &saved, 0));
#else
				CHK_DX(D3DX10SaveTextureToMemory(pSrcTexture, D3DX10_IFF_JPG, &saved, 0));
#endif
			}
			else if (strstr(Core.Params, "-ss_tga"))
			{ // hq
				xr_sprintf(buf, sizeof(buf), "ss_%s_%s_(%s).tga", Core.UserName, timestamp(t_stemp), (g_pGameLevel) ? g_pGameLevel->name().c_str() : "mainmenu");
#ifdef USE_DX11
				CHK_DX(D3DX11SaveTextureToMemory(HW.pContext, pSrcTexture, D3DX11_IFF_BMP, &saved, 0));
#else
				CHK_DX(D3DX10SaveTextureToMemory(pSrcTexture, D3DX10_IFF_BMP, &saved, 0));
				//		CHK_DX				(D3DXSaveSurfaceToFileInMemory (&saved,D3DXIFF_TGA,pFB,0,0));
#endif
			}
			else
			{
				xr_sprintf(buf, sizeof buf, "ss_%s_%s_(%s).png", Core.UserName, timestamp(t_stemp), (g_pGameLevel) ? g_pGameLevel->name().c_str() : "mainmenu");

#ifdef USE_DX11
				CHK_DX(D3DX11SaveTextureToMemory(HW.pContext, pSrcTexture, D3DX11_IFF_PNG, &saved, 0));
#else
				CHK_DX(D3DX10SaveTextureToMemory(pSrcTexture, D3DX10_IFF_PNG, &saved, 0));
#endif
			}

			IWriter* fs = FS.w_open("$screenshots$", buf); R_ASSERT(fs);
			fs->w(saved->GetBufferPointer(), (u32)saved->GetBufferSize());
			FS.w_close(fs);
			_RELEASE(saved);
		}
			break;
		case IRender_interface::SM_FOR_LEVELMAP:
		case IRender_interface::SM_FOR_CUBEMAP:
			{
				VERIFY(!"CRender::Screenshot. This screenshot type is not supported for DX10.");
				/*
				string64			t_stemp;
				string_path			buf;
				VERIFY				(name);
				strconcat			(sizeof(buf),buf,"ss_",Core.UserName,"_",timestamp(t_stemp),"_#",name);
				xr_strcat				(buf,".tga");
				IWriter*		fs	= FS.w_open	("$screenshots$",buf); R_ASSERT(fs);
				TGAdesc				p;
				p.format			= IMG_24B;

				//	TODO: DX10: This is totally incorrect but mimics 
				//	original behaviour. Fix later.
				hr					= pFB->LockRect(&D,0,D3DLOCK_NOSYSLOCK);
				if(hr!=D3D_OK)		return;
				hr					= pFB->UnlockRect();
				if(hr!=D3D_OK)		goto _end_;

				// save
				u32* data			= (u32*)xr_malloc(Device.dwHeight*Device.dwHeight*4);
				imf_Process			(data,Device.dwHeight,Device.dwHeight,(u32*)D.pBits,Device.dwWidth,Device.dwHeight,imf_lanczos3);
				p.scanlenght		= Device.dwHeight*4;
				p.width				= Device.dwHeight;
				p.height			= Device.dwHeight;
				p.data				= data;
				p.maketga			(*fs);
				xr_free				(data);

				FS.w_close			(fs);
				*/
			}
			break;
	}

	_RELEASE(pSrcTexture);
}

#else	//	USE_DX10

void CRender::ScreenshotImpl	(ScreenshotMode mode, LPCSTR name, CMemoryWriter* memory_writer)
{
	if (!Device.b_is_Ready)			return;

	// Create temp-surface
	IDirect3DSurface9*	pFB;
	D3DLOCKED_RECT		D;
	HRESULT				hr;
	hr					= HW.pDevice->CreateOffscreenPlainSurface(Device.dwWidth,Device.dwHeight,D3DFMT_A8R8G8B8,D3DPOOL_SYSTEMMEM,&pFB,NULL);
	if(hr!=D3D_OK)		return;

	hr					= HW.pDevice->GetFrontBufferData(0,pFB);
	if(hr!=D3D_OK)		return;

	
	hr					= pFB->LockRect(&D,0,D3DLOCK_NOSYSLOCK);
	if(hr!=D3D_OK)		return;

	// Image processing (gamma-correct)
	u32* pPixel		= (u32*)D.pBits;
	u32* pEnd		= pPixel+(Device.dwWidth*Device.dwHeight);
	//	IGOR: Remove inverse color correction and kill alpha
	/*
	D3DGAMMARAMP	G;
	dxRenderDeviceRender::Instance().gammaGenLUT(G);
	for (int i=0; i<256; i++) {
		G.red	[i]	/= 256;
		G.green	[i]	/= 256;
		G.blue	[i]	/= 256;
	}
	for (;pPixel!=pEnd; pPixel++)	{
		u32 p = *pPixel;
		*pPixel = color_xrgb	(
			G.red	[color_get_R(p)],
			G.green	[color_get_G(p)],
			G.blue	[color_get_B(p)]
			);
	}
	*/

	//	Kill alpha
	for (;pPixel!=pEnd; pPixel++)	
	{
		u32 p = *pPixel;
		*pPixel = color_xrgb	(
			color_get_R(p),
			color_get_G(p),
			color_get_B(p)
		);
	}

	hr					= pFB->UnlockRect();
	if(hr!=D3D_OK)		goto _end_;
	

	// Save
	switch (mode)	{
		case IRender_interface::SM_FOR_GAMESAVE:
			{
				// texture
				ID3DTexture2D*	texture	= NULL;
				hr					= D3DXCreateTexture(HW.pDevice,GAMESAVE_SIZE,GAMESAVE_SIZE,1,0,D3DFMT_DXT1,D3DPOOL_SCRATCH,&texture);
				if(hr!=D3D_OK)		goto _end_;
				if(NULL==texture)	goto _end_;

				// resize&convert to surface
				IDirect3DSurface9*	surface = 0;
				hr					= texture->GetSurfaceLevel(0,&surface);
				if(hr!=D3D_OK)		goto _end_;
				VERIFY				(surface);
				hr					= D3DXLoadSurfaceFromSurface(surface,0,0,pFB,0,0,D3DX_DEFAULT,0);
				_RELEASE			(surface);
				if(hr!=D3D_OK)		goto _end_;

				// save (logical & physical)
				ID3DXBuffer* saved = 0;
				hr					= D3DXSaveTextureToFileInMemory (&saved,D3DXIFF_DDS,texture,0);
				if(hr!=D3D_OK)		goto _end_;
				
				IWriter*			fs		= FS.w_open	(name); 
				if (fs)				{
					fs->w				(saved->GetBufferPointer(),saved->GetBufferSize());
					FS.w_close			(fs);
				}
				_RELEASE			(saved);

				// cleanup
				_RELEASE			(texture);
			}
			break;
		case IRender_interface::SM_NORMAL:
		{
			ID3DXBuffer* saved = 0;
			string64 t_stemp{};
			string_path	buf{};
			LPCSTR SuffixFormatImage_ = ".png";
			D3DXIMAGE_FILEFORMAT ImageFormat_ = D3DXIFF_PNG;

			if (strstr(Core.Params, "-ss_jpg"))
			{
				SuffixFormatImage_ = ".jpg";
				ImageFormat_ = D3DXIFF_JPG;
			}
			else if (strstr(Core.Params, "-ss_tga"))
			{
				SuffixFormatImage_ = ".tga";
				ImageFormat_ = D3DXIFF_TGA;
			}

			xr_sprintf(buf, sizeof(buf), "ss_%s_%s_(%s)%s", Core.UserName, timestamp(t_stemp),
				g_pGameLevel ? g_pGameLevel->name().c_str() : "mainmenu", SuffixFormatImage_);

			CHK_DX(D3DXSaveSurfaceToFileInMemory(&saved, ImageFormat_, pFB, 0, 0));

			IWriter* fs = FS.w_open("$screenshots$", buf); R_ASSERT(fs);
			fs->w(saved->GetBufferPointer(), saved->GetBufferSize());
			FS.w_close(fs);
			_RELEASE(saved);
		}
		break;
		case IRender_interface::SM_FOR_LEVELMAP:
		case IRender_interface::SM_FOR_CUBEMAP:
			{
//				string64			t_stemp;
				string_path			buf;
				VERIFY				(name);
				strconcat			(sizeof(buf), buf, name, ".tga");
				IWriter*		fs	= FS.w_open	("$screenshots$",buf); R_ASSERT(fs);
				TGAdesc				p;
				p.format			= IMG_24B;

				//	TODO: DX10: This is totally incorrect but mimics 
				//	original behavior. Fix later.
				hr					= pFB->LockRect(&D,0,D3DLOCK_NOSYSLOCK);
				if(hr!=D3D_OK)		return;
				hr					= pFB->UnlockRect();
				if(hr!=D3D_OK)		goto _end_;

				// save
				u32* data			= (u32*)xr_malloc(Device.dwHeight*Device.dwHeight*4);
				imf_Process			(data,Device.dwHeight,Device.dwHeight,(u32*)D.pBits,Device.dwWidth,Device.dwHeight,imf_lanczos3);
				p.scanlenght		= Device.dwHeight*4;
				p.width				= Device.dwHeight;
				p.height			= Device.dwHeight;
				p.data				= data;
				p.maketga			(*fs);
				xr_free				(data);

				FS.w_close			(fs);
			}
			break;
	}

_end_:
	_RELEASE		(pFB);
}

#endif	//	USE_DX10

void CRender::Screenshot(ScreenshotMode mode, LPCSTR name)
{
	ScreenshotImpl(mode, name, NULL);
}

void CRender::Screenshot(ScreenshotMode mode, CMemoryWriter& memory_writer)
{
	ScreenshotImpl(mode, NULL, &memory_writer);
}

void DoAsyncScreenshot()
{
	RImplementation.Target->DoAsyncScreenshot();
}
