#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_internal.h"
#include "fonts.h"
#include "images.h"
#include <d3d11.h>
#include <tchar.h>
#include "../../../../../../Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include/D3DX11tex.h"

// Data
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

//Helpers
ImDrawList* drawlist;
ImVec2 pos;
int tabs = 0;
ImFont* icons;
ID3D11ShaderResourceView* background = nullptr;
ID3D11ShaderResourceView* playermodel = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    D3DX11_IMAGE_LOAD_INFO info;
    ID3DX11ThreadPump* pump{ nullptr };
    D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, backgroundhxd, sizeof(backgroundhxd), &info,
        pump, &background, 0);

    D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, playermodelhxd, sizeof(playermodelhxd), &info,
        pump, &playermodel, 0);

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load Fonts
    ImFont* mainfont = io.Fonts->AddFontFromMemoryTTF(mainfonthxd, sizeof(mainfonthxd), 18.f, NULL, io.Fonts->GetGlyphRangesCyrillic());
    icons = io.Fonts->AddFontFromMemoryTTF(iconshxd, sizeof(iconshxd), 18.f, NULL, io.Fonts->GetGlyphRangesCyrillic());

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        static ImVec2 esp_preview_pos;

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        {
            ImGui::GetBackgroundDrawList()->AddImage(background, ImVec2(0, 0), ImVec2(1920, 1080), ImVec2(0, 0), ImVec2(1, 1), ImColor(150, 150, 150, 255));

            ImGui::Begin("Hello, world!", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

            //basic sizing
            ImGui::SetWindowSize(ImVec2(710, 530));

            //helpers def
            drawlist = ImGui::GetWindowDrawList();
            pos = ImGui::GetWindowPos();

            //for the esp preview
            esp_preview_pos = ImGui::GetWindowPos() + ImVec2(730, 45);

            //buttons
            drawlist->AddCircleFilled(ImVec2(pos.x + 15, pos.y + 15), 5.f, ImColor(254, 94, 87)); //red
            drawlist->AddCircleFilled(ImVec2(pos.x + 35, pos.y + 15), 5.f, ImColor(250, 188, 56)); //orange
            drawlist->AddCircleFilled(ImVec2(pos.x + 55, pos.y + 15), 5.f, ImColor(116, 170, 92)); //green

            //search part
            drawlist->AddLine(ImVec2(pos.x, pos.y + 35), ImVec2(pos.x + 190, pos.y + 35), ImColor(50, 50, 50));

            //tabs
            ImGui::SetCursorPos(ImVec2(20, 55));
            ImGui::BeginGroup();
            {
                if (ImGui::TabButton("Aim Settings", "A", ImVec2(150, 35), tabs == 0))
                    tabs = 0;

                if (ImGui::TabButton("Visuals", "B", ImVec2(150, 35), tabs == 1))
                    tabs = 1;

                if (ImGui::TabButton("Exploits", "C", ImVec2(150, 35), tabs == 2))
                    tabs = 2;

                if (ImGui::TabButton("Misc", "D", ImVec2(150, 35), tabs == 3))
                    tabs = 3;
            }
            ImGui::EndGroup();

            //separator
            drawlist->AddLine(ImVec2(pos.x + 190, pos.y), ImVec2(pos.x + 190, pos.y + 530), ImColor(50, 50, 50));

            //functions
            if (tabs == 0) // aimbot
            {
                static int weaponconfig = 0;
                ImGui::SetCursorPos(ImVec2(210, -16));
                ImGui::SetNextItemWidth(120);
                ImGui::Combo("WeaponConfig", &weaponconfig, "Select Weapon\0\Assault Rifle\0\SMG\0\Shotgun\0\Others");

                drawlist->AddLine(ImVec2(pos.x + 190, pos.y + 50), ImVec2(pos.x + 710, pos.y + 50), ImColor(50, 50, 50));

                ImGui::SetCursorPos(ImVec2(210, 70));
                ImGui::BeginChild("Child0", ImVec2(230, 440), true);
                {
                    //functions for the demo
                    static bool checkbox[4];
                    static int key = 0;

                    ImGui::SetCursorPos(ImVec2(15, 20));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox("Enable Aimbot", &checkbox[0]);
                        ImGui::Checkbox("Enable Smoothing", &checkbox[1]);
                        ImGui::Checkbox("Distance Limit", &checkbox[2]);
                        ImGui::Checkbox("Weapon Config", &checkbox[3]);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();


                ImGui::SetCursorPos(ImVec2(460, 70));
                ImGui::BeginChild("Child1", ImVec2(230, 440), true);
                {
                    //functions for the demo
                    static int sliderint0 = 0;
                    static int sliderint1 = 0;
                    static int sliderint2 = 0;
                    static int sliderint3 = 0;
                    static int combo = 0;

                    ImGui::SetCursorPos(ImVec2(15, 20));
                    ImGui::BeginGroup();
                    {
                        ImGui::Combo("Aim Bone", &combo, "Head\0\Neck\0\Chest");
                        ImGui::SliderInt("Smoothing Value", &sliderint0, 0, 100);
                        ImGui::SliderInt("FOV Value", &sliderint1, 0, 100);
                        ImGui::SliderInt("Shotgun FOV & Smooth", &sliderint2, 0, 100);
                        ImGui::SliderInt("Max Distance", &sliderint3, 0, 1000);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }

            //functions for the demo
            static bool espcheckbox[6];

            if (tabs == 1) // visuals
            {
                ImGui::SetCursorPos(ImVec2(210, 20));
                ImGui::BeginChild("Child0", ImVec2(230, 490), true);
                {
                    ImGui::SetCursorPos(ImVec2(15, 20));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox("Box", &espcheckbox[0]);
                        ImGui::Checkbox("Skeleton", &espcheckbox[1]);
                        ImGui::Checkbox("Distance", &espcheckbox[2]);
                        ImGui::Checkbox("Team Check", &espcheckbox[3]);
                        ImGui::Checkbox("Platform", &espcheckbox[4]);
                        ImGui::Checkbox("Held Item/Weapon", &espcheckbox[5]);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();


                ImGui::SetCursorPos(ImVec2(460, 20));
                ImGui::BeginChild("Child1", ImVec2(230, 490), true);
                {
                    //functions for the demo
                    static int sliderint0 = 0;
                    static int sliderint1 = 0;

                    ImGui::SetCursorPos(ImVec2(15, 20));
                    ImGui::BeginGroup();
                    {
                        ImGui::SliderInt("Skeleton Thickness", &sliderint0, 0, 10);
                        ImGui::SliderInt("Max Distance", &sliderint1, 0, 1000);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                ImGui::SetNextWindowPos(esp_preview_pos, ImGuiCond_Always);
                ImGui::SetNextWindowSize(ImVec2(250, 440));
                ImGui::Begin("ESP Preview", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                {
                    //helpers redef so it locks on the window
                    drawlist = ImGui::GetWindowDrawList();
                    pos = ImGui::GetWindowPos();

                    //title
                    ImGui::SetCursorPos(ImVec2(15, 10));
                    ImGui::Text("ESP Preview");

                    //separator
                    drawlist->AddLine(ImVec2(pos.x, pos.y + 40), ImVec2(pos.x + 250, pos.y + 40), ImColor(38, 37, 44), 1.f);

                    //load model
                    drawlist->AddImage((PVOID)playermodel, ImVec2(pos.x - 70, pos.y + 90), ImVec2(pos.x + 330, pos.y + 375), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255));

                    //box
                    if (espcheckbox[0])
                    {
                        //left top
                        drawlist->AddLine(ImVec2(pos.x + 30, pos.y + 85), ImVec2(pos.x + 70, pos.y + 85), ImColor(255, 255, 255));
                        drawlist->AddLine(ImVec2(pos.x + 30, pos.y + 85), ImVec2(pos.x + 30, pos.y + 125), ImColor(255, 255, 255));

                        //right top
                        drawlist->AddLine(ImVec2(pos.x + 190, pos.y + 85), ImVec2(pos.x + 230, pos.y + 85), ImColor(255, 255, 255));
                        drawlist->AddLine(ImVec2(pos.x + 230, pos.y + 85), ImVec2(pos.x + 230, pos.y + 125), ImColor(255, 255, 255));

                        //left bottom
                        drawlist->AddLine(ImVec2(pos.x + 30, pos.y + 385), ImVec2(pos.x + 70, pos.y + 385), ImColor(255, 255, 255));
                        drawlist->AddLine(ImVec2(pos.x + 30, pos.y + 345), ImVec2(pos.x + 30, pos.y + 385), ImColor(255, 255, 255));

                        //right bottom
                        drawlist->AddLine(ImVec2(pos.x + 190, pos.y + 385), ImVec2(pos.x + 230, pos.y + 385), ImColor(255, 255, 255));
                        drawlist->AddLine(ImVec2(pos.x + 230, pos.y + 345), ImVec2(pos.x + 230, pos.y + 385), ImColor(255, 255, 255));
                    }

                    //distance
                    if (espcheckbox[2])
                    {
                        ImGui::SetCursorPos(ImVec2(108, 416));
                        ImGui::TextColored(ImColor(255, 255, 255), "1337m");
                    }

                    //weapon
                    if (espcheckbox[5])
                    {
                        ImGui::SetCursorPos(ImVec2(102, 396));
                        ImGui::TextColored(ImColor(255, 255, 255), "Weapon");
                    }

                    //skeleton
                    if (espcheckbox[1])
                    {
                        //body
                        drawlist->AddLine(ImVec2(pos.x + 125, pos.y + 150), ImVec2(pos.x + 125, pos.y + 205), ImColor(255, 255, 255), 2.f);

                        //left hand
                        drawlist->AddLine(ImVec2(pos.x + 90, pos.y + 160), ImVec2(pos.x + 125, pos.y + 160), ImColor(255, 255, 255), 2.f);
                        drawlist->AddLine(ImVec2(pos.x + 60, pos.y + 178), ImVec2(pos.x + 90, pos.y + 160), ImColor(255, 255, 255), 2.f);

                        //right hand
                        drawlist->AddLine(ImVec2(pos.x + 125, pos.y + 160), ImVec2(pos.x + 155, pos.y + 160), ImColor(255, 255, 255), 2.f);
                        drawlist->AddLine(ImVec2(pos.x + 155, pos.y + 160), ImVec2(pos.x + 170, pos.y + 175), ImColor(255, 255, 255), 2.f);

                        //left leg
                        drawlist->AddLine(ImVec2(pos.x + 125, pos.y + 204), ImVec2(pos.x + 105, pos.y + 225), ImColor(255, 255, 255), 2.f);
                        drawlist->AddLine(ImVec2(pos.x + 105, pos.y + 225), ImVec2(pos.x + 107, pos.y + 300), ImColor(255, 255, 255), 2.f);

                        //right leg
                        drawlist->AddLine(ImVec2(pos.x + 125, pos.y + 204), ImVec2(pos.x + 155, pos.y + 225), ImColor(255, 255, 255), 2.f);
                        drawlist->AddLine(ImVec2(pos.x + 155, pos.y + 225), ImVec2(pos.x + 170, pos.y + 300), ImColor(255, 255, 255), 2.f);
                    }
                }
                ImGui::End();
            }

            if (tabs == 2) // exploits
            {
                ImGui::SetCursorPos(ImVec2(210, 20));
                ImGui::BeginChild("Child0", ImVec2(230, 490), true);
                {
                    //functions for the demo
                    static bool checkbox[6];

                    ImGui::SetCursorPos(ImVec2(15, 20));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox("Triggerbot", &checkbox[0]);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }

            if (tabs == 3) // misc
            {
                ImGui::SetCursorPos(ImVec2(210, 20));
                ImGui::BeginChild("Child0", ImVec2(230, 490), true);
                {
                    //functions for the demo
                    static bool checkbox[6];

                    ImGui::SetCursorPos(ImVec2(15, 20));
                    ImGui::BeginGroup();
                    {
                        ImGui::Checkbox("Radar", &checkbox[0]);
                        ImGui::Checkbox("V-Sync", &checkbox[1]);
                        ImGui::Checkbox("Crosshair", &checkbox[2]);
                        ImGui::Checkbox("Auto Aim", &checkbox[3]);
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();
            }

            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
