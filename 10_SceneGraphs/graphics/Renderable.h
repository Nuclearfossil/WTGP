#pragma once

#include <vector>
#include <memory>
#include <d3d11_4.h>
#include <DirectXMath.h>

#include "Shader.h"

struct [[nodiscard]] ColorVertexNormal
{
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
    float a;
    float nx;
    float ny;
    float nz;
};

struct [[nodiscard]] ColorVertexNormalUV
{
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
    float a;
    float nx;
    float ny;
    float nz;
    float u;
    float v;
};

class Renderable
{
public:
    Renderable() = default;
    ~Renderable();

    void Initialize(std::vector<ColorVertexNormal> vertexBuffer, std::vector<uint16_t> indexbuffer, ID3D11Device* pD3D11Device);
    void Initialize(std::vector<ColorVertexNormalUV> vertexBuffer, std::vector<uint16_t> indexbuffer, ID3D11Device* pD3D11Device);
    void Render(ID3D11DeviceContext* pD3D11DeviceContext, std::shared_ptr<Shader> shader, ID3D11Buffer* lightConstants);

    void Cleanup();

private:
    ID3D11Buffer* m_vertexBuffer = nullptr; // The D3D11 Buffer used to hold the vertex data for the grid
    ID3D11Buffer* m_indexBuffer = nullptr;  // The D3D11 Index Buffer for the grid

    UINT m_stride = 0;
    UINT m_offset = 0;
    uint16_t m_numIndices = 0;
};
