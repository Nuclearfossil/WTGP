#pragma once

HRESULT InitIMGUI(HWND hWnd, GraphicsDX11& graphics);
bool HandleWindowsMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool CheckGuiTrapsMouse();
void DrawUI(GameData& data, std::shared_ptr<SceneNode> sceneRoot);
void DestroyIMGUI();

void DrawMatrix(const char* tableName, DirectX::XMMATRIX& matrix, bool enhanceMatrix);
