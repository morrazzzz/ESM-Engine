#include "stdafx.h"
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_properties.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_dx11.h>
#include <imgui.h>
#include "xr_input.h"

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
    
    IMGUI_CHECKVERSION();
    ImguiContext = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //   io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //   io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

       // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    ImGuiStyle& style = ImGui::GetStyle();
    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplSDL3_InitForD3D(SDLWindow);
    
    /*
    IMGUI_CHECKVERSION();
    ImguiContext = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //   io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //   io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    */
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
    ImGui_ImplSDL3_Shutdown();
    SDL_DestroyWindow(SDLWindow);

    SDL_Quit();
}

void CRenderDevice::EventWindow()
{
    SDL_Event SDLWindowEvent;
    while (SDL_PollEvent(&SDLWindowEvent))
    {
        if (getImGuiActivated())
        {
            if (ImGui_ImplSDL3_ProcessEvent(&SDLWindowEvent))
                continue;
        }

        switch (SDLWindowEvent.type)
        {
        case SDL_EVENT_QUIT:
            setNeedExitGame(true);
            break;
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
        case SDL_EVENT_WINDOW_FOCUS_LOST:
        {
            bool active = SDLWindowEvent.type == SDL_EVENT_WINDOW_FOCUS_GAINED;
            SetWindowActive(active);
            break;
        }
        case SDL_EVENT_TEXT_INPUT:
        {
            pInput->TextInputProcess(SDLWindowEvent.text.text);
            break;
        }
        case SDL_EVENT_TEXT_EDITING:
            __debugbreak();
            break;
        case SDL_EVENT_MOUSE_MOTION:
            pInput->mouseX += SDLWindowEvent.motion.xrel;
            pInput->mouseY += SDLWindowEvent.motion.yrel;
            pInput->mouseMove = true;

            break;
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

bool costil = true;
void CRenderDevice::WindowNewFrameImGui()
{
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    //morrazzzz: Fake!! Need normal ImGui manager!!! For test.
    if (costil)
        ImGui::ShowDemoWindow(&costil);

    ImGui::Begin("Test");

    ImGui::End();
    
}