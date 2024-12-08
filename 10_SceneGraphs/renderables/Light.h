#pragma once
#include <d3d11.h>

#include "RenderPrimitive.h"
#include <Shader.h>

class Light : public RenderPrimitive
{
public:
    Light() = default;
    ~Light();

    HRESULT Initialize(ID3D11Device* pD3D11Device);
    void Render(ID3D11DeviceContext* pD3D11DeviceContext, Shader& shader, ID3D11Buffer* mvpConstants, ID3D11Buffer* lightConstants);
    void Cleanup() override;

private:
    ID3D11Buffer* m_vertices = nullptr;
    ID3D11Buffer* m_indices = nullptr;
};
