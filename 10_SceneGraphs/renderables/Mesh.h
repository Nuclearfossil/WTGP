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

    void Render(ID3D11DeviceContext* pD3D11DeviceContext, Shader& shader, ID3D11Buffer* mvp, ID3D11Buffer* light) const;

private:
    std::vector<Renderable*> mRenderables;
};
