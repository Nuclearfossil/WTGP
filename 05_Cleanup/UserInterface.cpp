#include <windowsx.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include "framework.h"
#include "GraphicsDX11.h"
#include "UserInterface.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/// @brief Passthrough to allow ImGui to do it's own windows message pump handling
/// @param hWnd Handle to window
/// @param msg Windows Message
/// @param wParam WPARAM
/// @param lParam LPRARM
/// @return true if no additional processing needs to be handled.
bool HandleWindowsMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
}

/// @brief Does the ImGui system trap the mouse (do not pass on through to the app)
/// @return true if ImGui is trapping the mouse.
bool CheckGuiTrapsMouse()
{
    return ImGui::GetIO().WantCaptureMouse;
}

/// @brief Initialize Everything relating to ImGui
/// @return S_OK if successful
HRESULT InitIMGUI(HWND hWnd, GraphicsDX11& graphics)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    io.MouseDrawCursor = true;                            // Avoid mouse lag

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(graphics.GetD3DDevice(), graphics.GetD3DDeviceContext());

    return S_OK;
}

/// @brief Draw our UI
void DrawUI(GameData& data)
{
    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();

    ImGui::Begin("App Settings");

    ImGui::Checkbox("Invert Y Axis", &data.m_InvertYAxis); // Edit if we want to invert the Y axis

    ImGui::SliderFloat3("Object Position", data.m_cubePosition, -3.0f, 3.0f);
    ImGui::SliderFloat3("Object Rotation", data.m_cubeRotation, -360.0f, 360.0f);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();

    // Rendering
    ImGui::Render();
}

/// @brief Clean up IMGui
void DestroyIMGUI()
{
    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}