#pragma once

#include "DirectXMath.h"

/// @brief Structure defining the Constant buffer. This buffer will be used to pass data into the shader
struct MatrixConstantBuffer
{
    DirectX::XMMATRIX mViewProjection;
};

struct LocalToWorldConstantBuffer
{
    DirectX::XMMATRIX mLocalToWorld;
};

struct LightConstantBuffer
{
    DirectX::XMFLOAT4 mLightPosition;
    DirectX::XMFLOAT4 mDiffuse;
};