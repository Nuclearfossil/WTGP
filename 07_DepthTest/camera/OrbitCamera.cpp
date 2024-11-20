#include "pch.h"

#include <DirectXMath.h>

#include "mathutils.h"
#include "OrbitCamera.h"

#include "framework.h"

void OrbitCamera::Initialize()
{
    PLOG_INFO << "Initializing the Orbit Camera";

    m_World = DirectX::XMMatrixIdentity();
    m_View = DirectX::XMMatrixIdentity();
    m_Projection = DirectX::XMMatrixIdentity();
    m_ViewProjection = DirectX::XMMatrixIdentity();
    m_ModelViewProjection = DirectX::XMMatrixIdentity();
    m_invertY = false;

    m_CameraRadius = 2.0f;
}

void OrbitCamera::ChangeRadius(float delta)
{
    m_CameraRadius += delta;

    m_CameraRadius = clamp(m_CameraRadius, 1.0f, 10.0f);
}

void OrbitCamera::Translate(float x, float y)
{
    auto rightScaled = DirectX::XMVectorScale(m_Right, x * -0.001f);
    auto upScaled = DirectX::XMVectorScale(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), y * -0.001f);

    m_EyeFocusPoint = DirectX::XMVectorAdd(m_EyeFocusPoint, DirectX::XMVectorAdd(rightScaled, upScaled));
}

void OrbitCamera::SetProjection(float width, float height)
{
    float aspect = width / height;
    m_Projection = DirectX::XMMatrixPerspectiveFovLH(degreesToRadians(78), aspect, 0.01f, 100.0f);
}

void OrbitCamera::SetInvertY(bool invertY)
{
    m_invertY = invertY;
}

void OrbitCamera::SetEyeFocusPoint(float x, float y, float z)
{
    m_EyeFocusPoint = DirectX::XMVectorSet(x, y, z, 1.0f);
}

void OrbitCamera::RotateAroundPoint(float polar, float azimuth)
{
    float y = m_CameraRadius * sinf(azimuth);
    float r = m_CameraRadius * cosf(azimuth);
    float x = r * cosf(polar);
    float z = r * sinf(polar);

    DirectX::XMVECTOR At = m_EyeFocusPoint;
    DirectX::XMVECTOR Eye = DirectX::XMVectorAdd(m_EyeFocusPoint, DirectX::XMVectorSet(x, y, z, 0.0f));
    DirectX::XMVECTOR Up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    m_View = DirectX::XMMatrixLookAtLH(Eye, At, Up);
    m_Forward = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(At, Eye));
    m_Right = DirectX::XMVector3Cross(m_Forward, DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
}

void OrbitCamera::Update(double deltaTime)
{
    m_ViewProjection = m_View * m_Projection;
    m_ModelViewProjection = m_World * m_View * m_Projection;
}

DirectX::XMMATRIX& OrbitCamera::GetMVP()
{
    return m_ModelViewProjection;
}

DirectX::XMMATRIX& OrbitCamera::GetVP()
{
    return m_ViewProjection;
}

DirectX::XMMATRIX& OrbitCamera::GetView()
{
    return m_View;
}
