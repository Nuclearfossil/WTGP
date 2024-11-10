#include <windowsx.h>

#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <stdio.h>

#include "framework.h"
#include "GraphicsDX11.h"
#include "UserInterface.h"
#include <cstdio>

//
// if you're ever looking for help debugging/creating widgets:
//    https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html

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
    ImU32 backgroundColors[4] = {
        ImGui::GetColorU32(ImVec4(1.0f, 0.0f, 0.0f, 1.0f)),
        ImGui::GetColorU32(ImVec4(0.0f, 0.8f, 0.0f, 1.0f)),
        ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 1.0f, 1.0f)),
        ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, 1.0f))
    };

    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();

    ImGui::Begin("App Settings");

    ImGui::Checkbox("Invert Y Axis", &data.m_InvertYAxis); // Edit if we want to invert the Y axis

    ImGui::BeginTable("nested_table", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable);
    {

        ImGui::TableSetupColumn("Transform 01");
        ImGui::TableSetupColumn("Transform 02");
        ImGui::TableHeadersRow();

        ImGui::TableNextColumn();
        ImGui::Checkbox("Render Transform 01", &data.m_showTransform01);
        ImGui::SetNextItemOpen(true);
        if (ImGui::TreeNode("Transform 01"))
        {
            ImGui::SetNextItemOpen(true);
            if (ImGui::TreeNode("Position"))
            {
                char buf1[12];
                char buf2[12];
                char buf3[12];
                snprintf(buf1, sizeof(buf1), "%.2f", data.m_cubePosition1[0]);
                snprintf(buf2, sizeof(buf2), "%.2f", data.m_cubePosition1[1]);
                snprintf(buf3, sizeof(buf3), "%.2f", data.m_cubePosition1[2]);
                ImGui::BeginGroup();
                if (ImGui::InputText("X Axis", buf1, 32, ImGuiInputTextFlags_CharsDecimal))
                    data.m_cubePosition1[0] = strtof(buf1, nullptr);
                if (ImGui::InputText("Y Axis", buf2, 32, ImGuiInputTextFlags_CharsDecimal))
                    data.m_cubePosition1[1] = strtof(buf2, nullptr);
                if (ImGui::InputText("Z Axis", buf3, 32, ImGuiInputTextFlags_CharsDecimal))
                    data.m_cubePosition1[2] = strtof(buf3, nullptr);

                if (ImGui::Button("Reset"))
                {
                    data.m_cubePosition1[0] = 0.0f;
                    data.m_cubePosition1[1] = 0.0f;
                    data.m_cubePosition1[2] = 0.0f;
                }
                ImGui::EndGroup();
                ImGui::TreePop();
            }

            ImGui::SetNextItemOpen(true);
            if (ImGui::TreeNode("Rotation"))
            {
                char buf1[12];
                char buf2[12];
                char buf3[12];
                snprintf(buf1, sizeof(buf1), "%.2f", data.m_cubeRotation1[0]);
                snprintf(buf2, sizeof(buf3), "%.2f", data.m_cubeRotation1[1]);
                snprintf(buf3, sizeof(buf3), "%.2f", data.m_cubeRotation1[2]);

                ImGui::BeginGroup();
                if (ImGui::InputText("X Axis", buf1, 32, ImGuiInputTextFlags_CharsDecimal))
                    data.m_cubeRotation1[0] = strtof(buf1, nullptr);
                if (ImGui::InputText("Y Axis", buf2, 32, ImGuiInputTextFlags_CharsDecimal))
                    data.m_cubeRotation1[1] = strtof(buf2, nullptr);
                if (ImGui::InputText("Z Axis", buf3, 32, ImGuiInputTextFlags_CharsDecimal))
                    data.m_cubeRotation1[2] = strtof(buf3, nullptr);
                if (ImGui::Button("Reset"))
                {
                    data.m_cubeRotation1[0] = 0.0f;
                    data.m_cubeRotation1[1] = 0.0f;
                    data.m_cubeRotation1[2] = 0.0f;
                }
                ImGui::EndGroup();
                ImGui::TreePop();
            }
            ImGui::TreePop();
            ImGui::BeginGroup();
            ImGui::BeginTable("TF01_mat", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable);
            ImGui::TableSetupColumn("01");
            ImGui::TableSetupColumn("02");
            ImGui::TableSetupColumn("03");
            ImGui::TableSetupColumn("04");
            ImGui::TableHeadersRow();

            for (int index = 0; index < 4; index++)
            {
                ImGui::TableNextColumn();
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, backgroundColors[index]);
                {
                    char buf1[12];
                    char buf2[12];
                    char buf3[12];
                    char buf4[12];
                    snprintf(buf1, sizeof(buf1), "%.2f", data.m_matrix01.r[index].m128_f32[0]);
                    snprintf(buf2, sizeof(buf3), "%.2f", data.m_matrix01.r[index].m128_f32[1]);
                    snprintf(buf3, sizeof(buf3), "%.2f", data.m_matrix01.r[index].m128_f32[2]);
                    snprintf(buf4, sizeof(buf4), "%.2f", data.m_matrix01.r[index].m128_f32[2]);
                    ImGui::Text(buf1);
                    ImGui::TableNextColumn();
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, backgroundColors[index]);
                    ImGui::Text(buf2);
                    ImGui::TableNextColumn();
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, backgroundColors[index]);
                    ImGui::Text(buf3);
                    ImGui::TableNextColumn();
                    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, backgroundColors[index]);
                    ImGui::Text(buf4);
                }
                ImGui::TableNextRow();
            }
            ImGui::EndTable();
            ImGui::EndGroup();
        }
    }

    ImGui::TableNextColumn();
    ImGui::Checkbox("Render Transform 02", &data.m_showTransform02);
    ImGui::SetNextItemOpen(true);
    if (ImGui::TreeNode("Transform 02"))
    {
        ImGui::SetNextItemOpen(true);
        if (ImGui::TreeNode("Position"))
        {
            char buf1[12];
            char buf2[12];
            char buf3[12];
            snprintf(buf1, sizeof(buf1), "%.2f", data.m_cubePosition2[0]);
            snprintf(buf2, sizeof(buf2), "%.2f", data.m_cubePosition2[1]);
            snprintf(buf3, sizeof(buf3), "%.2f", data.m_cubePosition2[2]);
            ImGui::BeginGroup();
            if (ImGui::InputText("X Axis", buf1, 32, ImGuiInputTextFlags_CharsDecimal))
                data.m_cubePosition2[0] = strtof(buf1, nullptr);
            if (ImGui::InputText("Y Axis", buf2, 32, ImGuiInputTextFlags_CharsDecimal))
                data.m_cubePosition2[1] = strtof(buf2, nullptr);
            if (ImGui::InputText("Z Axis", buf3, 32, ImGuiInputTextFlags_CharsDecimal))
                data.m_cubePosition2[2] = strtof(buf3, nullptr);

            if (ImGui::Button("Reset"))
            {
                data.m_cubePosition2[0] = 0.0f;
                data.m_cubePosition2[1] = 0.0f;
                data.m_cubePosition2[2] = 0.0f;
            }
            ImGui::EndGroup();
            ImGui::TreePop();
        }

        ImGui::SetNextItemOpen(true);
        if (ImGui::TreeNode("Rotation"))
        {
            char buf1[12];
            char buf2[12];
            char buf3[12];
            snprintf(buf1, sizeof(buf1), "%.2f", data.m_cubeRotation2[0]);
            snprintf(buf2, sizeof(buf2), "%.2f", data.m_cubeRotation2[1]);
            snprintf(buf3, sizeof(buf3), "%.2f", data.m_cubeRotation2[2]);

            ImGui::BeginGroup();
            if (ImGui::InputText("X Axis", buf1, 32, ImGuiInputTextFlags_CharsDecimal))
                data.m_cubeRotation2[0] = strtof(buf1, nullptr);
            if (ImGui::InputText("Y Axis", buf2, 32, ImGuiInputTextFlags_CharsDecimal))
                data.m_cubeRotation2[1] = strtof(buf2, nullptr);
            if (ImGui::InputText("Z Axis", buf3, 32, ImGuiInputTextFlags_CharsDecimal))
                data.m_cubeRotation2[2] = strtof(buf3, nullptr);
            if (ImGui::Button("Reset"))
            {
                data.m_cubeRotation2[0] = 0.0f;
                data.m_cubeRotation2[1] = 0.0f;
                data.m_cubeRotation2[2] = 0.0f;
            }
            ImGui::EndGroup();
            ImGui::TreePop();
        }
        ImGui::BeginGroup();
        ImGui::BeginTable("TF02_mat", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable);
        ImGui::TableSetupColumn("01");
        ImGui::TableSetupColumn("02");
        ImGui::TableSetupColumn("03");
        ImGui::TableSetupColumn("04");
        ImGui::TableHeadersRow();

        for (int index = 0; index < 4; index++)
        {
            ImGui::TableNextColumn();
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, backgroundColors[index]);
            {
                char buf1[12];
                char buf2[12];
                char buf3[12];
                char buf4[12];
                snprintf(buf1, sizeof(buf1), "%.2f", data.m_matrix02.r[index].m128_f32[0]);
                snprintf(buf2, sizeof(buf3), "%.2f", data.m_matrix02.r[index].m128_f32[1]);
                snprintf(buf3, sizeof(buf3), "%.2f", data.m_matrix02.r[index].m128_f32[2]);
                snprintf(buf4, sizeof(buf4), "%.2f", data.m_matrix02.r[index].m128_f32[2]);
                ImGui::Text(buf1);
                ImGui::TableNextColumn();
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, backgroundColors[index]);
                ImGui::Text(buf2);
                ImGui::TableNextColumn();
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, backgroundColors[index]);
                ImGui::Text(buf3);
                ImGui::TableNextColumn();
                ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, backgroundColors[index]);
                ImGui::Text(buf4);
            }
            ImGui::TableNextRow();
        }
        ImGui::EndTable();
        ImGui::EndGroup();
        ImGui::TreePop();
    }
    ImGui::EndTable();

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