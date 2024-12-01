#pragma once

#include <string>
#include <d3d11_4.h>

class Material
{
public:
    Material() = default;
    ~Material();

    bool LoadImageFromFile(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const std::string filepath);
    void UseMaterial(ID3D11DeviceContext* pDeviceContext);

    void Cleanup();

    float diffuse[3];

private:
    ID3D11Texture2D* m_pTexture = nullptr;
    ID3D11SamplerState* m_pSamplerState = nullptr;
    ID3D11ShaderResourceView* m_pShaderResourceView = nullptr;
};
