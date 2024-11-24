#include "TexturedMesh.h"

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <directxmath.h>

#include <filesystem>

#include "ResourceManager.h"
#include "Material.h"
#include "framework.h"

#include "Mesh.h"

using namespace Assimp;

HRESULT TexturedMesh::Initialize(ID3D11Device* pD3D11Device, std::string path)
{
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
        return S_FALSE;
    }

    std::vector<ColorVertexNormalUV> vertexBuffer;
    std::vector<uint16_t> indexBuffer;
    std::vector<Material> materials;

    if (scene->HasMaterials())
    {
        auto materialCount = scene->mNumMaterials;
        for (size_t materialIndex = 0; materialIndex < materialCount; materialIndex++)
        {
            auto material = scene->mMaterials[materialIndex];
            aiColor3D diffuse;
            material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
            if (material->GetTextureCount(aiTextureType_DIFFUSE) == 0)
            {
                PLOG_ERROR << "Unable to find any textures associated with the material!";
                return S_FALSE;
            }

            aiString texturePath;
            auto textureID = material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);

            Material newMaterial;
            newMaterial.diffuse[0] = diffuse.r;
            newMaterial.diffuse[1] = diffuse.g;
            newMaterial.diffuse[2] = diffuse.b;
        }
    }

    return true;
}

void TexturedMesh::Render(ID3D11DeviceContext* pD3D11DeviceContext, Shader& shader, ID3D11Buffer* mvpConstants)
{

}

void TexturedMesh::Cleanup()
{

}


bool TexturedMesh::LoadFromFile(ID3D11DeviceContext* pD3D11DeviceContext, std::string path)
{
    return false;
}