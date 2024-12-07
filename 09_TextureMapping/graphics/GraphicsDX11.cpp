#include "GraphicsDX11.h"
#include "imgui_impl_dx11.h"
#include "mathutils.h"
#include "ResourceManager.h"

#include "framework.h"
#include <dxgidebug.h>

// Debug names for some of the D3D11 resources we'll be creating
#ifdef _DEBUG
constexpr char c_gridVertexBufferID[] = "gridVertexBuffer";
constexpr char c_gridIndexBufferID[] = "gridIndexBuffer";
constexpr char c_constantBufferID[] = "matrixConstantBuffer";
constexpr char c_lightConstantBufferID[] = "lightconstantBuffer";
constexpr char c_depthStencilBufferID[] = "depthStencilBuffer";
constexpr char c_rasterizerStateID[] = "rasterizerState";
#endif // DEBUG

/// @brief Utility function for getting the Texture that represents the backbuffer
/// @param swapChain DXGI Swapchain to work from
/// @return a D3D11 Texture 2D to work with
ID3D11Texture2D* GraphicsDX11::GetBackBuffer(IDXGISwapChain* swapChain)
{
    ID3D11Texture2D* backBuffer = nullptr;
    auto hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
    if (FAILED(hr))
    {
        // Handle error
        OutputDebugStringA("Failed to get the Back buffer from the swapchain. Aborting.\n");
        return nullptr;
    }

    return backBuffer;
}

/// @brief Create the D3D11 Device and Device Context
/// @param hWnd Handle to the window
/// @param width Window Width
/// @param height Window Height
/// @param ppDevice Reference to the D3D11 device to populate
/// @param ppContext Reference to the D3D11 Context to populate
/// @param ppSwapChain Reference to the DXGI swapchain to populate
/// @return S_OK if we're successfully created the context and device
HRESULT GraphicsDX11::CreateD3D11DeviceAndContext(HWND hWnd,
    UINT width,
    UINT height)
{
    PLOG_INFO << "Initializing D3D11 Device and Context";

    // Define swap chain descriptor
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.OutputWindow = hWnd;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.Flags = 0;

    // Create device, context, and swap chain
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
    auto hr = D3D11CreateDeviceAndSwapChain(
        nullptr,                  // First Parameter
        D3D_DRIVER_TYPE_HARDWARE, // Second Parameter
        nullptr,                  // Third Parameter
#ifdef _DEBUG
        D3D11_CREATE_DEVICE_DEBUG, // Fourth Parameter - use D3D11_CREATE_DEVICE_DEBUG
                                   // if you want additional debug spew in the console.
#else
        0, // Fourth Parameter
#endif                     // DEBUG
        &featureLevel,     // Fifth Parameter
        1,                 // Sixth Parameter
        D3D11_SDK_VERSION, // Seventh Parameter
        &swapChainDesc,    // Eighth Parameter
        &m_SwapChain,      // Ninth Parameter
        &m_D3DDevice,      // Tenth Parameter
        nullptr,           // Eleventh Parameter
        &m_D3DContext);    // Twelfth Parameter

    if (FAILED(hr))
    {
        PLOG_ERROR << "Failed calling D3D11CreateDeviceAndSwapChain.";
        return hr;
    }

    return S_OK;
}

/// @brief Create the D3D11 Context
/// @param device D3D11 Device to use for creating the D3D11 Context from
/// @param context Reference to the context to populate
/// @return
HRESULT GraphicsDX11::CreateD3D11Context(ID3D11Device* device, ID3D11DeviceContext** context)
{
    PLOG_INFO << "Creating the D3D11 Context";

    if (device == nullptr || context == nullptr)
    {
        PLOG_ERROR << "Calling GraphicsDX11::CreateD3D11Context with either a null device or context";
        return E_INVALIDARG;
    }

    // Create the D3D11 context
    auto hr = device->CreateDeferredContext(0, context);
    if (FAILED(hr))
    {
        PLOG_ERROR << "Failed calling 'CreateDeferredContext' with the given D3D11Device ";
        return hr;
    }

    return S_OK;
}

/// @brief Load the Shader from file and compile the Vertex and Pixel shaders
/// @return S_OK if we were able to compile the shaders
HRESULT GraphicsDX11::LoadAndCompileShaders()
{
    m_texturedShader.Compile(m_D3DDevice, L"vsTexturedShader.hlsl", L"psTexturedShader.hlsl", IALayout_VertexColorNormalUV);
    m_lightGeometryShader.Compile(m_D3DDevice, L"LightGeometry.hlsl", IALayout_VertexColor);
    m_simpleLit.Compile(m_D3DDevice, L"SimpleLit.hlsl", IALayout_VertexColorNormal);
    return m_shader.Compile(m_D3DDevice, L"CombinedShader02.hlsl", IALayout_VertexColor);
}

