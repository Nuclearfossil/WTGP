#pragma once
#include <d3d11.h>

#include "RenderPrimitive.h"
#include "shader.h"

class Cube : public RenderPrimitive
{
public:
    Cube() = default;
    ~Cube();

    HRESULT Initialize(ID3D11Device* pD3D11Device);
    void Render(ID3D11DeviceContext* pD3D11DeviceContext, std::shared_ptr<Shader> shader, DirectX::XMMATRIX world);
    void Cleanup() override;

    void Draw(ID3D11DeviceContext* pD3DContext, std::shared_ptr<Shader> shader, DirectX::XMMATRIX world) override;

private:
    ID3D11Buffer* m_cubeVertexBuffer = nullptr; // The D3D11 Buffer used to hold the vertex data for the cube.
    ID3D11Buffer* m_cubeIndexBuffer = nullptr;  // The D3D11 Index Buffer for the cube
    ID3D11Buffer* m_worldConstantBuffer = nullptr;  // The D3D11 Constant buffer used for World Transforms
};
