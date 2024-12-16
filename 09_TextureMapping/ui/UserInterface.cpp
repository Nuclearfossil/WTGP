#include <windowsx.h>

#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <stdio.h>

#include "framework.h"
#include "GraphicsDX11.h"
#include "UserInterface.h"
#include "OrbitCamera.h"
#include <cstdio>
#include <GameData.h>

//
// if you're ever looking for help debugging/creating widgets:
//    https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void DrawMatrix(const char* tableName, DirectX::XMMATRIX& matrix, bool enhanceMatrix);

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

struct TreeNodeData
{
    bool showPosition = { true };
    bool showRotation = { true };
};

static void DrawTransform(float (&position)[3], float (&rotation)[3], TreeNodeData& treeNode)
{
    ImGui::SetNextItemOpen(treeNode.showPosition);
    treeNode.showPosition = ImGui::TreeNode("Position");
    if (treeNode.showPosition)
    {
        char buf1[8];
        char buf2[8];
        char buf3[8];
        snprintf(buf1, sizeof(buf1), "%.2f", position[0]);
        snprintf(buf2, sizeof(buf2), "%.2f", position[1]);
        snprintf(buf3, sizeof(buf3), "%.2f", position[2]);
        ImGui::BeginGroup();
        if (ImGui::InputText("X Axis", buf1, 32, ImGuiInputTextFlags_CharsDecimal))
            position[0] = strtof(buf1, nullptr);
        if (ImGui::InputText("Y Axis", buf2, 32, ImGuiInputTextFlags_CharsDecimal))
            position[1] = strtof(buf2, nullptr);
        if (ImGui::InputText("Z Axis", buf3, 32, ImGuiInputTextFlags_CharsDecimal))
            position[2] = strtof(buf3, nullptr);

        if (ImGui::Button("Reset"))
        {
            position[0] = 0.0f;
            position[1] = 0.0f;
            position[2] = 0.0f;
        }
        ImGui::EndGroup();
        ImGui::TreePop();
     }

    ImGui::SetNextItemOpen(treeNode.showRotation);
    treeNode.showRotation = ImGui::TreeNode("Rotation");
    if (treeNode.showRotation)
    {
        char buf1[8];
        char buf2[8];
        char buf3[8];
        snprintf(buf1, sizeof(buf1), "%.2f", rotation[0]);
        snprintf(buf2, sizeof(buf3), "%.2f", rotation[1]);
        snprintf(buf3, sizeof(buf3), "%.2f", rotation[2]);

        ImGui::BeginGroup();
        if (ImGui::InputText("X Axis", buf1, 32, ImGuiInputTextFlags_CharsDecimal))
            rotation[0] = strtof(buf1, nullptr);
        if (ImGui::InputText("Y Axis", buf2, 32, ImGuiInputTextFlags_CharsDecimal))
            rotation[1] = strtof(buf2, nullptr);
        if (ImGui::InputText("Z Axis", buf3, 32, ImGuiInputTextFlags_CharsDecimal))
            rotation[2] = strtof(buf3, nullptr);
        if (ImGui::Button("Reset"))
        {
            rotation[0] = 0.0f;
            rotation[1] = 0.0f;
            rotation[2] = 0.0f;
        }
        ImGui::EndGroup();
        ImGui::TreePop();
    }

    ImGui::TreePop();
}

void DrawLight(const char* name, LightData& light)
{
    ImGui::BeginTable("LightPosition", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable);
    ImGui::TableSetupColumn("X");
    ImGui::TableSetupColumn("Y");
    ImGui::TableSetupColumn("Z");
    ImGui::TableHeadersRow();

    ImGui::TableNextColumn();

    char buf1[8];
    char buf2[8];
    char buf3[8];
    snprintf(buf1, sizeof(buf1), "%.2f", light.m_LightPosition[0]);
    snprintf(buf2, sizeof(buf3), "%.2f", light.m_LightPosition[1]);
    snprintf(buf3, sizeof(buf3), "%.2f", light.m_LightPosition[2]);
    if (ImGui::InputText("XPosition", buf1, 32, ImGuiInputTextFlags_CharsDecimal))
        light.m_LightPosition[0] = strtof(buf1, nullptr);
    ImGui::TableNextColumn();
    if (ImGui::InputText("YPosition", buf2, 32, ImGuiInputTextFlags_CharsDecimal))
        light.m_LightPosition[1] = strtof(buf2, nullptr);
    ImGui::TableNextColumn();
    if (ImGui::InputText("ZPosition", buf3, 32, ImGuiInputTextFlags_CharsDecimal))
        light.m_LightPosition[2] = strtof(buf3, nullptr);

    ImGui::EndTable();

    ImGui::ColorEdit4("Diffuse", light.m_Diffuse);

}