/// @brief Create the Vertex and Index buffers, as well as the input layout
/// @return S_OK if we are able to create the buffers and input layout
HRESULT GraphicsDX11::CreateVertexAndIndexBuffers()
{
    PLOG_INFO << "Creating the vertex and Index Buffers for General Purpose use";

    D3D11_BUFFER_DESC constantBufferDesc = {};
    constantBufferDesc.ByteWidth = sizeof(MatrixConstantBuffer);
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (FAILED(m_D3DDevice->CreateBuffer(&constantBufferDesc, nullptr, &m_mvpConstantBuffer)))
    {
        PLOG_ERROR << "Failed to create a new constant buffer.";
        return S_FALSE;
    }

    D3D11_BUFFER_DESC lightConstantBufferDesc = {};
    lightConstantBufferDesc.ByteWidth = sizeof(LightConstantBuffer);
    lightConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    lightConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (FAILED(m_D3DDevice->CreateBuffer(&lightConstantBufferDesc, nullptr, &m_lightConstantBuffer)))
    {
        PLOG_ERROR << "Failed to create a new lighting constant buffer.";
        return S_FALSE;
    }

#ifdef _DEBUG
    m_mvpConstantBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_constantBufferID) - 1, c_constantBufferID);
    m_lightConstantBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_lightConstantBufferID) - 1, c_lightConstantBufferID);
#endif // DEBUG

    m_cube.Initialize(m_D3DDevice);
    m_grid.Initialize(m_D3DDevice);
    m_plane.Initialize(m_D3DDevice);
    m_light.Initialize(m_D3DDevice);
    m_sphere.Initialize(m_D3DDevice, 0.25f, 12, 6);
    m_gizmoXYZ01.LoadFromFile(m_D3DContext, "gizmoxyz.fbx");
    m_gizmoXYZ02.LoadFromFile(m_D3DContext, "gizmoxyz.fbx");
    m_texturedMesh.LoadFromFile(m_D3DContext, "brickCube.fbx");

    return S_OK;
}

/// @brief Create the depth/stencil buffer as well as the rasterizer state objects
/// @return S_OK if we are able to create these objects
HRESULT GraphicsDX11::CreateDepthStencilAndRasterizerState()
{
    PLOG_INFO << "Creating the Depth Stencil and Rasterizer State";

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

    m_D3DDevice->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);

#ifdef _DEBUG
    m_depthStencilState->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_depthStencilBufferID) - 1, c_depthStencilBufferID);
#endif // DEBUG

    D3D11_RASTERIZER_DESC rasterizerDesc = {};

    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.AntialiasedLineEnable = TRUE;

    if (m_fillmode == FillMode::Solid)
        rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    else
        rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;

    switch (m_cullmode)
        {
        case CullMode::None:
            rasterizerDesc.CullMode = D3D11_CULL_NONE;
            break;
        case CullMode::Backface:
            rasterizerDesc.CullMode = D3D11_CULL_BACK;
            break;
        case CullMode::Frontface:
            rasterizerDesc.CullMode = D3D11_CULL_FRONT;
            break;
        default:
            break;
    }

    m_D3DDevice->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState);

#ifdef _DEBUG
    m_rasterizerState->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_rasterizerStateID) - 1, c_rasterizerStateID);
#endif // DEBUG

    return S_OK;
}

/// @brief Create all D3D11 resources we will need for this application
/// @return S_OK if successful
HRESULT GraphicsDX11::CreateD3DResources()
{
    PLOG_INFO << "Creating D3D Resources";

    if (!SUCCEEDED(LoadAndCompileShaders()))
    {
        PLOG_ERROR << "Unable to load and compile shaders.";
        return S_FALSE;
    }

    if (!SUCCEEDED(CreateVertexAndIndexBuffers()))
    {
        PLOG_ERROR << "Unable to create Vertex and index buffers for our default geometry";
        return S_FALSE;
    }

    if (!SUCCEEDED(CreateDepthStencilAndRasterizerState()))
    {
        PLOG_ERROR << "Unable to create the Depth Stencil and set the Rasterizer state";
        return S_FALSE;
    }

    return S_OK;
}

