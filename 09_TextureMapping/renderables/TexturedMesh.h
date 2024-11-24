#pragma once
#include <d3d11.h>
#include "shader.h"


class TexturedMesh
{
public:
    TexturedMesh() = default;
    ~TexturedMesh()
    {
        Cleanup();
    }

    HRESULT Initialize(ID3D11Device* pD3D11Device, std::string path);
    void Render(ID3D11DeviceContext* pD3D11DeviceContext, Shader& shader, ID3D11Buffer* mvpConstants);
    void Cleanup();

private:
    bool LoadFromFile(ID3D11DeviceContext* pD3D11DeviceContext, std::string path);
};
