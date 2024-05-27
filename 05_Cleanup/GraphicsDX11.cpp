#include "GraphicsDX11.h"
#include "imgui_impl_dx11.h"

// Debug names for some of the D3D11 resources we'll be creating
#ifdef _DEBUG
constexpr char c_vertexShaderID[] = "vertexShader";
constexpr char c_pixelShaderID[] = "pixelShader";
constexpr char c_inputLayoutID[] = "inputLayout";
constexpr char c_vertexBufferID[] = "vertexBuffer";
constexpr char c_gridVertexBufferID[] = "gridVertexBuffer";
constexpr char c_gridIndexBufferID[] = "gridIndexBuffer";
constexpr char c_constantBufferID[] = "constantBuffer";
constexpr char c_indexBufferID[] = "indexBuffer";
constexpr char c_depthStencilBufferID[] = "depthStencilBuffer";
constexpr char c_rasterizerStateID[] = "rasterizerState";
#endif // DEBUG

GraphicsDX11::GraphicsDX11()
{

}


/// @brief Utility function for getting the Texture that represents the backbuffer
/// @param swapChain DXGI Swapchain to work from
/// @return a D3D11 Texture 2D to work with
ID3D11Texture2D* GraphicsDX11::GetBackBuffer(IDXGISwapChain* swapChain)
{
    ID3D11Texture2D* backBuffer = nullptr;
    HRESULT hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
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
    UINT height) {
    HRESULT hr = S_OK;

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
    hr = D3D11CreateDeviceAndSwapChain(
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

    if (FAILED(hr)) {
        // TODO: Add logging as to why this failed.
        return hr;
    }

    return S_OK;
}

/// @brief Create the D3D11 Context
/// @param device D3D11 Device to use for creating the D3D11 Context from
/// @param context Reference to the context to populate
/// @return
HRESULT GraphicsDX11::CreateD3D11Context(ID3D11Device* device, ID3D11DeviceContext** context) {
    HRESULT hr = S_OK;

    if (device == nullptr || context == nullptr)
        return E_INVALIDARG;

    // Create the D3D11 context
    hr = device->CreateDeferredContext(0, context);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

/// @brief Load the Shader from file and compile the Vertex and Pixel shaders
/// @return S_OK if we were able to compile the shaders
HRESULT GraphicsDX11::LoadAndCompileShaders() {
    // creation of Shader Resources
    ID3DBlob* vsBlob;
    ID3DBlob* psBlob;
    ID3DBlob* shaderCompileErrorBlob;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows
    // the shaders to be optimized and to run exactly the way they will run in
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;

    // Disable optimizations to further improve shader debugging
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    // We compile the Vertex shader from the `vertexShaderSource` source string and check for validity
    if (!SUCCEEDED(D3DCompileFromFile(L"CombinedShader.hlsl", // Shader File
            nullptr,                                          // Optional array of D3D_SHADER_MARCO defining macros used in compilation
            nullptr,                                          // optional pointer to an ID3DInclude that defines how the compiler handles include files
            "vs_main",                                        // Entry-point of the shader
            "vs_5_0",                                         // String that specifies what the shader target is.
            dwShaderFlags,                                    // Any flags that drive D3D compile constants. Things like `D3DCOMPILE_DEBUG`
            0,                                                // Any flags for compiler effect constants. For now we can ignore
            &vsBlob,                                          // An interface to the compiled shader
            &shaderCompileErrorBlob)))                        // An interface to any errors from the compile process.
    {
        OutputDebugStringA(static_cast<const char*>(shaderCompileErrorBlob->GetBufferPointer()));
        shaderCompileErrorBlob->Release();
        return S_FALSE;
    }

    // We then create the appropriate Vertex Shader resource: `m_vertexShader`.
    if (!SUCCEEDED(m_D3DDevice->CreateVertexShader(
            vsBlob->GetBufferPointer(), // A pointer to the compiled shader.
            vsBlob->GetBufferSize(),    // And the size of the compiled shader.
            nullptr,                    // A pointer to the Class Linkage (for now, let's use null).
            &m_vertexShader)))          // Address of the ID3D11VertexShader interface.
    {
        OutputDebugStringA("Failed to create the Vertex Shader!\n");
        return S_FALSE;
    }

#ifdef _DEBUG
    m_vertexShader->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_vertexShaderID) - 1, c_vertexShaderID);
#endif // DEBUG


    // We compile the Pixel shader from the `pixelShaderSource` source string and check for validity
    if (!SUCCEEDED(D3DCompileFromFile(L"CombinedShader.hlsl",
            nullptr,
            nullptr,
            "ps_main",
            "ps_5_0",
            0,
            0,
            &psBlob,
            &shaderCompileErrorBlob))) {
        OutputDebugStringA(static_cast<const char*>(shaderCompileErrorBlob->GetBufferPointer()));
        shaderCompileErrorBlob->Release();
        return S_FALSE;
    }

    // We then create the appropriate Pixel Shader resource: `m_pixelShader`
    if (!SUCCEEDED(m_D3DDevice->CreatePixelShader(
            psBlob->GetBufferPointer(),
            psBlob->GetBufferSize(),
            nullptr,
            &m_pixelShader))) {
        OutputDebugStringA("Failed to create the Pixel Shader\n");
        return S_FALSE;
    }

    psBlob->Release();

#ifdef _DEBUG
    m_pixelShader->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_pixelShaderID) - 1, c_pixelShaderID);
