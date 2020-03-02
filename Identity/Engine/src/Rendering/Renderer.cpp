#include <stdafx.h>
#include <Rendering/Renderer.h>
#include <d3dcompiler.h>
#include <Tools/DirectX/dxerr.h>
#include <Tools/ImGUI/imgui.h>
#include <Tools/ImGUI/imgui_impl_dx11.h>
#include <3DLoader/ObjectLoader.h>
#include <Tools/DirectX/GraphicsMacros.h>

using namespace Engine::Rendering;

std::unique_ptr<Renderer> Renderer::instance;
Renderer::Renderer(const HWND& p_hwnd, const int& p_clientWidth, const int& p_clientHeight) :
    m_width(static_cast<float>(p_clientWidth)), m_height(static_cast<float>(p_clientHeight))
{
    HRESULT hr;

    //create the device
    GFX_THROW_INFO(
    D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &m_pDevice,
        nullptr,
        &m_pContext
    ));

    //check for msaa quality support
    GFX_THROW_INFO(m_pDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m_4xMsaaQuality));

    CreateSwapChain(p_hwnd);
    SetBackBuffer();
    SetDepthStencilBuffers();

    m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    SetViewPort(m_width, m_height);

    ImGui_ImplDX11_Init(m_pDevice.Get(), m_pContext.Get());
}

void Renderer::EndFrame() const
{
    HRESULT hr;

    if (FAILED(hr = m_pSwapChain->Present(1u, 0u)))
    {
        if (hr == DXGI_ERROR_DEVICE_REMOVED)
        {
            throw GFX_DEVICE_REMOVED_EXCEPT(m_pDevice->GetDeviceRemovedReason());
        }
        throw GFX_EXCEPT(hr);
    }
}

void Renderer::ClearBuffers(const float& p_red, const float& p_green, const float& p_blue) const
{
    const float colour[] = { p_red, p_green, p_blue, 1.0f };
    m_pContext->ClearRenderTargetView(m_pTarget.Get(), colour);
    m_pContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);
}

void Renderer::ResetContext()
{
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> nullView;
    m_pContext->OMSetRenderTargets(0, nullView.GetAddressOf(), nullptr);

    m_pTarget.Reset();
    m_pDepthStencilView.Reset();
    m_pContext->Flush();
}

void Renderer::CreateSwapChain(const HWND& p_hwnd)
{
    HRESULT hr;

    //describe the swap chain
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferDesc.Width = 0;
    swapChainDesc.BufferDesc.Height = 0;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

    //with 4x msaa
    if (m_enable4xMSAA)
    {
        swapChainDesc.SampleDesc.Count = 4;
        swapChainDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
    }
    //without
    else
    {
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
    }

    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.OutputWindow = p_hwnd;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;

    //Get the factory to generate the swapChain
    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    GFX_THROW_INFO(m_pDevice->QueryInterface(__uuidof(IDXGIDevice), &dxgiDevice));

    Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
    GFX_THROW_INFO(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), &dxgiAdapter));

    Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;
    GFX_THROW_INFO(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), &dxgiFactory));

    GFX_THROW_INFO(dxgiFactory->CreateSwapChain(m_pDevice.Get(), &swapChainDesc, &m_pSwapChain));
}

void Renderer::SetDepthStencilBuffers()
{
    HRESULT hr;

    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDSState;
    GFX_THROW_INFO(m_pDevice->CreateDepthStencilState(&dsDesc, &pDSState));

    m_pContext->OMSetDepthStencilState(pDSState.Get(), 1u);

    Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
    D3D11_TEXTURE2D_DESC descDepth;
    descDepth.Width = m_width;
    descDepth.Height = m_height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D32_FLOAT;

    //with 4x msaa
    if (m_enable4xMSAA)
    {
        descDepth.SampleDesc.Count = 4;
        descDepth.SampleDesc.Quality = m_4xMsaaQuality - 1;
    }
    //without
    else
    {
        descDepth.SampleDesc.Count = 1;
        descDepth.SampleDesc.Quality = 0;
    }

    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;

    GFX_THROW_INFO(m_pDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencil));

    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = DXGI_FORMAT_D32_FLOAT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0u;
    GFX_THROW_INFO(m_pDevice->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, &m_pDepthStencilView));

    m_pContext->OMSetRenderTargets(1u, m_pTarget.GetAddressOf(), m_pDepthStencilView.Get());
}

void Renderer::SetRenderTarget()
{
    m_pContext->OMSetRenderTargets(1u, m_pTarget.GetAddressOf(), m_pDepthStencilView.Get());
}

void Renderer::SetViewPort(const float& p_width, const float& p_height) const
{
    D3D11_VIEWPORT viewPort;
    viewPort.TopLeftX = 0;
    viewPort.TopLeftY = 0;
    viewPort.Width = p_width;
    viewPort.Height = p_height;
    viewPort.MinDepth = 0;
    viewPort.MaxDepth = 1;
    m_pContext->RSSetViewports(1u, &viewPort);
}

void Renderer::InitRenderer(const HWND p_hwnd, const int p_clientWidth, const int p_clientHeight)
{
    if (instance != nullptr)
        MessageBox(p_hwnd, "ERROR : Singleton duplication !?", "An instance was already found for Renderer. It is forbidden to have two singletons.", MB_OK | MB_ICONERROR);
    else
        instance = std::make_unique<Renderer>(p_hwnd, p_clientWidth, p_clientHeight);
}

void Renderer::SetBackBuffer()
{
    HRESULT hr;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    GFX_THROW_INFO(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer));
    GFX_THROW_INFO(m_pDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_pTarget));
}

