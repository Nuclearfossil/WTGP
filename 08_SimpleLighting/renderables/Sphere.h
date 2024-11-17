#pragma once
#include <d3d11.h>
#include <Shader.h>

class Sphere
{
public:

    Sphere() = default;
    ~Sphere()
    {
        Cleanup();
    }

    HRESULT Initialize(ID3D11Device* pD3D11Device, float radius, int sliceCount, int stackCount);
    void Render(ID3D11DeviceContext* pD3D11DeviceContext, Shader& shader, ID3D11Buffer* mvpConstants, ID3D11Buffer* lightConstants);

    void Cleanup();

private:

    ID3D11Buffer* m_vertices;
    ID3D11Buffer* m_indices;

    UINT m_stride = 0;
    UINT m_offset = 0;
    UINT m_numVertexIndices = 0;
};
