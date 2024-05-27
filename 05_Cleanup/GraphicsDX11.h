#pragma once

#include <d3d11.h>
#include <directxmath.h>
#include <d3dcompiler.h>
#include <d3dcommon.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>


/// @brief Structure defining the Constant buffer. This buffer will be used to pass data into the shader
struct ConstantBuffer
{
    DirectX::XMMATRIX mModelViewProjection;
};

class GraphicsDX11
{
public:
    GraphicsDX11();

    void Render(HWND hWnd, RECT winRect);

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

    void SetMVP(DirectX::XMMATRIX& mvp) { m_MVP = mvp; }

    void SetVP(DirectX::XMMATRIX& vp) { m_VP = vp; }

    void SetViewport(D3D11_VIEWPORT viewport) { m_viewport = viewport; }

    void Render(HWND hWnd, RECT winRect, double increment);

    void Cleanup();

private:
    IDXGISwapChain* m_SwapChain; // DXGI swapchain for double/triple buffering

    ID3D11Device* m_D3DDevice;                     // The Direct3D Device
    ID3D11DeviceContext* m_D3DContext;             // The Direct3D Device Context
    ID3D11RenderTargetView* m_D3DRenderTargetView; // The Render Target View

    ID3D11VertexShader* m_vertexShader = nullptr; // The Vertex Shader resource used in this example
    ID3D11PixelShader* m_pixelShader = nullptr;   // The Pixel Shader resource used in this example
    ID3D11InputLayout* m_inputLayout = nullptr;   // The Input layout resource used for the vertex shader

    ID3D11Buffer* m_cubeVertexBuffer = nullptr; // The D3D11 Buffer used to hold the vertex data for the cube.
    ID3D11Buffer* m_cubeIndexBuffer = nullptr;  // The D3D11 Index Buffer for the cube

    ID3D11Buffer* m_gridVertexBuffer = nullptr; // The D3D11 Buffer used to hold the vertex data for the grid
    ID3D11Buffer* m_gridIndexBuffer = nullptr;  // The D3D11 Index Buffer for the grid

    ID3D11Buffer* m_mvpConstantBuffer = nullptr;  // The constant buffer for the WVP matrices
    ID3D11DepthStencilView* m_depthBufferView;    // The Depth/Stencil view buffer
    ID3D11DepthStencilState* m_depthStencilState; // The Depth/Stencil State
    ID3D11RasterizerState* m_rasterizerState;     // The Rasterizer State

    D3D11_VIEWPORT m_viewport;
    DirectX::XMMATRIX m_MVP;
    DirectX::XMMATRIX m_VP;

    // Input Assembler and Vertex Buffer globals
    UINT g_stride = 0;
    UINT g_offset = 0;
    UINT g_numCubeVerts = 0;
    uint16_t g_numCubeIndices = 0;
    UINT g_numGridVerts = 0;
    uint16_t g_numGridIndices = 0;

    const float g_clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
};
