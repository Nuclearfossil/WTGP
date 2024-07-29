#pragma once

HRESULT InitIMGUI(HWND hWnd, GraphicsDX11& graphics);
bool HandleWindowsMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool CheckGuiTrapsMouse();
void DrawUI(GameData& data);
void DestroyIMGUI();