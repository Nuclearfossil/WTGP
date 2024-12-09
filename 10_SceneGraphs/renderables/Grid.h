#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

#include "RenderPrimitive.h"
#include "shader.h"

class Grid : public RenderPrimitive
{
public:
    Grid() = default;
    ~Grid();

    HRESULT Initialize(ID3D11Device* pD3D11Device);
    void Render(ID3D11DeviceContext* pD3D11DeviceContext, std::shared_ptr<Shader> shader, DirectX::XMMATRIX world);
    void Cleanup() override;

    virtual void Draw(ID3D11DeviceContext* pD3DContext, std::shared_ptr<Shader> shader, DirectX::XMMATRIX world) override;

private:
    ID3D11Buffer* m_gridVertexBuffer = nullptr;     // The D3D11 Buffer used to hold the vertex data for the grid
    ID3D11Buffer* m_gridIndexBuffer = nullptr;      // The D3D11 Index Buffer for the grid
    ID3D11Buffer* m_worldConstantBuffer = nullptr;  // The D3D11 Constant buffer used for World Transforms
};
