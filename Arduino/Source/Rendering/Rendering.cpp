#include "Rendering.hpp"
#include "Interface.hpp"
#include "Fonts.hpp"

void Rendering::CreateWindowAndDX11()
{
    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGuiDX11App", nullptr };
    if (!RegisterClassExW(&wc))
    {
        throw std::runtime_error("Failed to register window class: " + std::to_string(GetLastError()));
    }

    hwnd = CreateWindowW(L"ImGuiDX11App", L"ImGui DX11 Example", WS_OVERLAPPEDWINDOW, 100, 100, 770, 350, nullptr, nullptr, wc.hInstance, this);
    if (!hwnd)
    {
        throw std::runtime_error("Failed to create window: " + std::to_string(GetLastError()));
    }

    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = 770;
    scd.BufferDesc.Height = 350;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &scd, &swap_chain, &device, nullptr, &device_context);
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create DX11 device and swap chain: " + std::to_string(hr));
    }

    ID3D11Texture2D* back_buffer = nullptr;
    hr = swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to get back buffer: " + std::to_string(hr));
    }

    hr = device->CreateRenderTargetView(back_buffer, nullptr, &render_target_view);
    back_buffer->Release();
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create render target view: " + std::to_string(hr));
    }
}

void Rendering::InitializeImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImFontConfig config;
    const ImWchar iconRanges[] = {
        0xe005,
        0xf8ff,
        0
    };

    config.FontDataOwnedByAtlas = false;
    m_default_font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\Arial.ttf", 18.f);
    m_icon_font = io.Fonts->AddFontFromMemoryTTF((void*)Fonts::Iconfont, Fonts::IconfontSize, 16.f, &config, iconRanges);

    if (m_default_font == nullptr)
    {
        throw std::runtime_error("Failed to load default font!"); 
        return;
    }
    
    if (m_icon_font == nullptr)
    {
        throw std::runtime_error("Failed to load icon font!"); 
        return; 
    }

    ImGui::StyleColorsDark();

    if (!ImGui_ImplWin32_Init(hwnd))
    {
        throw std::runtime_error("Failed to initialize ImGui Win32 backend");
    }

    if (!ImGui_ImplDX11_Init(device, device_context))
    {
        throw std::runtime_error("Failed to initialize ImGui DX11 backend");
    }
}

void Rendering::Run()
{
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (msg.message != WM_QUIT)
    {
        if (PeekMessageW(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            continue;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        {
            g_Interface->InitStyle();
            g_Interface->Render();
        }
        float clear_color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
        device_context->OMSetRenderTargets(1, &render_target_view, nullptr);
        device_context->ClearRenderTargetView(render_target_view, clear_color);
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        swap_chain->Present(1, 0);
        //std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
    }
}

void Rendering::ResizeBuffers(UINT width, UINT height)
{
    if (!swap_chain || !device_context || width == 0 || height == 0) return;

    SAFE_RELEASE(render_target_view);

    HRESULT hr = swap_chain->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to resize swap chain buffers: " + std::to_string(hr));
    }

    ID3D11Texture2D* back_buffer = nullptr;
    hr = swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to get back buffer after resize: " + std::to_string(hr));
    }

    hr = device->CreateRenderTargetView(back_buffer, nullptr, &render_target_view);
    back_buffer->Release();
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create render target view after resize: " + std::to_string(hr));
    }
}

LRESULT CALLBACK Rendering::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
    {
        return true;
    }

    Rendering* app = nullptr;
    if (msg == WM_CREATE)
    {
        CREATESTRUCT* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
        app = static_cast<Rendering*>(createStruct->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
        BOOL dark = TRUE;
        DwmSetWindowAttribute(hwnd, 20, &dark, sizeof(dark));
    }
    else
    {
        app = reinterpret_cast<Rendering*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    switch (msg)
    {
        case WM_SIZE:
        {
            if (app && wParam != SIZE_MINIMIZED)
            {
                UINT width = LOWORD(lParam);
                UINT height = HIWORD(lParam);
                app->ResizeBuffers(width, height);
            }
            return 0;
        }
        break;
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
        break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}