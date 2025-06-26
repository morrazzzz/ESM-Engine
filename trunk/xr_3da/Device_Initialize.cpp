#include "stdafx.h"
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_properties.h>
#include <backends/imgui_impl_sdl3.h>
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
   
    ImGui_ImplSDL3_InitForD3D(SDLWindow);
    
    /*
    IMGUI_CHECKVERSION();
    ImguiContext = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //   io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //   io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    */
}

void CRenderDevice::InitializeImGuiContext()
{
    IMGUI_CHECKVERSION();
    ImguiContext = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //   io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //   io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

//    io.MouseDrawCursor = true;

    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\sitkavf.ttf", 15.0f, nullptr, io.Fonts->GetGlyphRangesCyrillic());

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

    ImGuiTask = new Concurrency::task_group();
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
            ImGui_ImplSDL3_ProcessEvent(&SDLWindowEvent);

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
        case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
        case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
        {
            Reset();
            break;
        }
        case SDL_EVENT_TEXT_INPUT:
        {
            if (getImGuiActivated())
                break;

            pInput->TextInputProcess(SDLWindowEvent.text.text);
            break;
        }
        case SDL_EVENT_KEY_UP:
            if (SDLWindowEvent.key.scancode == SDL_SCANCODE_F9)
                Device.setImGuiActivated(!Device.getImGuiActivated());
            break;
        case SDL_EVENT_TEXT_EDITING:
            __debugbreak();
            break;
        case SDL_EVENT_MOUSE_MOTION:
            if (getImGuiActivated())
                break;

            pInput->SetMouseMotion(SDLWindowEvent.motion.xrel,
                SDLWindowEvent.motion.yrel);

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
        {
//            SDL_MaximizeWindow(SDLWindow);
            Device.seqAppActivate.Process(rp_AppActivate);
        }
        else
        {
//            SDL_MinimizeWindow(SDLWindow);
            Device.seqAppDeactivate.Process(rp_AppDeactivate);
        }
    }
}

void CRenderDevice::WindowNewFrameImGui()
{
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();    
}

bool OldRelativeMode = false;
void CRenderDevice::setImGuiActivated(bool value)
{
    ImGuiActivated = value;
    
    if (value && SDL_GetWindowRelativeMouseMode(SDLWindow))
    {
        OldRelativeMode = true;
        pInput->SetInputRelativeMouseMode(false);
    }

    if (!value)
    {
        SDL_HideCursor();

        if (OldRelativeMode)
            pInput->SetInputRelativeMouseMode(true);

        if (GetWindowActiveTextInput())
            StopWindowTextInput();
    }
}

void CRenderDevice::StartWindowTextInput()
{
    SDL_StartTextInput(Device.SDLWindow);
}

void CRenderDevice::StopWindowTextInput()
{
    SDL_StopTextInput(Device.SDLWindow);
}

bool CRenderDevice::GetWindowActiveTextInput() const
{
    return SDL_TextInputActive(Device.SDLWindow);
}