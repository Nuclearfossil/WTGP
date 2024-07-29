#pragma once
#include <d3d11.h>

#include "shader.h"

class Grid
{
public:
    Grid() = default;
    ~Grid();

    HRESULT Initialize(ID3D11Device* pD3D11Device);
    void Render(ID3D11DeviceContext* pD3D11DeviceContext, Shader& shader, ID3D11Buffer* mvpConstants);
    void Cleanup();

private:
    ID3D11Buffer* m_gridVertexBuffer = nullptr; // The D3D11 Buffer used to hold the vertex data for the grid
    ID3D11Buffer* m_gridIndexBuffer = nullptr;  // The D3D11 Index Buffer for the grid

    UINT m_stride = 0;
    UINT m_offset = 0;
    uint16_t m_numGridIndices = 0;
};
