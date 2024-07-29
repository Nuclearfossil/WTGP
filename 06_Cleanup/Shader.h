#pragma once

#include <d3d11.h>
#include <string>

class Shader
{
public:
    Shader() = default;
    ~Shader();

    HRESULT Compile(ID3D11Device* pD3D11Device,const std::wstring& filename);

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
