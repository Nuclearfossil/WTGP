#pragma once

struct GameData
{
    int m_lastX = 0;
    int m_lastY = 0;

    int m_deltaMouseX = 0;
    int m_deltaMouseY = 0;
    float m_wheelDelta = 0.f;

    double m_delta = 0.0;

    bool m_InvertYAxis = false;

    float m_increment = 0;

    float m_cubePosition[3] = { 0.f };
    float m_cubeRotation[3] = { 0.f };
};