#pragma once

#include <d3d11_4.h>
#include "framework.h"
#include "Shader.h"

HRESULT InitResources(ID3D11DeviceContext* pD3D11DeviceContext);

HRESULT RenderResources(ID3D11DeviceContext* pD3D11DeviceContext, Shader& shader, ID3D11Buffer* mvp);

void ClearResources();