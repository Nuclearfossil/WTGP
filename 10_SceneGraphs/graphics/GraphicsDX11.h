#pragma once

#include <d3d11.h>
#include <directxmath.h>
#include <vector>

#include "ConstantBuffers.h"
#include "SceneNode.h"
#include "GameData.h"
#include "Shader.h"
#include "Grid.h"
#include "Cube.h"
#include "Plane.h"
#include "Mesh.h"
#include "TexturedMesh.h"
#include "Light.h"
#include "Sphere.h"

#include <dxgi.h>
#include <minwindef.h>
#include <windef.h>
#include <winnt.h>


enum class FillMode
{
    Wireframe,
    Solid
};

enum class CullMode
{
    None,
    Backface,
    Frontface
};

class GraphicsDX11
{
public:
    GraphicsDX11() = default;

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

    void SetWorldViewProjection(DirectX::XMMATRIX const& mvp) { m_MVP = mvp; }

    //void SetViewProjection(DirectX::XMMATRIX const& vp) { m_VP = vp; }

    void SetViewport(D3D11_VIEWPORT viewport) { m_viewport = viewport; }

    void Update(double deltaTime);

    void Render(HWND hWnd, RECT winRect, GameData& data, double increment);

    void Cleanup();

private:
    ID3D11Device* m_D3DDevice = nullptr;   // The Direct3D Device

    ID3D11DeviceContext* m_D3DContext = nullptr;                // The Direct3D Device Context
    ID3D11RenderTargetView* m_D3DRenderTargetView = nullptr;    // The Render Target View

    IDXGISwapChain* m_SwapChain = nullptr; // DXGI swapchain for double/triple buffering

    std::shared_ptr<Shader> m_shader;
    std::shared_ptr<Shader> m_lightGeometryShader;
    Shader m_simpleLit;
    Shader m_texturedShader;

    std::shared_ptr<SceneNode> m_SceneRoot;
    std::shared_ptr<Grid> m_grid;
    std::shared_ptr<Mesh> m_gizmoXYZ01;
    std::shared_ptr<Mesh> m_gizmoXYZ02;
    std::shared_ptr<Cube> m_cube;
    std::shared_ptr<Plane> m_plane;
    std::shared_ptr<TexturedMesh> m_texturedMesh;
    std::shared_ptr<Light> m_light;
    std::shared_ptr<Sphere> m_sphere;

    ID3D11Buffer* m_viewProjectionConstantBuffer = nullptr; // The constant buffer for the View Projection matrix
//    ID3D11Buffer* m_localToWorldConstantBuffer = nullptr;   // The constant buffer for the local to world matrix
    ID3D11Buffer* m_lightConstantBuffer = nullptr;          // The constant buffer for lighting
    ID3D11DepthStencilView* m_depthBufferView = nullptr;    // The Depth/Stencil view buffer
    ID3D11DepthStencilState* m_depthStencilState = nullptr; // The Depth/Stencil State
    ID3D11RasterizerState* m_rasterizerState;               // The Rasterizer State

    D3D11_VIEWPORT m_viewport;
    //DirectX::XMMATRIX m_World;
    DirectX::XMMATRIX m_MVP;
    //DirectX::XMMATRIX m_VP;

    const std::vector<float> g_clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

    FillMode m_fillmode = FillMode::Solid;
    CullMode m_cullmode = CullMode::None;
};
