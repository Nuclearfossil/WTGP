#include "Renderable.h"
#include "utils.h"
#include "plog/Log.h"

#ifdef _DEBUG
constexpr char c_vertexBufferID[] = "Renderable-vertexBuffer";
constexpr char c_indexBufferID[] = "Renderable-indexBuffer";
#endif

Renderable::~Renderable()
{
    Cleanup();
}


void Renderable::Initialize(
    std::vector<ColorVertexNormal> vertexBuffer,
    std::vector<uint16_t> indexbuffer,
    ID3D11Device* pD3D11Device)
{
    m_numIndices = static_cast<uint16_t>(indexbuffer.size());

    m_stride = 3 * sizeof(float) + 4 * sizeof(float) + 3 * sizeof(float);
    m_offset = 0;

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = static_cast<UINT>(vertexBuffer.size() * m_stride);
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexSubresourceData = {};
    vertexSubresourceData.pSysMem = vertexBuffer.data();
    vertexSubresourceData.SysMemPitch = 0;
    vertexSubresourceData.SysMemSlicePitch = 0;

    if (!SUCCEEDED(pD3D11Device->CreateBuffer(
        &vertexBufferDesc,
        &vertexSubresourceData,
        &m_vertexBuffer)))
    {
        PLOG_ERROR << "Failed to create the Vertex Buffer for a Renderable!";
        return;
    }

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = static_cast<UINT>(m_numIndices * sizeof(WORD));
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA indexbufferData = {};
    indexbufferData.pSysMem = indexbuffer.data();
    indexbufferData.SysMemPitch = 0;
    indexbufferData.SysMemSlicePitch = 0;

    if (!SUCCEEDED(pD3D11Device->CreateBuffer(
        &indexBufferDesc,
        &indexbufferData,
        &m_indexBuffer)))
    {
        PLOG_ERROR << "Failed to create a new grid index buffer";
        return ;
    }
}


void Renderable::Initialize(
    std::vector<ColorVertexNormalUV> vertexBuffer,
    std::vector<uint16_t> indexbuffer,
    ID3D11Device* pD3D11Device)
{
    m_numIndices = static_cast<uint16_t>(indexbuffer.size());

    m_stride = 3 * sizeof(float) + 4 * sizeof(float) + 3 * sizeof(float) + 2 * sizeof(float);
    m_offset = 0;

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = static_cast<UINT>(vertexBuffer.size() * m_stride);
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexSubresourceData = {};
    vertexSubresourceData.pSysMem = vertexBuffer.data();
    vertexSubresourceData.SysMemPitch = 0;
    vertexSubresourceData.SysMemSlicePitch = 0;

    if (!SUCCEEDED(pD3D11Device->CreateBuffer(
            &vertexBufferDesc,
            &vertexSubresourceData,
            &m_vertexBuffer)))
        {
            PLOG_ERROR << "Failed to create the Vertex Buffer for a Renderable!";
            return;
        }

#ifdef _DEBUG
    m_vertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_vertexBufferID) - 1, c_vertexBufferID);
#endif // DEBUG


    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = static_cast<UINT>(m_numIndices * sizeof(WORD));
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA indexbufferData = {};
    indexbufferData.pSysMem = indexbuffer.data();
    indexbufferData.SysMemPitch = 0;
    indexbufferData.SysMemSlicePitch = 0;

    if (!SUCCEEDED(pD3D11Device->CreateBuffer(
            &indexBufferDesc,
            &indexbufferData,
            &m_indexBuffer)))
        {
            PLOG_ERROR << "Failed to create a new grid index buffer";
            return;
        }

#ifdef _DEBUG
    m_indexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_indexBufferID) - 1, c_indexBufferID);
#endif // DEBUG
}


void Renderable::Render(ID3D11DeviceContext* pD3D11DeviceContext, Shader& shader, ID3D11Buffer* mvpConstants, ID3D11Buffer* lightConstants)
{
    pD3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pD3D11DeviceContext->IASetInputLayout(shader.GetLayout());

    pD3D11DeviceContext->VSSetShader(shader.GetVertexShader(), nullptr, 0);
    pD3D11DeviceContext->PSSetShader(shader.GetPixelShader(), nullptr, 0);
    pD3D11DeviceContext->VSSetConstantBuffers(0, 1, &mvpConstants);
    pD3D11DeviceContext->PSSetConstantBuffers(0, 1, &lightConstants);

    pD3D11DeviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &m_stride, &m_offset);
    pD3D11DeviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);

    pD3D11DeviceContext->DrawIndexed(m_numIndices, 0, 0);
}

void Renderable::Cleanup()
{
    PLOG_INFO << "Renderable Destructor";

    SafeRelease(m_vertexBuffer);
    SafeRelease(m_indexBuffer);

    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;
}
