#pragma once
#include <d3d11.h>

#include "RenderPrimitive.h"
#include <Shader.h>

class Light : public RenderPrimitive
{
public:
    Light() = default;
    ~Light();

    HRESULT Initialize(ID3D11Device* pD3D11Device, ID3D11Buffer* lightConstantBufferPtr);
    void Render(ID3D11DeviceContext* pD3D11DeviceContext, std::shared_ptr<Shader> shader, DirectX::XMMATRIX world);
    void Cleanup() override;

    void Draw(ID3D11DeviceContext* pD3DContext, std::shared_ptr<Shader> shader, DirectX::XMMATRIX world) override;


private:
    ID3D11Buffer* m_vertices = nullptr;
    ID3D11Buffer* m_indices = nullptr;
    ID3D11Buffer* m_worldConstantBuffer = nullptr;  // The D3D11 Constant buffer used for World Transforms
    ID3D11Buffer* lightConstantBuffer = nullptr;         // the D3D11 Constant buffer used for Light information
};
