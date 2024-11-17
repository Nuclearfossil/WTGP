#include <vector>

#include "Light.h"
#include "framework.h"
#include "utils.h"

#ifdef _DEBUG
constexpr char c_vertexBufferID[] = "lightVertexBuffer";
constexpr char c_indexBufferID[] = "lightIndexBuffer";
#endif

Light::~Light()
{
    Cleanup();
}

HRESULT Light::Initialize(ID3D11Device* pD3D11Device)
{
    std::vector<float> vertexData =
    {
    //    X,    Y,    Z,    R,    G,    B,    A
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        0.25f, 0.0f, 0.0f, 0.25f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.25f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.25f, 0.0f, 0.0f, 1.0f, 1.0f,
    };

    std::vector<WORD> indices = {
        0, 3,
        1, 4,
        2, 5
    };

    // The stride represents the _actual_ width of the data for each vertex.
    // In this case, we now have 7 elements per vertex. Three for the position, four for the colour.
    m_stride = 3 * sizeof(float) + 4 * sizeof(float);
    m_offset = 0;

    m_numVertexIndices = static_cast<UINT>(indices.size());

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = static_cast<UINT>(vertexData.size() * sizeof(float));
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexSubresourceData = {};
    vertexSubresourceData.pSysMem = vertexData.data();
    vertexSubresourceData.SysMemPitch = 0;
    vertexSubresourceData.SysMemSlicePitch = 0;

    if (!SUCCEEDED(pD3D11Device->CreateBuffer(
        &vertexBufferDesc,
        &vertexSubresourceData,
        &m_vertices)))
    {
        PLOG_ERROR << "Failed to create Vertex Buffer for Light representation";
        return S_FALSE;
    }

#ifdef _DEBUG
    m_vertices->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_vertexBufferID) - 1, c_vertexBufferID);
#endif

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = static_cast<UINT>(indices.size() * sizeof(WORD));
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA indexInitData = {};
    indexInitData.pSysMem = indices.data();
    indexInitData.SysMemPitch = 0;
    indexInitData.SysMemSlicePitch = 0;

    if (!SUCCEEDED(pD3D11Device->CreateBuffer(
            &indexBufferDesc,
            &indexInitData,
            &m_indices)))
        {
            PLOG_ERROR << "Failed to create Index Buffer for Light representation";
            return S_FALSE;
        }

#ifdef _DEBUG
    m_vertices->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_indexBufferID) - 1, c_indexBufferID);
#endif


    return S_OK;
}

void Light::Render(ID3D11DeviceContext* pD3D11DeviceContext, Shader& shader, ID3D11Buffer* mvpConstants, ID3D11Buffer* lightConstants)
{
    pD3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    pD3D11DeviceContext->IASetInputLayout(shader.GetLayout());

    pD3D11DeviceContext->VSSetShader(shader.GetVertexShader(), nullptr, 0);
    pD3D11DeviceContext->PSSetShader(shader.GetPixelShader(), nullptr, 0);
    pD3D11DeviceContext->VSSetConstantBuffers(0, 1, &mvpConstants);
    pD3D11DeviceContext->VSSetConstantBuffers(1, 1, &lightConstants);

    pD3D11DeviceContext->IASetVertexBuffers(0, 1, &m_vertices, &m_stride, &m_offset);
    pD3D11DeviceContext->IASetIndexBuffer(m_indices, DXGI_FORMAT_R16_UINT, 0);

    pD3D11DeviceContext->DrawIndexed(m_numVertexIndices, 0, 0);
}

void Light::Cleanup()
{
    PLOG_INFO << "Calling Cleanup on the light";

    SafeRelease(m_vertices);
    SafeRelease(m_indices);

    m_vertices = nullptr;
    m_indices = nullptr;
}
