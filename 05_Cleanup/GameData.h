#pragma once

constexpr int MAX_LOADSTRING = 1000;

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

    double m_delta = 0.0;

    bool m_InvertYAxis = false;

    float m_increment = 0;
};
