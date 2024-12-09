#pragma once

#include <memory>
#include <Shader.h>
#include <DirectXMath.h>

class RenderBase
{
public:
    RenderBase();
    ~RenderBase();

    virtual void Draw(ID3D11DeviceContext* pD3DContext, std::shared_ptr<Shader> shader, DirectX::XMMATRIX world) = 0;

    virtual void Cleanup() {};

protected:
    ID3D11Buffer* worldConstantBuffer = nullptr;
};