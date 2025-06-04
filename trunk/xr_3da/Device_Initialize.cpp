#include "stdafx.h"
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_properties.h>

constexpr int BaseWidth = 800;
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
        
        SDL_WindowFlags flags = SDL_WINDOW_RESIZABLE;

        SDLWindow = SDL_CreateWindow("S.T.A.L.K.E.R.: ESM Engine", BaseWidth,
            BaseHeight, flags);
        R_ASSERT2(SDLWindow, "Failed SDL_CreateWindow!");

        m_hWnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(SDLWindow),
            SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);

        int Width, Height;
        SDL_GetWindowSize(SDLWindow, &Width, &Height);

        //Hack for GSC! Delete me!
        Device.dwWidth = static_cast<u32>(Width);
        Device.dwHeight = static_cast<u32>(Height);

        SDL_HideCursor();
        SDL_RaiseWindow(Device.SDLWindow);
    }
}

void CRenderDevice::ResizeWindow()
{
    //Hack!!
    int Width = static_cast<int>(dwWidth);
    int Height = static_cast<int>(dwHeight);

    SDL_SetWindowSize(SDLWindow, Width, Height);

    if (!b_is_Ready)
        return;

    Reset();

    seqResolutionChanged.Process(rp_ScreenResolutionChanged);
}

void CRenderDevice::SetFullscreenWindow(bool value)
{
    R_ASSERT(SDL_SetWindowFullscreen(SDLWindow, value));
    SDL_SetWindowPosition(SDLWindow, 0, 0);
}

void CRenderDevice::DestroyWindow()
{
    SDL_DestroyWindow(SDLWindow);

    SDL_Quit();
}

void CRenderDevice::EventWindow()
{
    SDL_Event SDLWindowEvent;
    if (SDL_PollEvent(&SDLWindowEvent))
    {
        switch (SDLWindowEvent.type)
        {
        case SDL_EVENT_QUIT:
            setNeedExitGame(true);
            break;
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            bool active = SDLWindowEvent.type == SDL_EVENT_WINDOW_FOCUS_GAINED;
            SetWindowActive(active);
        }
    }
}

void CRenderDevice::SetWindowActive(bool active)
{
    if (active != Device.b_is_Active)
    {
        Device.b_is_Active = active;

        if (Device.b_is_Active)
            Device.seqAppActivate.Process(rp_AppActivate);
        else
            Device.seqAppDeactivate.Process(rp_AppDeactivate);
    }
}