void Renderer::Resize(const float& p_width, const float& p_height)
{
    HRESULT hr;

    m_width = p_width;
    m_height = p_height;

    ResetContext();

    if (isFullscreen)
    {
        GFX_THROW_INFO(m_pSwapChain->ResizeBuffers(
            0,
            m_fullWidth,
            m_fullHeight,
            DXGI_FORMAT_UNKNOWN,
            0
        ));
        SetViewPort(m_fullWidth, m_height);
    }
    else
    {
        GFX_THROW_INFO(m_pSwapChain->ResizeBuffers(
            0,
            m_width,
            m_height,
            DXGI_FORMAT_UNKNOWN,
            0
        ));
        SetViewPort(m_width, m_height);
    }
    m_pSwapChain->SetFullscreenState(isFullscreen, nullptr);

    SetBackBuffer();

    SetDepthStencilBuffers();
}

void Renderer::GetResolution(int& p_width, int& p_height)
{
    ChangeResolution();

    if (isFullscreen)
    {
        p_width = static_cast<int>(m_fullWidth);
        p_height = static_cast<int>(m_fullHeight);
    }
    else
    {
        p_width = static_cast<int>(m_width);
        p_height = static_cast<int>(m_height);
    }
}

void Renderer::SetFullscreen(const bool& p_state)
{
    isFullscreen = p_state;

    Resize(m_fullWidth, m_fullHeight);
}

void Renderer::ChangeResolution()
{
    const char* res[] = { "1920x1080", "1280x720", "800x600" };
    static const char* currentItem = "1920x1080";

    if (ImGui::Begin("Render Tool"))
    {
        // ImGui::SliderFloat("Camera FOV", &angle, 10.f, 180.f, "%1.f");
        if (ImGui::BeginCombo("Fullscreen Resolution", currentItem, ImGuiComboFlags_NoArrowButton))
        {
            for (auto& re : res)
            {
                const bool isSelected = (currentItem == re); // You can store your selection however you want, outside or inside your objects
                if (ImGui::Selectable(re, isSelected))
                    currentItem = re;
                if (isSelected)
                    ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
            }

            ImGui::EndCombo();
        }
    }ImGui::End();

    std::string selected = currentItem;
    std::replace(selected.begin(), selected.end(), 'x', ' ');

    std::vector<float> array;
    std::stringstream ss(selected);
    int temp;
    while(ss >> temp)
        array.push_back(temp);

    m_fullWidth = array[0];
    m_fullHeight = array[1];

    Resize(m_width, m_height);
}



#pragma region ExceptionsClass

#pragma region HrExceptionClass
Renderer::HrException::HrException(int p_line, const char* p_file, HRESULT p_hr,
    std::vector<std::string> p_infoMsg) noexcept
    : Exception(p_line, p_file),
    m_hr(p_hr)
{
    for (const auto& msg : p_infoMsg)
    {
        m_info += msg;
        m_info.push_back('\n');
    }
    if (!m_info.empty())
    {
        m_info.pop_back();
    }
}

const char* Renderer::HrException::what() const noexcept
{
    std::ostringstream oss;
    oss << GetType() << std::endl
        << "[ERROR CODE] 0x" << std::hex << std::uppercase << GetErrorCode()
        << std::dec << " (" << GetErrorCode() << ")" << std::endl
        << "[ERROR STRING] " << GetErrorString() << std::endl
        << "[DESCRIPTION] " << GetErrorDescription() << std::endl;

    if (!m_info.empty())
    {
        oss << "\n[ERROR INFO]\n" << GetErrorInfo() << std::endl;
    }

    oss << GetOriginString();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

const char* Renderer::HrException::GetType() const noexcept
{
    return "Identity Graphics Exception";
}

HRESULT Renderer::HrException::GetErrorCode() const noexcept
{
    return m_hr;
}

std::string Renderer::HrException::GetErrorString() const noexcept
{
    return DXGetErrorString(m_hr);
}

std::string Renderer::HrException::GetErrorDescription() const noexcept
{
    char buf[512];
    DXGetErrorDescription(m_hr, buf, sizeof(buf));
    return buf;
}

std::string Renderer::HrException::GetErrorInfo() const noexcept
{
    return m_info;
}
#pragma endregion

#pragma region InfoExceptionClass
Renderer::InfoException::InfoException(int p_line, const char* p_file, std::vector<std::string> p_infoMsg) noexcept
    : Exception(p_line, p_file)
{
    for (const auto& msg : p_infoMsg)
    {
        m_info += msg;
        m_info.push_back('\n');
    }
    if (!m_info.empty())
    {
        m_info.pop_back();
    }
}

const char* Renderer::InfoException::what() const noexcept
{
    std::ostringstream oss;
    oss << GetType() << std::endl
        << "\n[ERROR INFO]\n" << GetErrorInfo() << std::endl << std::endl;
    oss << GetOriginString();
    whatBuffer = oss.str();
    return whatBuffer.c_str();
}

const char* Renderer::InfoException::GetType() const noexcept
{
    return "Identity Graphics Info Exception";
}

std::string Renderer::InfoException::GetErrorInfo() const noexcept
{
    return m_info;
}
#pragma endregion

#pragma region DeviceExceptionClass
const char* Renderer::DeviceException::GetType() const noexcept
{
    return "Identity Graphics Exception [DEVICE REMOVED] (DXGI_ERROR_DEVICE_REMOVED)";
}
#pragma endregion

#pragma endregion