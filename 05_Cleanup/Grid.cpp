#include <vector>
#include "Grid.h"

#include "framework.h"
#include "utils.h"

#ifdef _DEBUG
constexpr char c_gridVertexBufferID[] = "gridVertexBuffer";
constexpr char c_gridIndexBufferID[] = "gridIndexBuffer";
#endif

Grid::~Grid()
{
    PLOG_INFO << "Destroying the Grid";
    Cleanup();
}

HRESULT Grid::Initialize(ID3D11Device * pD3D11Device)
{
    PLOG_INFO << "Initializing the Grid";

    std::vector<float> gridVertexData =
    {
        //      x,    y,     z,    r,    g,    b,    a
        // columns
        -5.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -5.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -4.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -4.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -3.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -3.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -2.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -2.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        2.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        2.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        3.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        3.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        4.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        4.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        5.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        5.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        // rows
        -5.0f, 0.0f, -5.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        5.0f, 0.0f, -5.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -5.0f, 0.0f, -4.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        5.0f, 0.0f, -4.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -5.0f, 0.0f, -3.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        5.0f, 0.0f, -3.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -5.0f, 0.0f, -2.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        5.0f, 0.0f, -2.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -5.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        5.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -5.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        5.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -5.0f, 0.0f, 2.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        5.0f, 0.0f, 2.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -5.0f, 0.0f, 3.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        5.0f, 0.0f, 3.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -5.0f, 0.0f, 4.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        5.0f, 0.0f, 4.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -5.0f, 0.0f, 5.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        5.0f, 0.0f, 5.0f, 1.0f, 0.0f, 0.0f, 1.0f
    };

    std::vector<WORD> gridIndices =
    {
        0, 1,
        2, 3,
        4, 5,
        6, 7,
        8, 9,
        10, 11,
        12, 13,
        14, 15,
        16, 17,
        18, 19,
        20, 21,
        22, 23,
        24, 25,
        26, 27,
        28, 29,
        30, 31,
        32, 33,
        34, 35,
        36, 37,
        38, 39,
        40, 41,
        42, 43
    };

    // The stride represents the _actual_ width of the data for each vertex.
    // In this case, we now have 7 elements per vertex. Three for the position, four for the colour.
    m_stride = 3 * sizeof(float) + 4 * sizeof(float);
    m_offset = 0;

    m_numGridIndices = static_cast<uint16_t>(gridIndices.size());

    D3D11_BUFFER_DESC gridVertexBufferDesc = {};
    gridVertexBufferDesc.ByteWidth = static_cast<UINT>(gridVertexData.size() * sizeof(float));
    gridVertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    gridVertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA gridVertexSubresourceData = {};
    gridVertexSubresourceData.pSysMem = gridVertexData.data();
    gridVertexSubresourceData.SysMemPitch = 0;
    gridVertexSubresourceData.SysMemSlicePitch = 0;

    if (!SUCCEEDED(pD3D11Device->CreateBuffer(
            &gridVertexBufferDesc,
            &gridVertexSubresourceData,
            &m_gridVertexBuffer)))
        {
            PLOG_ERROR << "Failed to create the Grid vertex buffer!";
            return S_FALSE;
        }

#ifdef _DEBUG
    m_gridVertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_gridVertexBufferID) - 1, c_gridVertexBufferID);
#endif // _DEBUG

    D3D11_BUFFER_DESC gridIndexBufferDesc = {};
    gridIndexBufferDesc.ByteWidth = static_cast<UINT>(gridIndices.size() * sizeof(WORD));
    gridIndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    gridIndexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    gridIndexBufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA gridInitData = {};
    gridInitData.pSysMem = gridIndices.data();
    gridInitData.SysMemPitch = 0;
    gridInitData.SysMemSlicePitch = 0;

    if (FAILED(pD3D11Device->CreateBuffer(&gridIndexBufferDesc, &gridInitData, &m_gridIndexBuffer)))
    {
        PLOG_ERROR << "Failed to create a new grid index buffer";
        return S_FALSE;
    }

#ifdef _DEBUG
    m_gridIndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_gridIndexBufferID) - 1, c_gridIndexBufferID);
#endif

    return S_OK;
}

void Grid::Render(ID3D11DeviceContext* pD3D11DeviceContext, Shader& shader, ID3D11Buffer* mvpConstants)
{
    pD3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    pD3D11DeviceContext->IASetInputLayout(shader.GetLayout());

    pD3D11DeviceContext->VSSetShader(shader.GetVertexShader(), nullptr, 0);
    pD3D11DeviceContext->PSSetShader(shader.GetPixelShader(), nullptr, 0);
    pD3D11DeviceContext->VSSetConstantBuffers(0, 1, &mvpConstants);

    pD3D11DeviceContext->IASetVertexBuffers(0, 1, &m_gridVertexBuffer, &m_stride, &m_offset);
    pD3D11DeviceContext->IASetIndexBuffer(m_gridIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    pD3D11DeviceContext->DrawIndexed(m_numGridIndices, 0, 0);
}

void Grid::Cleanup()
{
    PLOG_INFO << "Calling Cleanup on the grid";

    SafeRelease(m_gridVertexBuffer);
    SafeRelease(m_gridIndexBuffer);

    m_gridVertexBuffer = nullptr;
    m_gridIndexBuffer = nullptr;
}
