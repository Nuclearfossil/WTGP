#include "Material.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "utils.h"
#include "framework.h"

Material::~Material()
{
    Cleanup();
}

bool Material::LoadImageFromFile(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const std::string filepath)
{
    int imgWidth;
    int imgHeight;
    int imgChannels;
    unsigned char* data = stbi_load(filepath.c_str(), &imgWidth, &imgHeight, &imgChannels, STBI_rgb_alpha);

    D3D11_TEXTURE2D_DESC texture_desc = {};
    texture_desc.Width = imgWidth;
    texture_desc.Height = imgHeight;
    texture_desc.MipLevels = 1;
    texture_desc.ArraySize = 1;
    texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texture_desc.SampleDesc.Count = 1;
    texture_desc.Usage = D3D11_USAGE_DEFAULT;
    texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA subresource_data = {};
    subresource_data.pSysMem = data;
    subresource_data.SysMemPitch = imgWidth * 4;

    HRESULT result = pDevice->CreateTexture2D(&texture_desc, &subresource_data, &m_pTexture);
    if (FAILED(result))
    {
        PLOG_ERROR << L"Failed to create texture 2d";
        stbi_image_free(data);
        return false;
    }

    ID3D11ShaderResourceView* shader_resource_view = nullptr;
    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
    srv_desc.Format = texture_desc.Format;
    srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = 1;

    result = pDevice->CreateShaderResourceView(m_pTexture, &srv_desc, &shader_resource_view);
    if (FAILED(result))
    {
        PLOG_ERROR << "Failed to create shader resource view";
        m_pTexture->Release();
        stbi_image_free(data);
        return false;
    }

    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    pDevice->CreateSamplerState(&sampDesc, &m_pSamplerState);

    pDevice->Release();
    stbi_image_free(data);
    return true;
}

void Material::UseMaterial(ID3D11DeviceContext* pDeviceContext)
{
    pDeviceContext->PSSetSamplers(0, 1, &m_pSamplerState);
}

void Material::Cleanup()
{
    PLOG_INFO << "MaterialCleanup Destructor";

    SafeRelease(m_pSamplerState);
    SafeRelease(m_pTexture);

    m_pTexture = nullptr;
    m_pSamplerState = nullptr;
}