#pragma once
#include <string>
#include <vector>

#include "RenderBase.h"
#include "Renderable.h"

class Mesh : public RenderBase
{
public:
    Mesh() = default;
    ~Mesh();

    HRESULT Initialize(ID3D11Device* pD3D11Device, ID3D11Buffer* lightConstantBufferPtr);
    bool LoadFromFile(ID3D11DeviceContext* pD3D11DeviceContext, std::string path);
    void Cleanup() override;

    void Render(ID3D11DeviceContext* pD3D11DeviceContext, std::shared_ptr<Shader> shader, DirectX::XMMATRIX world) const;

    virtual void Draw(ID3D11DeviceContext* pD3DContext, std::shared_ptr<Shader> shader, DirectX::XMMATRIX world) override;

private:
    std::vector<Renderable*> mRenderables;

    ID3D11Buffer* m_worldConstantBuffer = nullptr;  // The D3D11 Constant buffer used for World Transforms
    ID3D11Buffer* lightConstantBuffer = nullptr;         // the D3D11 Constant buffer used for Light information
};
