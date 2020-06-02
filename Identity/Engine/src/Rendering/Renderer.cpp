#include <stdafx.h>

#include <d3dcompiler.h>

#include <3DLoader/ObjectLoader.h>
#include <Managers/ResourceManager.h>
#include <Rendering/Renderer.h>
#include <Tools/DirectX/dxerr.h>
#include <Tools/ImGUI/imgui.h>
#include <Tools/ImGUI/imgui_impl_dx11.h>
#include <Tools/DirectX/GraphicsMacros.h>

using namespace Engine::Rendering;

constexpr bool V_SYNC = false;

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

    SetViewPort(m_width, m_height);
    SetBackBuffer();
    SetDepthStencilBuffers();
    m_pContext->OMSetRenderTargets(0, m_pTarget.GetAddressOf(), nullptr);

    m_pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ImGui_ImplDX11_Init(m_pDevice.Get(), m_pContext.Get());

}

void Renderer::EndFrame() const
{
    HRESULT hr;

    if (V_SYNC)
    {
        if (FAILED(hr = m_pSwapChain->Present(1u, 0u)))
        {
            if (hr == DXGI_ERROR_DEVICE_REMOVED)
            {
                throw GFX_DEVICE_REMOVED_EXCEPT(m_pDevice->GetDeviceRemovedReason());
            }
            throw GFX_EXCEPT(hr);
        }
    }
    else
    {
        if (FAILED(hr = m_pSwapChain->Present(0u, 0u)))
        {
            if (hr == DXGI_ERROR_DEVICE_REMOVED)
            {
                throw GFX_DEVICE_REMOVED_EXCEPT(m_pDevice->GetDeviceRemovedReason());
            }
            throw GFX_EXCEPT(hr);
        }
    }
}   

void Renderer::ClearBuffers(const float& p_red, const float& p_green, const float& p_blue) const
{
    const float colour[] = { p_red, p_green, p_blue, 1.0f };
    
    m_pContext->ClearRenderTargetView(GetTarget().Get(), colour);
    m_pContext->ClearDepthStencilView(GetDepthStencilView().Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);

    for (auto& RT : m_renderTextures)
    {
        m_pContext->ClearRenderTargetView(RT.GetTarget().Get(), colour);
        if(*RT.GetDepthStencilView().GetAddressOf() != nullptr)
            m_pContext->ClearDepthStencilView(RT.GetDepthStencilView().Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);
    }
}

void Renderer::Bind(const bool p_bindDefaultDepthStencil)
{
    if (p_bindDefaultDepthStencil)
        m_pContext->OMSetRenderTargets(1, m_pTarget.GetAddressOf(), *m_pDepthStencilView.GetAddressOf());
    else
        m_pContext->OMSetRenderTargets(1, m_pTarget.GetAddressOf(), nullptr);
}

void Renderer::Bind(Microsoft::WRL::ComPtr<ID3D11RenderTargetView> p_target, const bool p_bindDefaultDepthStencil) const
{
    if (p_bindDefaultDepthStencil)
        m_pContext->OMSetRenderTargets(1, p_target.GetAddressOf(), *m_pDepthStencilView.GetAddressOf());
    else
        m_pContext->OMSetRenderTargets(1, p_target.GetAddressOf(), nullptr);

}

void Renderer::Bind(Microsoft::WRL::ComPtr<ID3D11RenderTargetView> p_target, const Microsoft::WRL::ComPtr<ID3D11DepthStencilView> p_ds) const
{
    m_pContext->OMSetRenderTargets(1, p_target.GetAddressOf(), *p_ds.GetAddressOf());
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
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
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
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

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

    GFX_THROW_INFO(m_pDevice->CreateTexture2D(&descDepth, NULL, &pDepthStencil));

    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0u;
    GFX_THROW_INFO(m_pDevice->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, &m_pDepthStencilView));
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
    {
        instance = std::make_unique<Renderer>(p_hwnd, p_clientWidth, p_clientHeight);
        instance->CreateRenderTexture();
        instance->CreateRect();
    }
}

void Renderer::SetBackBuffer()
{
    HRESULT hr;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    GFX_THROW_INFO(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer));
    GFX_THROW_INFO(m_pDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_pTarget));
}

void Renderer::CreateRenderTexture()
{
    const RenderTexture sceneRenderTexture{ static_cast<UINT>(m_width), static_cast<UINT>(m_height), true };
    m_renderTextures.push_back(sceneRenderTexture);
}

void Renderer::CreateRect()
{
    // I have to invert Y value for some reason I don't understand yet.
    std::vector<Geometry::Vertex> quadvtx{ Geometry::Vertex{ Vector3F{-1.0f, 1.0f, 0.f}, Vector2F{0.0f, -1.0f}, Vector3F::zero },
                                           Geometry::Vertex{ Vector3F{1.0f, 1.0f, 0.f}, Vector2F{1.0f, -1.0f}, Vector3F::zero },
                                           Geometry::Vertex{ Vector3F{-1.0f, -1.0f, 0.f}, Vector2F{0.0f, 0.0f}, Vector3F::zero },
                                           Geometry::Vertex{ Vector3F{1.0f, -1.0f, 0.f}, Vector2F{1.0f, 0.0f}, Vector3F::zero } };
    std::vector<unsigned short> quadidx = { 3,2,1,1,2,0 };

    //The quad is the screen "camera rect" we might want to store it somewhere later.
    m_rect = std::make_shared<ObjectElements::Mesh>(quadvtx, quadidx);
    m_rect->SetMaterial(Managers::ResourceManager::GetMaterial("RenderText"));
    m_rect->GetMaterial()->SetTextureState(true);
}

void Renderer::Resize(const float p_width, const float p_height)
{
    HRESULT hr;

    if (p_width == 0.0f || p_height == 0.0f)
        return;

    m_width = p_width;
    m_height = p_height;

    ResetContext();

    if (m_isFullscreen)
    {
        GFX_THROW_INFO(m_pSwapChain->ResizeBuffers(
            0,
            m_fullWidth,
            m_fullHeight,
            DXGI_FORMAT_UNKNOWN,
            0
        ));
        SetViewPort(m_fullWidth, m_fullHeight);
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
    m_pSwapChain->SetFullscreenState(m_isFullscreen, nullptr);

    SetBackBuffer();
    SetDepthStencilBuffers();
}

void Renderer::GetResolution(int& p_width, int& p_height) const
{
    if (m_isFullscreen)
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
    m_isFullscreen = p_state;

    Resize(m_fullWidth, m_fullHeight);
}

void Renderer::ChangeResolution()
{
    const char* res[] = { "1920x1080", "1280x720", "800x600" };
    static const char* currentItem = "1920x1080";

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
Renderer::HrException::HrException(int p_line, const char* p_file, const HRESULT p_hr,
                                   const std::vector<std::string>& p_infoMsg) noexcept
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
Renderer::InfoException::InfoException(const int p_line, const char* p_file, const std::vector<std::string>& p_infoMsg) noexcept
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
