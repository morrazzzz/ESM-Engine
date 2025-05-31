#include "stdafx.h"
#include "resource.h"
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_properties.h>

extern LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

constexpr int BaseWeight = 800;
constexpr int BaseHeight = 600;

void CRenderDevice::Initialize()
{
	Log("Initializing Engine...");
	TimerGlobal.Start			();
	TimerMM.Start				();

	// Unless a substitute hWnd has been specified, create a window to render into
    if(!SDLWindow)
    {
        R_ASSERT2(SDL_Init(SDL_INIT_VIDEO), "Failed init SDL3!!");

        SDLWindow = SDL_CreateWindow("S.T.A.L.K.E.R.: ESM Engine", BaseWeight,
            BaseHeight, SDL_WINDOW_FULLSCREEN);
        R_ASSERT2(SDLWindow, "Failed SDL_CreateWindow! ");

        m_hWnd = static_cast<HWND>(SDL_GetPointerProperty(SDL_GetWindowProperties(SDLWindow),
            SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
    }
}

