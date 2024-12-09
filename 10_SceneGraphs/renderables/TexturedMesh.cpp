#include "TexturedMesh.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <directxmath.h>

#include <filesystem>

#include "ResourceManager.h"
#include "Material.h"
#include "framework.h"

#include "ConstantBuffers.h"

#include "utils.h"
#include <assimp\types.h>
#include <d3d11.h>
#include <cstdint>
#include <string>
#include <vector>
#include <Renderable.h>
#include <Shader.h>
#include <assimp\material.h>
#include <plog\Log.h>

using namespace Assimp;

HRESULT TexturedMesh::Initialize(ID3D11Device* pD3D11Device, ID3D11Buffer* lightConstantBufferPtr)
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
bool TexturedMesh::LoadFromFile(ID3D11DeviceContext* pDeviceContext, std::string path)
{
    ID3D11Device* pDevice = nullptr;
    pDeviceContext->GetDevice(&pDevice);
    auto refCount = pDevice->AddRef();
    pDevice->Release();

    PLOG_INFO << "Refcount at the start in Textured Mesh of pDevice: " << refCount;

    Importer importer;
    importer.SetExtraVerbose(true);

    std::filesystem::path filepath = std::filesystem::current_path() / path;

    PLOG_INFO << "Loading Textured Mesh from file: " << filepath;

    const aiScene* scene = importer.ReadFile(filepath.generic_string(),
                                            aiProcess_Triangulate |
                                            aiProcess_JoinIdenticalVertices |
                                            aiProcess_SortByPType);

    if (nullptr == scene)
    {
        PLOG_ERROR << importer.GetErrorString();
        return false;
    }

    std::vector<ColorVertexNormalUV> vertexBuffer;
    std::vector<uint16_t> indexBuffer;
    std::vector<DirectX::XMFLOAT3> materialbuffer;

    if (scene->HasMaterials())
    {
        auto materialCount = scene->mNumMaterials;
        if (materialCount > 1)
        {
            PLOG_ERROR << "Currently only support one texture per material! Found " << materialCount;
            return false;
        }

        // Grab the diffuse values
        for (size_t materialDiffuseIndex = 0; materialDiffuseIndex < materialCount; materialDiffuseIndex++)
        {
            auto material = scene->mMaterials[materialDiffuseIndex];
            aiColor3D diffuse;
            material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
            materialbuffer.push_back(DirectX::XMFLOAT3{ diffuse.r, diffuse.g, diffuse.b });
        }

        // And now a texture, if one is associated
        for (size_t materialIndex = 0; materialIndex < materialCount; materialIndex++)
        {
            auto material = scene->mMaterials[materialIndex];
            aiString texturePath;
            auto foundTexture = material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);

            if (!m_Material.LoadImageFromFile(pDevice, pDeviceContext, texturePath.C_Str()))
            {
                PLOG_ERROR << "Failed to load texture from file: " << texturePath.C_Str();
                return false;
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
                    auto normal = mesh->mNormals[vertexIndex];
                    auto uv = mesh->mTextureCoords[0][vertexIndex];
                    auto materialRef = materialbuffer[mesh->mMaterialIndex];
                    vertexBuffer.push_back(ColorVertexNormalUV{
                        vertex.x,
                        vertex.y,
                        vertex.z,
                        materialRef.x,
                        materialRef.y,
                        materialRef.z,
                        1.0f,
                        normal.x,
                        normal.y,
                        normal.z,
                        uv.x,
                        uv.y });
                }

                for (size_t faceIndex = 0; faceIndex < mesh->mNumFaces; faceIndex++)
                {
                    auto const& face = mesh->mFaces[faceIndex];
                    if (face.mNumIndices != 3)
                    {
                        PLOG_WARNING << "Skipping as the face is not triangulated. " << face.mNumIndices << "indices per face.";
                        continue;
                    }

                    indexBuffer.push_back(face.mIndices[0] + offsetVertex);
                    indexBuffer.push_back(face.mIndices[1] + offsetVertex);
                    indexBuffer.push_back(face.mIndices[2] + offsetVertex);
                }

                if (meshIndex < scene->mNumMeshes - 1)
                    offsetVertex += scene->mMeshes[meshIndex + 1]->mNumVertices;
            }

            // NB: Whenever you access a D3D resouce, like so, you need to release it when you're done with it.
            auto renderable = new Renderable();
            renderable->Initialize(vertexBuffer, indexBuffer, pDevice);

            mRenderables.push_back(renderable);

            // for the reader, what happens when you comment out this line?
            // pDevice->Release();
        }
    }

    refCount = pDevice->AddRef();
    pDevice->Release();

    PLOG_INFO << "Refcount at the end in Textured Mesh of pDevice: " << refCount;

    return true;
}

void TexturedMesh::Draw(ID3D11DeviceContext* pD3DContext, std::shared_ptr<Shader> shader, DirectX::XMMATRIX world)
{
    Render(pD3DContext, shader, world);
}

void TexturedMesh::Render(ID3D11DeviceContext* pD3D11DeviceContext, std::shared_ptr<Shader> shader, DirectX::XMMATRIX world)
{
    {
        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        pD3D11DeviceContext->Map(m_worldConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
        LocalToWorldConstantBuffer* constants = (LocalToWorldConstantBuffer*)(mappedSubresource.pData);
        constants->mLocalToWorld = world;
        pD3D11DeviceContext->Unmap(m_worldConstantBuffer, 0);
    }

    m_Material.UseMaterial(pD3D11DeviceContext);
    for (auto* renderable : mRenderables)
    {
        renderable->Render(pD3D11DeviceContext, shader, m_worldConstantBuffer, lightConstantBuffer);
    }
}

void TexturedMesh::Cleanup()
{
    PLOG_INFO << "Clearing Mesh";

    for (auto p : mRenderables)
        delete p;

    mRenderables.clear();

    m_Material.Cleanup();

    SafeRelease(m_worldConstantBuffer);
    SafeRelease(lightConstantBuffer);

    m_worldConstantBuffer = nullptr;
    lightConstantBuffer = nullptr;

}
