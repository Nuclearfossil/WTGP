#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <directxmath.h>

#include <filesystem>


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

using namespace Assimp;

Mesh::~Mesh()
{
    Cleanup();
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
}

void Mesh::Render(ID3D11DeviceContext* pD3D11DeviceContext, Shader& shader, ID3D11Buffer* mvp, ID3D11Buffer* light) const
{
    for (auto* renderable : mRenderables)
        {
            renderable->Render(pD3D11DeviceContext, shader, mvp, light);
        }
}