#endif // DEBUG

    // Create Input Layout - this describes the format of the vertex data we will use.
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
        { "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    if (!SUCCEEDED(m_D3DDevice->CreateInputLayout(
            inputElementDesc,            // An array of D3D11_INPUT_ELEMENT_DESC describing the vertex data
            ARRAYSIZE(inputElementDesc), // How big is the array
            vsBlob->GetBufferPointer(),  // The compiled vertex shader
            vsBlob->GetBufferSize(),     // And the size of the vertex shader
            &m_inputLayout)))            // The resultant input layout
    {
        OutputDebugStringA("Failed to create the Input Layout");
        return S_FALSE;
    }

    vsBlob->Release();

#ifdef _DEBUG
    m_inputLayout->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_inputLayoutID) - 1, c_inputLayoutID);
#endif // DEBUG
    return S_OK;
}

/// @brief Create the Vertex and Index buffers, as well as the input layout
/// @return S_OK if we are able to create the buffers and input layout
HRESULT GraphicsDX11::CreateVertexAndIndexBuffers() {
    // Populate the array representing the vertex data. In this case, we are going to have
    // 6 elements per vertex:
    //   - X and Y co-ordinates
    //   - Colours for each vertex representing the Red, Green, Blue and Alpha channels.
    float vertexData[] = {
        //    x,     y,     z,    r,    g,    b,    a
        -0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        -0.5f,
        -0.5f,
        0.5f,
        0.0f,
        1.0f,
        0.0f,
        1.0f,
        -0.5f,
        0.5f,
        -0.5f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        -0.5f,
        0.5f,
        0.5f,
        0.0f,
        1.0f,
        0.0f,
        1.0f,
        0.5f,
        -0.5f,
        -0.5f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
        0.5f,
        -0.5f,
        0.5f,
        0.0f,
        1.0f,
        0.0f,
        1.0f,
        0.5f,
        0.5f,
        -0.5f,
        1.0f,
        0.0f,
        0.0f,
        1.0f,
        0.5f,
        0.5f,
        0.5f,
        0.0f,
        0.0f,
        1.0f,
        1.0f,
    };

    float gridVertexData[]{
        //      x,    y,     z,    r,    g,    b,    a
        // columns
        -5.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -5.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -4.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -4.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -3.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -3.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -2.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -2.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        2.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        2.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        3.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        3.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        4.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        4.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        5.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        5.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        // rows
        -5.0f, 0.0f, -5.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        5.0f, 0.0f, -5.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -5.0f, 0.0f, -4.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        5.0f, 0.0f, -4.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -5.0f, 0.0f, -3.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        5.0f, 0.0f, -3.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -5.0f, 0.0f, -2.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        5.0f, 0.0f, -2.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -5.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        5.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -5.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        5.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -5.0f, 0.0f, 2.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        5.0f, 0.0f, 2.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -5.0f, 0.0f, 3.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        5.0f, 0.0f, 3.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -5.0f, 0.0f, 4.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        5.0f, 0.0f, 4.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -5.0f, 0.0f, 5.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        5.0f, 0.0f, 5.0f, 1.0f, 0.0f, 0.0f, 1.0f
    };

    WORD gridIndices[] = {
        0, 1,
        2, 3,
        4, 5,
        6, 7,
        8, 9,
        10, 11,
        12, 13,
        14, 15,
        16, 17,
        18, 19,
        20, 21,
        22, 23,
        24, 25,
        26, 27,
        28, 29,
        30, 31,
        32, 33,
        34, 35,
        36, 37,
        38, 39,
        40, 41,
        42, 43
    };

    // Create index buffer
    WORD indices[] = {
        0, 6, 4,
        0, 2, 6,

        0, 3, 2,
        0, 1, 3,

        2, 7, 6,
        2, 3, 7,

        4, 6, 7,
        4, 7, 5,

        0, 4, 5,
        0, 5, 1,

        1, 5, 7,
        1, 7, 3
    };

    // The stride represents the _actual_ width of the data for each vertex.
    // In this case, we now have 7 elements per vertex. Three for the position, four for the colour.
    g_stride = 3 * sizeof(float) + 4 * sizeof(float);
    g_offset = 0;
    g_numCubeVerts = sizeof(indices) / sizeof(WORD);

    g_numCubeIndices = sizeof(indices) / sizeof(indices[0]);

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = sizeof(vertexData);
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexSubresourceData;
    vertexSubresourceData.pSysMem = vertexData;
    vertexSubresourceData.SysMemPitch = 0;
    vertexSubresourceData.SysMemSlicePitch = 0;

    if (!SUCCEEDED(m_D3DDevice->CreateBuffer(
            &vertexBufferDesc,      // The Vertex buffer description
            &vertexSubresourceData, // And then the sub-resource data
            &m_cubeVertexBuffer)))  // Finally, the interface that is the vertex buffer
    {
        OutputDebugStringA("Failed to create the vertex buffer!");
        return S_FALSE;
    }

#ifdef _DEBUG
    m_cubeVertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_vertexBufferID) - 1, c_vertexBufferID);
