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

    bool LoadFromFile(ID3D11DeviceContext* pD3D11DeviceContext, std::string path);
    void Render(ID3D11DeviceContext* pD3D11DeviceContext, std::shared_ptr<Shader> shader, ID3D11Buffer* light);
    void Cleanup() override;

    void Draw(ID3D11DeviceContext* pD3DContext, std::shared_ptr<Shader> shader, DirectX::XMMATRIX world) override;

private:
    std::vector<Renderable*> mRenderables;

    Material m_Material;
};
