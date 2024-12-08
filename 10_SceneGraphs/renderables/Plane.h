#pragma once

#include <d3d11.h>

#include "RenderPrimitive.h"
#include "shader.h"

class Plane : public RenderPrimitive
{
public:
    Plane() = default;
    ~Plane();

    HRESULT Initialize(ID3D11Device* pD3D11Device);
    void Render(ID3D11DeviceContext* pD3D11DeviceContext, Shader& shader, ID3D11Buffer* mvpConstants);
    void Cleanup() override;

private:
    ID3D11Buffer* m_VertexBuffer = nullptr;
    ID3D11Buffer* m_IndexBuffer = nullptr;
};