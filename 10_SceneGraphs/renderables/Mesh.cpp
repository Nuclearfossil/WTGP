#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <directxmath.h>

#include <filesystem>

#include "ConstantBuffers.h"
#include "Mesh.h"
#include <d3d11.h>
#include <cstdint>
#include <string>
#include <vector>
#include <Renderable.h>
#include <Shader.h>
#include <assimp\material.h>
#include <assimp\types.h>
#include <plog\Log.h>
#include "utils.h"

using namespace Assimp;

Mesh::~Mesh()
{
    Cleanup();
}

HRESULT Mesh::Initialize(ID3D11Device* pD3D11Device, ID3D11Buffer* lightConstantBufferPtr)
{
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

bool Mesh::LoadFromFile(ID3D11DeviceContext* pD3D11DeviceContext, std::string path)
{
    Importer importer;
    importer.SetExtraVerbose(true);

    std::filesystem::path filepath = std::filesystem::current_path() / path;

    PLOG_INFO << "Loading mesh from file: " << filepath;

    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more postprocessing than we do in this example.
    const aiScene* scene = importer.ReadFile(filepath.generic_string(),
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType);

    // If the import failed, report it
    if (nullptr == scene)
    {
        PLOG_ERROR << importer.GetErrorString();
        return false;
    }

    // construct the buffer to contain the vertices and the index buffer from the scene
    std::vector<ColorVertexNormal> vertexBuffer;
    std::vector<uint16_t> indexbuffer;
    std::vector<DirectX::XMFLOAT3> materialbuffer;

    if (scene->HasMaterials())
    {
        auto materialCount = scene->mNumMaterials;
        for (size_t materialIndex = 0; materialIndex < materialCount; materialIndex++)
        {
            auto material = scene->mMaterials[materialIndex];
            aiColor3D diffuse;
            material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
            materialbuffer.push_back(DirectX::XMFLOAT3{ diffuse.r, diffuse.g, diffuse.b });
        }
    }

    if (scene->HasMeshes())
    {
        int offsetVertex = 0;
        for (size_t meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
        {
            auto mesh = scene->mMeshes[meshIndex];
            auto vertexcount = mesh->mNumVertices;
            for (size_t vertexIndex = 0; vertexIndex < vertexcount; vertexIndex++)
            {
                auto vertex = mesh->mVertices[vertexIndex];
                auto material = materialbuffer[mesh->mMaterialIndex];
                auto normal = mesh->mNormals[vertexIndex];
                vertexBuffer.push_back(ColorVertexNormal
                {
                    vertex.x,
                    vertex.y,
                    vertex.z,
                    material.x,
                    material.y,
                    material.z,
                    1.0f,
                    normal.x,
                    normal.y,
                    normal.z
                });
            }

            for (size_t faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
            {
                auto const& face = mesh->mFaces[faceIndex];
                if (face.mNumIndices != 3)
                {
                    PLOG_WARNING << "Skipping as the face is not triangulated. " << face.mNumIndices << "indices per face.";
                    continue;
                }
                indexbuffer.push_back(face.mIndices[0] + offsetVertex);
                indexbuffer.push_back(face.mIndices[1] + offsetVertex);
                indexbuffer.push_back(face.mIndices[2] + offsetVertex);
            }

            if (meshIndex < scene->mNumMeshes - 1)
                offsetVertex += scene->mMeshes[meshIndex + 1]->mNumVertices;
        }

        auto renderable = new Renderable();

        // NB: Whenever you access a D3D resouce, like so, you need to release it when you're done with it.
        ID3D11Device* pD3D11Device;
        pD3D11DeviceContext->GetDevice(&pD3D11Device);
        renderable->Initialize(vertexBuffer, indexbuffer, pD3D11Device);

        mRenderables.push_back(renderable);

        // for the reader, what happens when you comment out this line?
        pD3D11Device->Release();

        return false;
    }
    return true;
}

void Mesh::Cleanup()
{
    PLOG_INFO << "Clearing Mesh";

    for (auto p : mRenderables)
        delete p;

    mRenderables.clear();

    SafeRelease(m_worldConstantBuffer);
    SafeRelease(lightConstantBuffer);

    m_worldConstantBuffer = nullptr;
    lightConstantBuffer = nullptr;
}

void Mesh::Draw(ID3D11DeviceContext* pD3DContext, std::shared_ptr<Shader> shader, DirectX::XMMATRIX world)
{
    Render(pD3DContext, shader, world);
}

void Mesh::Render(ID3D11DeviceContext* pD3D11DeviceContext, std::shared_ptr<Shader> shader, DirectX::XMMATRIX world) const
{
    {
        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        pD3D11DeviceContext->Map(m_worldConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
        LocalToWorldConstantBuffer* constants = (LocalToWorldConstantBuffer*)(mappedSubresource.pData);
        constants->mLocalToWorld = world;
        pD3D11DeviceContext->Unmap(m_worldConstantBuffer, 0);
    }

    for (auto* renderable : mRenderables)
    {
        renderable->Render(pD3D11DeviceContext, shader, m_worldConstantBuffer, lightConstantBuffer);
    }
}
