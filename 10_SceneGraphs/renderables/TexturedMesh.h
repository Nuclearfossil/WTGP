#pragma once
#include <d3d11.h>

#include "RenderBase.h"
#include "Renderable.h"
#include "Material.h"
#include "shader.h"


class TexturedMesh : public RenderBase
{
public:
    TexturedMesh() = default;
    ~TexturedMesh()
    {
        Cleanup();
    }

    HRESULT Initialize(ID3D11Device* pD3D11Device, ID3D11Buffer* lightConstantBufferPtr);

    bool LoadFromFile(ID3D11DeviceContext* pD3D11DeviceContext, std::string path);
    void Render(ID3D11DeviceContext* pD3D11DeviceContext, std::shared_ptr<Shader> shader, DirectX::XMMATRIX world);
    void Cleanup() override;

    void Draw(ID3D11DeviceContext* pD3DContext, std::shared_ptr<Shader> shader, DirectX::XMMATRIX world) override;

private:
    std::vector<Renderable*> mRenderables;

    Material m_Material;
    ID3D11Buffer* m_worldConstantBuffer = nullptr;  // The D3D11 Constant buffer used for World Transforms
    ID3D11Buffer* lightConstantBuffer = nullptr;         // the D3D11 Constant buffer used for Light information
};
