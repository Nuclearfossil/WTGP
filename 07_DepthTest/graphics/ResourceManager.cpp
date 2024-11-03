#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include <directxmath.h>

#include <filesystem>
#include <iostream>

#include "ResourceManager.h"
#include "Renderable.h"
#include "framework.h"

using namespace Assimp;

HRESULT InitResources(ID3D11DeviceContext* pD3D11DeviceContext)
{
    return S_OK;
}
