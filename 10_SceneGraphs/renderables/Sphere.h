#pragma once
#include <d3d11.h>

#include "RenderPrimitive.h"
#include <Shader.h>

class Sphere  : public RenderPrimitive
{
public:

    Sphere() = default;
    ~Sphere()
    {
        Cleanup();
    }

    HRESULT Initialize(ID3D11Device* pD3D11Device, ID3D11Buffer* lightConstantBufferPtr, float radius, int sliceCount, int stackCount);
    void Render(ID3D11DeviceContext* pD3D11DeviceContext, std::shared_ptr<Shader> shader, DirectX::XMMATRIX world);

    void Cleanup();

    void Draw(ID3D11DeviceContext* pD3DContext, std::shared_ptr<Shader> shader, DirectX::XMMATRIX world) override;

private:

    ID3D11Buffer* m_vertices = nullptr;
    ID3D11Buffer* m_indices = nullptr;
    ID3D11Buffer* m_worldConstantBuffer = nullptr;  // The D3D11 Constant buffer used for World Transforms
    ID3D11Buffer* lightConstantBuffer = nullptr;

    UINT m_stride = 0;
    UINT m_offset = 0;
    UINT m_numVertexIndices = 0;
};
