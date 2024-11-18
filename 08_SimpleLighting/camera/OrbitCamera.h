#pragma once

#include <directxmath.h>

class OrbitCamera
{
public:
    OrbitCamera() = default;

    void Initialize();

    void RotateAroundPoint(float polar, float azimuth);

    void Update(double deltaTime);

    void SetProjection(float width, float height, float projection);
    void SetInvertY(bool invertY);
    void SetEyeFocusPoint(float x, float y, float z);

    void ChangeRadius(float delta);

    void Translate(float x, float y);

    DirectX::XMMATRIX& GetMV();
    DirectX::XMMATRIX& GetMVP();
    DirectX::XMMATRIX& GetVP();
    DirectX::XMMATRIX& GetView();

private:
    DirectX::XMMATRIX m_World;      // The Model transform matrix
    DirectX::XMMATRIX m_View;       // The View (Camera) matrix
    DirectX::XMMATRIX m_Projection; // the Perspecitve projection matrix

    DirectX::XMMATRIX m_ModelView;
    DirectX::XMMATRIX m_ViewProjection;
    DirectX::XMMATRIX m_ModelViewProjection;

    DirectX::XMVECTOR m_EyeFocusPoint = { 0.0f };
    DirectX::XMVECTOR m_Forward = { 0.0f, 0.0f, 1.0f };
    DirectX::XMVECTOR m_Right = { 1.0f, 0.0f, 0.0f };

    float m_CameraRadius;           // distance of the camera to the polar center point
    bool m_invertY;                 // invert the Y axis
};