/// @brief Draw our UI
void DrawUI(GameData& data)
{
    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();

    ImGui::Begin("App Settings");

    static bool enhanceMatrix = false;
    static bool expandTransform01 = true;
    static bool expandTransform02 = true;
    static bool expandTransform03 = true;
    static TreeNodeData transform01TreeNode;
    static TreeNodeData transform02TreeNode;
    static TreeNodeData transform03TreeNode;

    ImGui::Checkbox("Invert Y Axis", &data.m_InvertYAxis); // Edit if we want to invert the Y axis
    ImGui::Checkbox("Enhance Matrix", &enhanceMatrix);

    ImGui::BeginTable("nested_table01", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable);
    ImGui::TableSetupColumn("Transform 01");
    ImGui::TableSetupColumn("Transform 02");
    ImGui::TableHeadersRow();

    ImGui::TableNextColumn();
    ImGui::Checkbox("Render Transform 01", &data.m_showTransform01);
    ImGui::SetNextItemOpen(expandTransform01);
    expandTransform01 = ImGui::TreeNode("Transform 01");
    if (expandTransform01)
    {
        DrawTransform(data.m_cubePosition1, data.m_cubeRotation1, transform01TreeNode);
        DrawMatrix("TF01_mat", data.m_matrix01, enhanceMatrix);
    }

    ImGui::TableNextColumn();
    ImGui::Checkbox("Render Transform 02", &data.m_showTransform02);
    ImGui::SetNextItemOpen(expandTransform02);
    expandTransform02 = ImGui::TreeNode("Transform 02");
    if (expandTransform02)
    {
        DrawTransform(data.m_cubePosition2, data.m_cubeRotation2, transform02TreeNode);
        DrawMatrix("TF02_mat", data.m_matrix02, enhanceMatrix);
    }
    ImGui::EndTable();

    ImGui::BeginTable("nested_table02", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable);
    ImGui::TableSetupColumn("Textured Mesh");
    ImGui::TableSetupColumn("Camera Matrix");
    ImGui::TableHeadersRow();

    ImGui::TableNextColumn();
    ImGui::SetNextItemOpen(expandTransform03);
    expandTransform03 = ImGui::TreeNode("Textured Mesh");
    if (expandTransform03)
        DrawTransform(data.m_texturedMeshPosition, data.m_texturedMeshRotation, transform03TreeNode);

    ImGui::TableNextColumn();
    ImGui::Text("Camera View Matrix");
    DrawMatrix("Camera", data.m_Camera->GetVP(), false);
    ImGui::EndTable();

    ImGui::Text("Light Information");
    DrawLight("Light01", data.m_Light);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();

    // Rendering
    ImGui::Render();
}

void DrawMatrix(const char* tableName, DirectX::XMMATRIX& matrix, bool enhanceMatrix)
{
    ImU32 backgroundColors[4] = {
        ImGui::GetColorU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)),
        ImGui::GetColorU32(ImVec4(0.0f, 0.8f, 0.0f, 1.0f)),
        ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 1.0f, 1.0f)),
        ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f))
    };

    ImGui::BeginGroup();
    ImGui::BeginTable(tableName, 5, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable);

    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("01", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("02", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("03", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("04", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();

    for (int index = 0; index < 4; index++)
    {
        char buf0[8];
        snprintf(buf0, sizeof(buf0), "%02d", index + 1);

        ImGui::TableNextColumn();
        ImGui::Text(buf0);

        ImGui::TableNextColumn();

        if (enhanceMatrix)
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, backgroundColors[index]);

        {
            char buf1[8];
            char buf2[8];
            char buf3[8];
            char buf4[8];
            snprintf(buf1, sizeof(buf1), "%.2f", matrix.r[index].m128_f32[0]);
            snprintf(buf2, sizeof(buf3), "%.2f", matrix.r[index].m128_f32[1]);
            snprintf(buf3, sizeof(buf3), "%.2f", matrix.r[index].m128_f32[2]);
            snprintf(buf4, sizeof(buf4), "%.2f", matrix.r[index].m128_f32[2]);
            ImGui::Text(buf1);
            ImGui::TableNextColumn();
            if (enhanceMatrix)
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, backgroundColors[index]);
            ImGui::Text(buf2);
            ImGui::TableNextColumn();
            if (enhanceMatrix)
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, backgroundColors[index]);
            ImGui::Text(buf3);
            ImGui::TableNextColumn();
            if (enhanceMatrix)
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, backgroundColors[index]);
            ImGui::Text(buf4);
        }
        ImGui::TableNextRow();
    }

    ImGui::EndTable();
    ImGui::EndGroup();
}
    /// @brief Clean up IMGui
void DestroyIMGUI()
{
    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}