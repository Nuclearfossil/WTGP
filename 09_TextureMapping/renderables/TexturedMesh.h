#pragma once
#include <d3d11.h>

#include "Renderable.h"
#include "Material.h"
#include "shader.h"


class TexturedMesh
{
public:
    TexturedMesh() = default;
    ~TexturedMesh()
    {
        Cleanup();
    }

    bool LoadFromFile(ID3D11DeviceContext* pD3D11DeviceContext, std::string path);
    void Render(ID3D11DeviceContext* pD3D11DeviceContext, Shader& shader, ID3D11Buffer* mvpConstants, ID3D11Buffer* light);
    void Cleanup();

private:
    std::vector<Renderable*> mRenderables;

    Material m_Material;
};
