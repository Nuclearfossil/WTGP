#include "pch.h"

#include <d3d11.h>
#include <directxmath.h>

#include "mathutils.h"
#include "Camera3D.h"

void Camera3D::Initialize()
{
    m_World = DirectX::XMMatrixIdentity();
    m_View = DirectX::XMMatrixIdentity();
    m_Projection = DirectX::XMMatrixIdentity();
    m_ViewProjection = DirectX::XMMatrixIdentity();
    m_ModelViewProjection = DirectX::XMMatrixIdentity();
    m_invertY = false;

    m_CameraRadius = 2.0f;
}

void Camera3D::ChangeRadius(float delta)
{
    m_CameraRadius += delta;

    m_CameraRadius = clamp(m_CameraRadius, 1.0f, 10.0f);
}

void Camera3D::SetProjection(float width, float height, float aspect)
{
    m_Projection = DirectX::XMMatrixPerspectiveFovLH(degreesToRadians(78), aspect, 0.01f, 100.0f);
}

void Camera3D::SetInvertY(bool invertY)
{
    m_invertY = invertY;
}

void Camera3D::RotateAroundPoint(DirectX::XMVECTOR point, float polar, float azimuth)
{
    float y = m_CameraRadius * sinf(azimuth);
    float r = m_CameraRadius * cosf(azimuth);
    float x = r * cosf(polar);
    float z = r * sinf(polar);


    DirectX::XMVECTOR Eye = DirectX::XMVectorSet(x, y, z, 1.0f);
    DirectX::XMVECTOR At = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    DirectX::XMVECTOR Up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
    m_View = DirectX::XMMatrixLookAtLH(Eye, At, Up);
}

void Camera3D::Update(double deltaTime)
{
    m_ViewProjection = m_View * m_Projection;
    m_ModelViewProjection = m_World * m_View * m_Projection;
}

DirectX::XMMATRIX& Camera3D::GetMVP()
{
    return m_ModelViewProjection;
}

DirectX::XMMATRIX& Camera3D::GetVP()
{
    return m_ViewProjection;
}


