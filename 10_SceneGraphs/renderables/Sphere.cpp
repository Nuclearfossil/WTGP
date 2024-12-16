#include "ConstantBuffers.h"
#include "Sphere.h"
#include "framework.h"
#include "utils.h"

#include <DirectXMath.h>

#ifdef _DEBUG
constexpr char c_vertexBufferID[] = "sphereVertexBuffer";
constexpr char c_indexBufferID[] = "sphereIndexBuffer";
#endif

struct Vertex
{
    float x;
    float y;
    float z;
    float r;
    float g;
    float b;
    float a;
};

HRESULT Sphere::Initialize(ID3D11Device* pD3D11Device, ID3D11Buffer* lightConstantBufferPtr, float radius, int sliceCount, int stackCount)
{
    std::vector<Vertex> vertices;
    std::vector<WORD> indices;

    // Generate vertices
    for (int i = 0; i <= stackCount; ++i)
    {
        float phi = DirectX::XM_PI * i / stackCount;
        for (int j = 0; j <= sliceCount; ++j)
        {
            float theta = DirectX::XM_2PI * j / sliceCount;

            Vertex vertex;
            vertex.x = radius * sinf(phi) * cosf(theta);
            vertex.y = radius * cosf(phi);
            vertex.z = radius * sinf(phi) * sinf(theta);
            vertex.r = 1.0f;
            vertex.g = 1.0f;
            vertex.b = 1.0f;
            vertex.a = 1.0f;

            vertices.push_back(vertex);
        }
    }

    // Generate indices for wireframe
    for (int i = 0; i < stackCount; ++i)
    {
        for (int j = 0; j < sliceCount; ++j)
        {
            // Connect the vertices in a grid to form lines
            // Horizontal lines
            indices.push_back(i * (sliceCount + 1) + j);       // Current point
            indices.push_back(i * (sliceCount + 1) + (j + 1)); // Next point in the same row

            // Vertical lines
            indices.push_back(i * (sliceCount + 1) + j);       // Current point
            indices.push_back((i + 1) * (sliceCount + 1) + j); // Point directly below
        }
    }

    // Connect last row to first row for horizontal lines
    for (int j = 0; j < sliceCount; ++j)
    {
        indices.push_back(stackCount * (sliceCount + 1) + j);       // Last row point
        indices.push_back(stackCount * (sliceCount + 1) + (j + 1)); // Connect to next point in last row
    }

    // The stride represents the _actual_ width of the data for each vertex.
    // In this case, we now have 7 elements per vertex. Three for the position, four for the colour.
    m_stride = 3 * sizeof(float) + 4 * sizeof(float);
    m_offset = 0;

    m_numVertexIndices = static_cast<UINT>(indices.size());

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = static_cast<UINT>(vertices.size() * sizeof(float) * 7);
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexSubresourceData = {};
    vertexSubresourceData.pSysMem = vertices.data();
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
    indexBufferDesc.ByteWidth = static_cast<UINT>(indices.size() * sizeof(DWORD));
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
    m_indices->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_indexBufferID) - 1, c_indexBufferID);
#endif

    D3D11_BUFFER_DESC localToWorldConstantBufferDesc = {};
    localToWorldConstantBufferDesc.ByteWidth = sizeof(LocalToWorldConstantBuffer);
    localToWorldConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    localToWorldConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    localToWorldConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (FAILED(pD3D11Device->CreateBuffer(&localToWorldConstantBufferDesc, nullptr, &m_worldConstantBuffer)))
    {
        PLOG_ERROR << "Failed to create a new constant buffer.";
        return S_FALSE;
    }

    lightConstantBuffer = lightConstantBufferPtr;
    lightConstantBuffer->AddRef();

    return S_OK;
}

void Sphere::Draw(ID3D11DeviceContext* pD3DContext, std::shared_ptr<Shader> shader, DirectX::XMMATRIX world)
{
    Render(pD3DContext, shader, world);
}

void Sphere::Render(ID3D11DeviceContext* pD3D11DeviceContext, std::shared_ptr<Shader> shader, DirectX::XMMATRIX world)
{
    {
        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        pD3D11DeviceContext->Map(m_worldConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
        LocalToWorldConstantBuffer* constants = (LocalToWorldConstantBuffer*)(mappedSubresource.pData);
        constants->mLocalToWorld = world;
        pD3D11DeviceContext->Unmap(m_worldConstantBuffer, 0);
    }
    pD3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    pD3D11DeviceContext->IASetInputLayout(shader->GetLayout());

    pD3D11DeviceContext->VSSetShader(shader->GetVertexShader(), nullptr, 0);
    pD3D11DeviceContext->PSSetShader(shader->GetPixelShader(), nullptr, 0);
    pD3D11DeviceContext->VSSetConstantBuffers(1, 1, &m_worldConstantBuffer);
    pD3D11DeviceContext->VSSetConstantBuffers(2, 1, &lightConstantBuffer);

    pD3D11DeviceContext->IASetVertexBuffers(0, 1, &m_vertices, &m_stride, &m_offset);
    pD3D11DeviceContext->IASetIndexBuffer(m_indices, DXGI_FORMAT_R16_UINT, 0);

    pD3D11DeviceContext->DrawIndexed(m_numVertexIndices, 0, 0);
}

void Sphere::Cleanup()
{
    PLOG_INFO << "Calling Cleanup on the Sphere";

    SafeRelease(m_vertices);
    SafeRelease(m_indices);
    SafeRelease(m_worldConstantBuffer);
    SafeRelease(lightConstantBuffer);

    m_vertices = nullptr;
    m_indices = nullptr;
    m_worldConstantBuffer = nullptr;
    lightConstantBuffer = nullptr;
}