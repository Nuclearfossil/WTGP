#pragma once

#include <d3d11.h>
#include <directxmath.h>
#include <d3dcompiler.h>
#include <d3dcommon.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>
#include <vector>

#include "Game.h"
#include "Shader.h"
#include "Grid.h"
#include "Cube.h"
#include "Mesh.h"


/// @brief Structure defining the Constant buffer. This buffer will be used to pass data into the shader
struct ConstantBuffer
{
    DirectX::XMMATRIX mModelViewProjection;
};

class GraphicsDX11
{
public:
    GraphicsDX11();

    // D3D11 related functions ---------------------------------------------------
    HRESULT CreateD3D11DeviceAndContext(HWND hWnd, UINT width, UINT height);
    HRESULT CreateD3D11Context(ID3D11Device* device, ID3D11DeviceContext** context);
    HRESULT CreateRenderTargetView();
    ID3D11Texture2D* GetBackBuffer(IDXGISwapChain* swapChain);
    ID3D11Device* GetD3DDevice() { return m_D3DDevice; }
    ID3D11DeviceContext* GetD3DDeviceContext() { return m_D3DContext; }

    HRESULT CreateD3DResources();
    HRESULT LoadAndCompileShaders();
    HRESULT CreateVertexAndIndexBuffers();
    HRESULT CreateDepthStencilAndRasterizerState();

    void SetMVP(DirectX::XMMATRIX const& mvp) { m_MVP = mvp; }

    void SetVP(DirectX::XMMATRIX const& vp) { m_VP = vp; }

    void SetViewport(D3D11_VIEWPORT viewport) { m_viewport = viewport; }

    void Render(HWND hWnd, RECT winRect, GameData& data, double increment);

    void Cleanup();

private:
    IDXGISwapChain* m_SwapChain = nullptr; // DXGI swapchain for double/triple buffering

    ID3D11Device* m_D3DDevice = nullptr;                        // The Direct3D Device
    ID3D11DeviceContext* m_D3DContext = nullptr;                // The Direct3D Device Context
    ID3D11RenderTargetView* m_D3DRenderTargetView = nullptr;    // The Render Target View

    Shader m_shader;

    Cube m_cube;
    Grid m_grid;
    Mesh m_gizmoXYZ;


    ID3D11Buffer* m_mvpConstantBuffer = nullptr;  // The constant buffer for the WVP matrices
    ID3D11DepthStencilView* m_depthBufferView = nullptr; // The Depth/Stencil view buffer
    ID3D11DepthStencilState* m_depthStencilState = nullptr; // The Depth/Stencil State
    ID3D11RasterizerState* m_rasterizerState;     // The Rasterizer State

    D3D11_VIEWPORT m_viewport;
    DirectX::XMMATRIX m_MVP;
    DirectX::XMMATRIX m_VP;

    const std::vector<float> g_clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
};
