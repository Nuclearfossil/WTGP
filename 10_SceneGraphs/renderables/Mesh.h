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

    bool LoadFromFile(ID3D11DeviceContext* pD3D11DeviceContext, std::string path);
    void Cleanup() override;

    void Render(ID3D11DeviceContext* pD3D11DeviceContext, std::shared_ptr<Shader> shader, ID3D11Buffer* light) const;

    virtual void Draw(ID3D11DeviceContext* pD3DContext, std::shared_ptr<Shader> shader, DirectX::XMMATRIX world) override;

private:
    std::vector<Renderable*> mRenderables;
};
