#pragma once

#include <d3d11.h>
#include "shader.h"

class Plane
{
public:
    Plane() = default;
    ~Plane();

    HRESULT Initialize(ID3D11Device* pD3D11Device);
    void Render(ID3D11DeviceContext* pD3D11DeviceContext, Shader& shader, ID3D11Buffer* mvpConstants);
    void Cleanup();

private:
    ID3D11Buffer* m_VertexBuffer = nullptr;
    ID3D11Buffer* m_IndexBuffer = nullptr;

    UINT m_stride = 0;
    UINT m_offset = 0;
    UINT m_numIndices = 0;
};