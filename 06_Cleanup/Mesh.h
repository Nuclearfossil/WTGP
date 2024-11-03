#pragma once
#include <string>
#include <vector>

#include "Renderable.h"

class Mesh
{
public:
    Mesh() = default;
    ~Mesh();

    bool LoadFromFile(ID3D11DeviceContext* pD3D11DeviceContext, std::string);
    void ClearResources();

    void Render(ID3D11DeviceContext* pD3D11DeviceContext, Shader& shader, ID3D11Buffer* mvp) const;

private:
    std::vector<Renderable*> mRenderables;
};
