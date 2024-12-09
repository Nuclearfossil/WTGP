#include <vector>

#include "framework.h"
#include "ConstantBuffers.h"
#include "Cube.h"

// Debug names for some of the D3D11 resources we'll be creating
#ifdef _DEBUG
constexpr char c_vertexBufferID[] = "vertexBuffer";
constexpr char c_indexBufferID[] = "indexBuffer";
#endif

Cube::~Cube()
{
    Cleanup();
}

HRESULT Cube::Initialize(ID3D11Device* pD3D11Device)
{
    // Populate the array representing the vertex data. In this case, we are going to have
    // 6 elements per vertex:
    //   - X and Y co-ordinates
    //   - Colours for each vertex representing the Red, Green, Blue and Alpha channels.
    std::vector<float> vertexData =
    {
    //      x,     y,     z,     r,     g,     b,     a
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f,  1.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,
    };

       // Create index buffer
    std::vector<WORD> indices =
    {
        0, 6, 4,
        0, 2, 6,

        0, 3, 2,
        0, 1, 3,

        2, 7, 6,
        2, 3, 7,

        4, 6, 7,
        4, 7, 5,

        0, 4, 5,
        0, 5, 1,

        1, 5, 7,
        1, 7, 3
    };

    // The stride represents the _actual_ width of the data for each vertex.
    // In this case, we now have 7 elements per vertex. Three for the position, four for the colour.
    stride = 3 * sizeof(float) + 4 * sizeof(float);
    offset = 0;

    numIndices = static_cast<UINT>(indices.size());

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = static_cast<UINT>(vertexData.size() * sizeof(float));
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexSubresourceData = {};
    vertexSubresourceData.pSysMem = vertexData.data();
    vertexSubresourceData.SysMemPitch = 0;
    vertexSubresourceData.SysMemSlicePitch = 0;

    if (!SUCCEEDED(pD3D11Device->CreateBuffer(
            &vertexBufferDesc,      // The Vertex buffer description
            &vertexSubresourceData, // And then the sub-resource data
            &m_cubeVertexBuffer)))  // Finally, the interface that is the vertex buffer
        {
            PLOG_ERROR << "Failed to create the vertex buffer!";
            return S_FALSE;
        }

#ifdef _DEBUG
    m_cubeVertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_vertexBufferID) - 1, c_vertexBufferID);
#endif // DEBUG

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = static_cast<UINT>(indices.size() * sizeof(WORD));
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = indices.data();
    initData.SysMemPitch = 0;
    initData.SysMemSlicePitch = 0;

    if (FAILED(pD3D11Device->CreateBuffer(&indexBufferDesc, &initData, &m_cubeIndexBuffer)))
        {
            OutputDebugStringA("Failed to create a new index buffer.");
            return S_FALSE;
        }

#ifdef _DEBUG
    m_cubeIndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_indexBufferID) - 1, c_indexBufferID);
#endif // DEBUG

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

    return S_OK;
}

void Cube::Draw(ID3D11DeviceContext* pD3DContext,std::shared_ptr<Shader> shader,DirectX::XMMATRIX world)
{
    Render(pD3DContext, shader, world);
}

void Cube::Render(ID3D11DeviceContext* pD3D11Context, std::shared_ptr<Shader> shader, DirectX::XMMATRIX world)
{
    {
        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        pD3D11Context->Map(m_worldConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
        LocalToWorldConstantBuffer* constants = (LocalToWorldConstantBuffer*)(mappedSubresource.pData);
        constants->mLocalToWorld = world;
        pD3D11Context->Unmap(m_worldConstantBuffer, 0);
    }

    pD3D11Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    pD3D11Context->IASetInputLayout(shader->GetLayout());

    pD3D11Context->VSSetShader(shader->GetVertexShader(), nullptr, 0);
    pD3D11Context->PSSetShader(shader->GetPixelShader(), nullptr, 0);
    pD3D11Context->VSSetConstantBuffers(1, 1, &m_worldConstantBuffer);

    pD3D11Context->IASetVertexBuffers(0, 1, &m_cubeVertexBuffer, &stride, &offset);
    pD3D11Context->IASetIndexBuffer(m_cubeIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    pD3D11Context->DrawIndexed(numIndices, 0, 0);
}

void Cube::Cleanup()
{
    if (m_cubeVertexBuffer != nullptr)
    {
        m_cubeVertexBuffer->Release();
        m_cubeVertexBuffer = nullptr;
    }

    if (m_cubeIndexBuffer != nullptr)
    {
        m_cubeIndexBuffer->Release();
        m_cubeIndexBuffer = nullptr;
    }

    if (m_worldConstantBuffer != nullptr)
    {
        m_worldConstantBuffer->Release();
        m_worldConstantBuffer = nullptr;
    }
}