#endif // DEBUG

    g_numGridVerts = sizeof(gridIndices) / sizeof(WORD);
    g_numGridIndices = sizeof(gridIndices) / sizeof(gridIndices[0]);

    D3D11_BUFFER_DESC gridVertexBufferDesc = {};
    gridVertexBufferDesc.ByteWidth = sizeof(gridVertexData);
    gridVertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    gridVertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA gridVertexSubresourceData = {};
    gridVertexSubresourceData.pSysMem = gridVertexData;
    gridVertexSubresourceData.SysMemPitch = 0;
    gridVertexSubresourceData.SysMemSlicePitch = 0;

    if (!SUCCEEDED(m_D3DDevice->CreateBuffer(
            &gridVertexBufferDesc,
            &gridVertexSubresourceData,
            &m_gridVertexBuffer))) {
        OutputDebugStringA("Failed to create the Grid vertex buffer!");
        return S_FALSE;
    }

#ifdef _DEBUG
    m_gridVertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_gridVertexBufferID) - 1, c_gridVertexBufferID);
#endif // _DEBUG


    D3D11_BUFFER_DESC constantBufferDesc = {};
    constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (FAILED(m_D3DDevice->CreateBuffer(&constantBufferDesc, nullptr, &m_mvpConstantBuffer))) {
        OutputDebugStringA("Failed to create a new constant buffer.");
        return S_FALSE;
    }

#ifdef _DEBUG
    m_mvpConstantBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_constantBufferID) - 1, c_constantBufferID);
#endif // DEBUG


    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = sizeof(indices);
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = indices;
    initData.SysMemPitch = 0;
    initData.SysMemSlicePitch = 0;

    if (FAILED(m_D3DDevice->CreateBuffer(&indexBufferDesc, &initData, &m_cubeIndexBuffer))) {
        OutputDebugStringA("Failed to create a new index buffer.");
        return S_FALSE;
    }

#ifdef _DEBUG
    m_cubeIndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_indexBufferID) - 1, c_indexBufferID);
#endif // DEBUG

    D3D11_BUFFER_DESC gridIndexBufferDesc = {};
    gridIndexBufferDesc.ByteWidth = sizeof(gridIndices);
    gridIndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    gridIndexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    gridIndexBufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA gridInitData = {};
    gridInitData.pSysMem = gridIndices;
    gridInitData.SysMemPitch = 0;
    gridInitData.SysMemSlicePitch = 0;

    if (FAILED(m_D3DDevice->CreateBuffer(&gridIndexBufferDesc, &gridInitData, &m_gridIndexBuffer))) {
        OutputDebugStringA("Failed to create a new grid index buffer");
        return S_FALSE;
    }

#ifdef _DEBUG
    m_gridIndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_gridIndexBufferID) - 1, c_gridIndexBufferID);
#endif

    return S_OK;
}

/// @brief Create the depth/stencil buffer as well as the rasterizer state objects
/// @return S_OK if we are able to create these objects
HRESULT GraphicsDX11::CreateDepthStencilAndRasterizerState() {
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

    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    // rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;

    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    // rasterizerDesc.CullMode = D3D11_CULL_FRONT;
    // rasterizerDesc.CullMode = D3D11_CULL_NONE;

    m_D3DDevice->CreateRasterizerState(&rasterizerDesc, &m_rasterizerState);

#ifdef _DEBUG
    m_rasterizerState->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_rasterizerStateID) - 1, c_rasterizerStateID);
#endif // DEBUG

    return S_OK;
}

