#include "Shader.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dcommon.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>

#include <vector>

#include "framework.h"

#ifdef _DEBUG
constexpr char c_vertexShaderID[] = "vertexShader";
constexpr char c_pixelShaderID[] = "pixelShader";
constexpr char c_inputLayoutID[] = "inputLayout";
#endif // DEBUG

Shader::~Shader()
{
    PLOG_INFO << "Destroying the Shader";
    Cleanup();
}

HRESULT Shader::Compile(ID3D11Device* pD3D11Device, const std::wstring& filename)
{
    PLOG_INFO << "Compiling the shader" << filename;

    // can we load the file?
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
    if (!SUCCEEDED(D3DCompileFromFile(filename.c_str(),     // Shader File
            nullptr,                                        // Optional array of D3D_SHADER_MARCO defining macros used in compilation
            nullptr,                                        // optional pointer to an ID3DInclude that defines how the compiler handles include files
            "vs_main",                                      // Entry-point of the shader
            "vs_5_0",                                       // String that specifies what the shader target is.
            dwShaderFlags,                                  // Any flags that drive D3D compile constants. Things like `D3DCOMPILE_DEBUG`
            0,                                              // Any flags for compiler effect constants. For now we can ignore
            &vsBlob,                                        // An interface to the compiled shader
            &shaderCompileErrorBlob)))                      // An interface to any errors from the compile process.
        {
            PLOG_ERROR << static_cast<const char*>(shaderCompileErrorBlob->GetBufferPointer());
            shaderCompileErrorBlob->Release();
            return S_FALSE;
        }

    // We then create the appropriate Vertex Shader resource: `m_vertexShader`.
    if (!SUCCEEDED(pD3D11Device->CreateVertexShader(
            vsBlob->GetBufferPointer(), // A pointer to the compiled shader.
            vsBlob->GetBufferSize(),    // And the size of the compiled shader.
            nullptr,                    // A pointer to the Class Linkage (for now, let's use null).
            &m_vertexShader)))          // Address of the ID3D11VertexShader interface.
        {
            PLOG_ERROR << "Failed to create the Vertex Shader!";
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
            &shaderCompileErrorBlob)))
        {
            PLOG_ERROR << static_cast<const char*>(shaderCompileErrorBlob->GetBufferPointer());
            shaderCompileErrorBlob->Release();
            return S_FALSE;
        }

    // We then create the appropriate Pixel Shader resource: `m_pixelShader`
    if (!SUCCEEDED(pD3D11Device->CreatePixelShader(
            psBlob->GetBufferPointer(),
            psBlob->GetBufferSize(),
            nullptr,
            &m_pixelShader)))
        {
            PLOG_ERROR << "Failed to create the Pixel Shader";
            return S_FALSE;
        }

    psBlob->Release();

#ifdef _DEBUG
    m_pixelShader->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_pixelShaderID) - 1, c_pixelShaderID);
#endif // DEBUG

    // Create Input Layout - this describes the format of the vertex data we will use.
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDesc = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    if (!SUCCEEDED(pD3D11Device->CreateInputLayout(
            inputElementDesc.data(),                    // An array of D3D11_INPUT_ELEMENT_DESC describing the vertex data
            static_cast<UINT>(inputElementDesc.size()), // How big is the array
            vsBlob->GetBufferPointer(),                 // The compiled vertex shader
            vsBlob->GetBufferSize(),                    // And the size of the vertex shader
            &m_inputLayout)))                           // The resultant input layout
        {
            PLOG_ERROR << "Failed to create the Input Layout";
            return S_FALSE;
        }

    vsBlob->Release();

#ifdef _DEBUG
    m_inputLayout->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_inputLayoutID) - 1, c_inputLayoutID);
#endif // DEBUG
    return S_OK;
}

void Shader::Cleanup()
{
    PLOG_INFO << "Cleaning up the Shader";
    if (m_vertexShader != nullptr)
    {
        m_vertexShader->Release();
        m_vertexShader = nullptr;
    }

    if (m_pixelShader != nullptr)
    {
        m_pixelShader->Release();
        m_pixelShader = nullptr;
    }

    if (m_inputLayout != nullptr)
    {
        m_inputLayout->Release();
        m_inputLayout = nullptr;
    }
}