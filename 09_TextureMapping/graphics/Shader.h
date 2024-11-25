#pragma once

#include <d3d11.h>
#include <vector>
#include <string>

enum IALayouts
{
    IALayout_VertexColor = 0,
    IALayout_VertexColorNormal,
    IALayout_VertexColorNormalUV
};

const std::vector<std::vector<D3D11_INPUT_ELEMENT_DESC>> m_IALayouts = {
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    },
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    },
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "UV", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    }
};

class InputLayouts
{
public:
    InputLayouts() = default;

    static std::vector<D3D11_INPUT_ELEMENT_DESC> GetInputLayout(IALayouts element)
    {
        return m_IALayouts[element];
    }
};

class Shader
{
public:
    Shader() = default;
    ~Shader();

    HRESULT Compile(ID3D11Device* pD3D11Device, const std::wstring& filename, IALayouts layout);

    void Cleanup();

    ID3D11InputLayout* GetLayout()
    {
        return m_inputLayout;
    }

    ID3D11VertexShader* GetVertexShader()
    {
        return m_vertexShader;
    }

    ID3D11PixelShader* GetPixelShader()
    {
        return m_pixelShader;
    }

private:
    ID3D11VertexShader* m_vertexShader = nullptr; // The Vertex Shader resource used in this example
    ID3D11PixelShader* m_pixelShader = nullptr;   // The Pixel Shader resource used in this example
    ID3D11InputLayout* m_inputLayout = nullptr;   // The Input layout resource used for the vertex shader
};