/// @brief Create all D3D11 resources we will need for this application
/// @return S_OK if successful
HRESULT GraphicsDX11::CreateD3DResources() {
    if (!SUCCEEDED(LoadAndCompileShaders()))
        return S_FALSE;

    if (!SUCCEEDED(CreateVertexAndIndexBuffers()))
        return S_FALSE;

    if (!SUCCEEDED(CreateDepthStencilAndRasterizerState()))
        return S_FALSE;

    return S_OK;
}

/// @brief Render off a frame
/// @param hWnd Handle to the window
/// @param winRect RECT that defines the window to render to
void GraphicsDX11::Render(HWND hWnd, RECT winRect, double increment)
{
    // Update constant buffer
    {
        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        m_D3DContext->Map(m_mvpConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
        ConstantBuffer* constants = (ConstantBuffer*)(mappedSubresource.pData);
        constants->mModelViewProjection = DirectX::XMMatrixTranspose(m_MVP);
        m_D3DContext->Unmap(m_mvpConstantBuffer, 0);
    }
    // Clear the back buffer to the clear color
    m_D3DContext->ClearRenderTargetView(m_D3DRenderTargetView, g_clearColor);
    m_D3DContext->ClearDepthStencilView(m_depthBufferView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    m_D3DContext->RSSetViewports(1, &m_viewport);
    m_D3DContext->RSSetState(m_rasterizerState);
    m_D3DContext->OMSetDepthStencilState(m_depthStencilState, 0);

    m_D3DContext->OMSetRenderTargets(1, &m_D3DRenderTargetView, nullptr);

    m_D3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    m_D3DContext->IASetInputLayout(m_inputLayout);

    m_D3DContext->VSSetShader(m_vertexShader, nullptr, 0);
    m_D3DContext->PSSetShader(m_pixelShader, nullptr, 0);
    m_D3DContext->VSSetConstantBuffers(0, 1, &m_mvpConstantBuffer);

    m_D3DContext->IASetVertexBuffers(0, 1, &m_gridVertexBuffer, &g_stride, &g_offset);
    m_D3DContext->IASetIndexBuffer(m_gridIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    m_D3DContext->DrawIndexed(g_numGridIndices, 0, 0);

    auto worldMat = DirectX::XMMatrixRotationY((float)increment) * DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
    {
        DirectX::XMMATRIX mvp = worldMat * m_VP;
        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        m_D3DContext->Map(m_mvpConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
        ConstantBuffer* constants = (ConstantBuffer*)(mappedSubresource.pData);
        constants->mModelViewProjection = DirectX::XMMatrixTranspose(mvp);
        m_D3DContext->Unmap(m_mvpConstantBuffer, 0);
    }

    m_D3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_D3DContext->IASetInputLayout(m_inputLayout);

    m_D3DContext->VSSetShader(m_vertexShader, nullptr, 0);
    m_D3DContext->PSSetShader(m_pixelShader, nullptr, 0);
    m_D3DContext->VSSetConstantBuffers(0, 1, &m_mvpConstantBuffer);

    m_D3DContext->IASetVertexBuffers(0, 1, &m_cubeVertexBuffer, &g_stride, &g_offset);
    m_D3DContext->IASetIndexBuffer(m_cubeIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    m_D3DContext->DrawIndexed(g_numCubeIndices, 0, 0);

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
    HRESULT hr = S_OK;

    auto backBuffer = GetBackBuffer(m_SwapChain);

    // Create render target view
    hr = m_D3DDevice->CreateRenderTargetView(backBuffer, NULL, &m_D3DRenderTargetView);
    if (FAILED(hr)) {
        OutputDebugStringA("Failed to create the Render Target view. Aborting\n");
        return hr;
    }

    D3D11_TEXTURE2D_DESC depthBufferDesc;
    backBuffer->GetDesc(&depthBufferDesc);

    backBuffer->Release();

    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    ID3D11Texture2D* depthBuffer;
    if (!SUCCEEDED(m_D3DDevice->CreateTexture2D(&depthBufferDesc, nullptr, &depthBuffer)))
        return S_FALSE;
    if (depthBuffer == nullptr)
        return S_FALSE;

    m_D3DDevice->CreateDepthStencilView(depthBuffer, nullptr, &m_depthBufferView);

    depthBuffer->Release();

    return S_OK;
}

void GraphicsDX11::Cleanup()
{
    // Release all our resources
    m_vertexShader->Release();
    m_pixelShader->Release();
    m_inputLayout->Release();
    m_cubeVertexBuffer->Release();
    m_cubeIndexBuffer->Release();
    m_gridVertexBuffer->Release();
    m_gridIndexBuffer->Release();
    m_mvpConstantBuffer->Release();
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
        debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
        debug->Release();
    }
#endif // DEBUG
}