#pragma once
#include <d3d11.h>
#include <Shader.h>

class Light
{
public:
    Light() = default;
    ~Light();

    HRESULT Initialize(ID3D11Device* pD3D11Device);
    void Render(ID3D11DeviceContext* pD3D11DeviceContext, Shader& shader, ID3D11Buffer* mvpConstants, ID3D11Buffer* lightConstants);
    void Cleanup();

private:
    ID3D11Buffer* m_vertices = nullptr;
    ID3D11Buffer* m_indices = nullptr;

    UINT m_stride = 0;
    UINT m_offset = 0;
    UINT m_numVertexIndices = 0;
};