/// @brief Render off a frame
/// @param hWnd Handle to the window
/// @param winRect RECT that defines the window to render to
void GraphicsDX11::Render(HWND hWnd, RECT winRect, GameData& data, double increment)
{
    // Update constant buffer
    {
        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        m_D3DContext->Map(m_mvpConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
        MatrixConstantBuffer* constants = (MatrixConstantBuffer*)(mappedSubresource.pData);
        constants->mWorld = DirectX::XMMatrixTranspose(m_World);
        constants->mModelViewProjection = DirectX::XMMatrixTranspose(m_MVP);
        m_D3DContext->Unmap(m_mvpConstantBuffer, 0);

        D3D11_MAPPED_SUBRESOURCE lightMappedSubresource;
        m_D3DContext->Map(m_lightConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &lightMappedSubresource);
        LightConstantBuffer* lightConstantBuffer = (LightConstantBuffer*)(lightMappedSubresource.pData);
        lightConstantBuffer->mLightPosition = DirectX::XMFLOAT4(data.m_Light.m_LightPosition[0], data.m_Light.m_LightPosition[1], data.m_Light.m_LightPosition[2], 0.0f);
        lightConstantBuffer->mDiffuse = DirectX::XMFLOAT4(data.m_Light.m_Diffuse[0], data.m_Light.m_Diffuse[1], data.m_Light.m_Diffuse[2], data.m_Light.m_Diffuse[3]);
        m_D3DContext->Unmap(m_lightConstantBuffer, 0);
    }

    // Clear the back buffer to the clear color
    m_D3DContext->ClearRenderTargetView(m_D3DRenderTargetView, g_clearColor.data());
    m_D3DContext->ClearDepthStencilView(m_depthBufferView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    m_D3DContext->RSSetViewports(1, &m_viewport);
    m_D3DContext->RSSetState(m_rasterizerState);
    m_D3DContext->OMSetDepthStencilState(m_depthStencilState, 0);

    m_D3DContext->OMSetRenderTargets(1, &m_D3DRenderTargetView, m_depthBufferView);

    // m_cube.Render(m_D3DContext, m_shader, m_mvpConstantBuffer);
    // m_plane.Render(m_D3DContext, m_shader, m_mvpConstantBuffer);

    m_grid.Render(m_D3DContext, m_shader, m_mvpConstantBuffer);

    m_light.Render(m_D3DContext, m_lightGeometryShader, m_mvpConstantBuffer, m_lightConstantBuffer);
    m_sphere.Render(m_D3DContext, m_lightGeometryShader, m_mvpConstantBuffer, m_lightConstantBuffer);

    data.m_matrix01 =
            DirectX::XMMatrixRotationY(degreesToRadians(data.m_cubeRotation1[1])) *
            DirectX::XMMatrixRotationX(degreesToRadians(data.m_cubeRotation1[0])) *
            DirectX::XMMatrixRotationZ(degreesToRadians(data.m_cubeRotation1[2])) *
            DirectX::XMMatrixTranslation(data.m_cubePosition1[0], data.m_cubePosition1[1], data.m_cubePosition1[2]);

    data.m_matrix02 =
        DirectX::XMMatrixRotationY(degreesToRadians(data.m_cubeRotation2[1])) *
        DirectX::XMMatrixRotationX(degreesToRadians(data.m_cubeRotation2[0])) *
        DirectX::XMMatrixRotationZ(degreesToRadians(data.m_cubeRotation2[2])) *
        DirectX::XMMatrixTranslation(data.m_cubePosition2[0], data.m_cubePosition2[1], data.m_cubePosition2[2]);

    data.m_matrix03 =
        DirectX::XMMatrixRotationY(degreesToRadians(data.m_texturedMeshRotation[1])) *
        DirectX::XMMatrixRotationX(degreesToRadians(data.m_texturedMeshRotation[0])) *
        DirectX::XMMatrixRotationZ(degreesToRadians(data.m_texturedMeshRotation[2])) *
        DirectX::XMMatrixTranslation(data.m_texturedMeshPosition[0], data.m_texturedMeshPosition[1], data.m_texturedMeshPosition[2]);

    {
        DirectX::XMMATRIX mvp = data.m_matrix02 * m_VP;
        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        m_D3DContext->Map(m_mvpConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
        auto* constants = (MatrixConstantBuffer*)(mappedSubresource.pData);
        constants->mWorld = DirectX::XMMatrixTranspose(data.m_matrix02);
        constants->mModelViewProjection = DirectX::XMMatrixTranspose(mvp);
        m_D3DContext->Unmap(m_mvpConstantBuffer, 0);
    }

    if (data.m_showTransform02)
        m_gizmoXYZ01.Render(m_D3DContext, m_simpleLit, m_mvpConstantBuffer, m_lightConstantBuffer);

    {
        auto mWorld = data.m_matrix01 * data.m_matrix02;
        DirectX::XMMATRIX mvp = mWorld * m_VP;
        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        m_D3DContext->Map(m_mvpConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
        auto* constants = (MatrixConstantBuffer*)(mappedSubresource.pData);
        constants->mWorld = DirectX::XMMatrixTranspose(mWorld);
        constants->mModelViewProjection = DirectX::XMMatrixTranspose(mvp);
        m_D3DContext->Unmap(m_mvpConstantBuffer, 0);
    }

    if (data.m_showTransform01)
        m_gizmoXYZ02.Render(m_D3DContext, m_simpleLit, m_mvpConstantBuffer, m_lightConstantBuffer);

    {
        auto mWorld = data.m_matrix03;
        DirectX::XMMATRIX mvp = mWorld * m_VP;
        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        m_D3DContext->Map(m_mvpConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
        auto* constants = (MatrixConstantBuffer*)(mappedSubresource.pData);
        constants->mWorld = DirectX::XMMatrixTranspose(mWorld);
        constants->mModelViewProjection = DirectX::XMMatrixTranspose(mvp);
        m_D3DContext->Unmap(m_mvpConstantBuffer, 0);
    }
    m_texturedMesh.Render(m_D3DContext, m_texturedShader, m_mvpConstantBuffer, m_lightConstantBuffer);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Present the back buffer to the screen
    m_SwapChain->Present(1, 0);

    m_D3DContext->ClearState();
    m_D3DContext->Flush();
}

/// @brief Create the Render Target view from the backbuffer
/// @param device D3D11 Device
/// @param renderTargetView Reference to the D3D11 Render Target View
/// @return S_OK if successful
HRESULT GraphicsDX11::CreateRenderTargetView()
{
    PLOG_INFO << "Creating the RenderTarget View";

    auto backBuffer = GetBackBuffer(m_SwapChain);

    // Create render target view
    auto hr = m_D3DDevice->CreateRenderTargetView(backBuffer, nullptr, &m_D3DRenderTargetView);
    if (FAILED(hr))
    {
        PLOG_ERROR << "Failed to create the Render Target view. Aborting";
        return hr;
    }

    D3D11_TEXTURE2D_DESC depthBufferDesc;
    backBuffer->GetDesc(&depthBufferDesc);

    backBuffer->Release();

    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* depthBuffer;
    if (!SUCCEEDED(m_D3DDevice->CreateTexture2D(&depthBufferDesc, nullptr, &depthBuffer)))
    {
        PLOG_ERROR << "Cannot Create the depth buffer";
        return S_FALSE;
    }
    if (depthBuffer == nullptr)
    {
        PLOG_ERROR << "Depth Buffer created, but it's null. This should not happen!";
        return S_FALSE;
    }

    m_D3DDevice->CreateDepthStencilView(depthBuffer, nullptr, &m_depthBufferView);

    depthBuffer->Release();

    return S_OK;
}

void GraphicsDX11::Cleanup()
{
    PLOG_INFO << "Cleaning up the resources for the Graphics DX11 class";

    // Release all our resources
    m_simpleLit.Cleanup();
    m_shader.Cleanup();
    m_lightGeometryShader.Cleanup();
    m_texturedShader.Cleanup();
    m_cube.Cleanup();
    m_grid.Cleanup();
    m_plane.Cleanup();
    m_gizmoXYZ01.Cleanup();
    m_gizmoXYZ02.Cleanup();
    m_texturedMesh.Cleanup();
    m_light.Cleanup();
    m_sphere.Cleanup();

    m_mvpConstantBuffer->Release();
    m_lightConstantBuffer->Release();
    m_depthBufferView->Release();
    m_depthStencilState->Release();
    m_rasterizerState->Release();

    m_SwapChain->Release();
    m_D3DRenderTargetView->Release();
    m_D3DContext->Release();
    m_D3DDevice->Release();

 #ifdef _DEBUG
    // Check to see if we've cleaned up all the D3D 11 resources.
    {
        IDXGIDebug1* debug = { 0 };
        DXGIGetDebugInterface1(0, IID_IDXGIDebug1, (void**)(&debug));
        debug->EnableLeakTrackingForThread();
        debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
        debug->Release();
    }
#endif // DEBUG
}