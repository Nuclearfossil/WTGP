#pragma once

#include <DirectXMath.h>

constexpr int MAX_LOADSTRING = 1000;

class OrbitCamera;

struct LightData
{
    float m_LightPosition[3];
    float m_Diffuse[4];
};

struct GameData
{
    HINSTANCE m_hInst = 0;
    HWND m_hWnd = 0;
    RECT m_winRect
    {
        0, 0, 0, 0
    };

    WCHAR m_szTitle[MAX_LOADSTRING] {};
    WCHAR m_szWindowClass[MAX_LOADSTRING] {};

    int m_lastX = 0;
    int m_lastY = 0;

    int m_deltaMouseX = 0;
    int m_deltaMouseY = 0;
    float m_wheelDelta = 0.f;

    bool m_LMBDown = false;

    int m_deltaTransformX = 0;
    int m_deltaTransformY = 0;

    double m_delta = 0.0;

    bool m_InvertYAxis = false;

    float m_cubePosition1[3] = { 0.f };
    float m_cubeRotation1[3] = { 0.f };

    float m_cubePosition2[3] = { 0.f };
    float m_cubeRotation2[3] = { 0.f };

    float m_increment = 0;

    DirectX::XMMATRIX m_matrix01;
    DirectX::XMMATRIX m_matrix02;

    bool m_showTransform01 = true;
    bool m_showTransform02 = true;

    OrbitCamera* m_Camera = nullptr;

    LightData m_Light = { 0 };
};
