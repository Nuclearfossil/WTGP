#pragma once

#include <directxmath.h>

class Camera3D
{
public:
    Camera3D() = default;

    void Initialize();

    void RotateAroundPoint(DirectX::XMVECTOR point, float polar, float azimuth);

    void Update(double deltaTime);

    void SetProjection(float width, float height, float projection);
    void SetInvertY(bool invertY);

    void ChangeRadius(float delta);

    DirectX::XMMATRIX& GetMVP();
    DirectX::XMMATRIX& GetVP();

private:
    DirectX::XMMATRIX m_World;      // The Model transform matrix
    DirectX::XMMATRIX m_View;       // The View (Camera) matrix
    DirectX::XMMATRIX m_Projection; // the Perspecitve projection matrix

    DirectX::XMMATRIX m_ViewProjection;
    DirectX::XMMATRIX m_ModelViewProjection;

    float m_CameraRadius;           // distance of the camera to the polar center point
    bool m_invertY;                 // invert the Y axis
};
