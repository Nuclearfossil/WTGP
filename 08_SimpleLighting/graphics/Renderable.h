#pragma once

#include <vector>
#include <d3d11_4.h>
#include <DirectXMath.h>

#include "Shader.h"

struct [[nodiscard]] ColorVertex
{
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
    float a;
};

class Renderable
{
public:
    Renderable() = default;
    ~Renderable();

    void Initialize(std::vector<ColorVertex> vertexBuffer, std::vector<uint16_t> indexbuffer, ID3D11Device* pD3D11Device);
    void Render(ID3D11DeviceContext* pD3D11DeviceContext, Shader& shader, ID3D11Buffer* mvpConstants);

    void Cleanup();

private:
    ID3D11Buffer* m_vertexBuffer = nullptr; // The D3D11 Buffer used to hold the vertex data for the grid
    ID3D11Buffer* m_indexBuffer = nullptr;  // The D3D11 Index Buffer for the grid

    UINT m_stride = 0;
    UINT m_offset = 0;
    uint16_t m_numIndices = 0;
};
