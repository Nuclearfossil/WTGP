// 03_RenderingAndResourcesApp.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "framework.h"
#include "01_WindowsApp.h"
#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND g_hWnd;                                    // Handle to the main window
IDXGISwapChain* g_SwapChain;                    // DXGI swapchain for double/triple buffering
WCHAR g_szTitle[MAX_LOADSTRING];                // The title bar text
WCHAR g_szWindowClass[MAX_LOADSTRING];          // The main window class name

ID3D11Device* g_D3DDevice;                      // The Direct3D Device
ID3D11DeviceContext* g_D3DContext;              // The Direct3D Device Context
ID3D11RenderTargetView* g_D3DRenderTargetView;  // The Render Target View

ID3D11VertexShader* g_vertexShader = nullptr;   // The Vertex Shader resource used in this example
ID3D11PixelShader* g_pixelShader = nullptr;     // The Pixel Shader resource used in this example
ID3D11InputLayout* g_inputLayout = nullptr;     // The Input layout resource used for the vertex shader
ID3D11Buffer* g_vertexBuffer = nullptr;         // The D3D11 Buffer used to hold the vertex data.

float g_clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
double g_delta;

UINT g_numVerts = 0;
UINT g_stride = 0;
UINT g_offset = 0;

const char* g_vertexShaderSource =
"struct VS_Input \
{ \
	float2 pos : POS; \
	float4 color : COL; \
}; \
\
struct VS_Output \
{ \
	float4 position : SV_POSITION; \
	float4 color : COL; \
}; \
 \
VS_Output vs_main(VS_Input input) \
{ \
	VS_Output output; \
	output.position = float4(input.pos, 0.0f, 1.0f); \
	output.color = input.color; \
 \
	return output; \
}";

const char* g_pixelShaderSource =
"struct VS_Output \
{ \
	float4 position : SV_POSITION; \
	float4 color : COL; \
}; \
 \
float4 ps_main(VS_Output input) : SV_TARGET \
{ \
	return input.color; \
}";

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

HRESULT CreateD3D11Context(ID3D11Device* device, ID3D11DeviceContext** context);
HRESULT CreateD3D11DeviceAndContext(HWND hWnd, UINT width, UINT height, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext, IDXGISwapChain** ppSwapChain);
HRESULT CreateRenderTargetView(ID3D11Device* device, ID3D11Texture2D* backBuffer, ID3D11RenderTargetView** renderTargetView);
ID3D11Texture2D* GetBackBuffer(IDXGISwapChain* swapChain);

HRESULT CreateD3DResources();

void Update(double deltaInSeconds);
void Render(ID3D11Device* device, ID3D11DeviceContext* context, IDXGISwapChain* swapChain, ID3D11RenderTargetView* renderTargetView);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, g_szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY01WINDOWSAPP, g_szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY01WINDOWSAPP));

    if (!SUCCEEDED(CreateD3D11DeviceAndContext(g_hWnd, 1024, 768, &g_D3DDevice, &g_D3DContext, &g_SwapChain)))
        return -1;

    auto backBuffer = GetBackBuffer(g_SwapChain);

    if (!SUCCEEDED(CreateRenderTargetView(g_D3DDevice, backBuffer, &g_D3DRenderTargetView)))
		return -2;

    backBuffer->Release();

    if (!SUCCEEDED(CreateD3DResources()))
    {
        return -3;
    }

    // Main message loop:
    MSG msg = { 0 };

    LARGE_INTEGER current = { 0 };
    LARGE_INTEGER lastStart = { 0 };
	LARGE_INTEGER frequency = { 0 };
	
    ::QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&lastStart);

    while (msg.message != WM_QUIT)
    {
        QueryPerformanceCounter(&current);

		double deltaSeconds = static_cast<double>(current.QuadPart - lastStart.QuadPart) / frequency.QuadPart;

        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
            }
        }

        // Let's throttle the application so that we render at a constant 
        // speed, regardless of processor speed.
        Update(deltaSeconds);
        Render(g_D3DDevice, g_D3DContext, g_SwapChain, g_D3DRenderTargetView);
        lastStart = current;
    }

    g_D3DRenderTargetView->Release();
    g_D3DContext->Release();
    g_D3DDevice->Release();

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY01WINDOWSAPP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MY01WINDOWSAPP);
    wcex.lpszClassName  = g_szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   RECT windowSize = { 0, 0, 1024, 768 };
   AdjustWindowRect(&windowSize, WS_OVERLAPPEDWINDOW, FALSE);

   HWND hWnd = CreateWindowW(g_szWindowClass, g_szTitle, WS_OVERLAPPEDWINDOW,
       0, 0, windowSize.right - windowSize.left, windowSize.bottom - windowSize.top, 
       nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   g_hWnd = hWnd;

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDK_QUIT:
                PostQuitMessage(0);
                break;
            default:
                break;
            }
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


HRESULT CreateD3D11DeviceAndContext(HWND hWnd, 
    UINT width, 
    UINT height, 
    ID3D11Device** ppDevice, 
    ID3D11DeviceContext** ppContext, 
    IDXGISwapChain** ppSwapChain)
{
	HRESULT hr = S_OK;

	// Define swap chain descriptor
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.Flags = 0;

	// Create device, context, and swap chain
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
	hr = D3D11CreateDeviceAndSwapChain(
        nullptr,                    // First Parameter
        D3D_DRIVER_TYPE_HARDWARE,   // Second Parameter
        nullptr,                    // Third Parameter
        0,                          // Fourth Parameter - use D3D11_CREATE_DEVICE_DEBUG 
                                    //                   if you want additional debug 
                                    //                   spew in the console.
        &featureLevel,              // Fifth Parameter
        1,                          // Sixth Parameter
        D3D11_SDK_VERSION,          // Seventh Parameter
        &swapChainDesc,             // Eighth Parameter
        ppSwapChain,                // Ninth Parameter
        ppDevice,                   // Tenth Parameter
        nullptr,                    // Eleventh Parameter
        ppContext);                 // Twelfth Parameter

	if (FAILED(hr))
	{
        // TODO: Add logging as to why this failed.
		return hr;
	}

	return S_OK;
}

HRESULT CreateD3D11Context(ID3D11Device* device, ID3D11DeviceContext** context)
{
	HRESULT hr = S_OK;

	if (device == nullptr || context == nullptr)
		return E_INVALIDARG;

	// Create the D3D11 context
	hr = device->CreateDeferredContext(0, context);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT CreateD3DResources()
{
    // creation of Shader Resources
    ID3DBlob* vsBlob;
	ID3DBlob* psBlob;
    ID3DBlob* shaderCompileErrorBlob;

    // We compile the Vertex shader from the `vertexShaderSource` source string and check for validity
    if (!SUCCEEDED(D3DCompile(g_vertexShaderSource, strlen(g_vertexShaderSource), "vertexShader", nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vsBlob, &shaderCompileErrorBlob)))
    {
        OutputDebugStringA(static_cast<const char*>(shaderCompileErrorBlob->GetBufferPointer()));
        shaderCompileErrorBlob->Release();
        return S_FALSE;
    }

    // We then create the appropriate Vertex Shader resource: `g_vertexShader`.
    if (!SUCCEEDED(g_D3DDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_vertexShader)))
    {
        OutputDebugStringA("Failed to create the Vertex Shader!\n");
        return S_FALSE;
    }

	// We compile the Pixel shader from the `pixelShaderSource` source string and check for validity
	if (!SUCCEEDED(D3DCompile(g_pixelShaderSource, strlen(g_pixelShaderSource), "pixelShader", nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &psBlob, &shaderCompileErrorBlob)))
    {
        OutputDebugStringA(static_cast<const char*>(shaderCompileErrorBlob->GetBufferPointer()));
        shaderCompileErrorBlob->Release();
        return S_FALSE;
    }

    // We then create the appropriate Pixel Shader resource: `g_pixelShader`
    if (!SUCCEEDED(g_D3DDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_pixelShader)))
    {
        OutputDebugStringA("Failed to create the Pixel Shader\n");
        return S_FALSE;
    }

	psBlob->Release();

	// Create Input Layout - this describes the format of the vertex data we will use.
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

    if (!SUCCEEDED(g_D3DDevice->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_inputLayout)))
    {
        OutputDebugStringA("Failed to create the Input Layout");
        return S_FALSE;
    }

	vsBlob->Release();

	// Populate the array representing the vertex data. In this case, we are going
    // to have 6 elements per vertex:
    // X and Y co-ordinates
    // Colours for each vertex representing the Red, Green, Blue and Alpha channels.
	float vertexData[] = 
    { //   x,     y,   r,   g,   b,   a
		0.0f,  0.5f, 0.f, 1.f, 0.f, 1.f,
		0.5f, -0.5f, 1.f, 0.f, 0.f, 1.f,
	   -0.5f, -0.5f, 0.f, 0.f, 1.f, 1.f
	};

	//float vertexData[] =
	//{
	//	-.90f,  .90f, 0.0f, 1.0f, 0.0f, 1.0f,
	//	 .90f,  .90f, 0.0f, 1.0f, 0.0f, 1.0f,
	//	 .90f, -.90f, 0.0f, 1.0f, 0.0f, 1.0f,
	//	-.90f,  .90f, 1.0f, 0.0f, 0.0f, 1.0f,
	//	 .90f, -.90f, 1.0f, 0.0f, 0.0f, 1.0f,
	//	-.90f, -.90f, 1.0f, 0.0f, 0.0f, 1.0f,
	//};
		
    //float vertexData[] =
    //{
    //   -1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
    //    1.0f,  1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
    //    1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
    //   -1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    //    1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    //   -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    //};

    // The stride represents the _actual_ width of the data for each vertex.
    // In this case, we have 6 elements per vertex.
	g_stride = 6 * sizeof(float);
	g_offset = 0;
	g_numVerts = sizeof(vertexData) / g_stride;

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.ByteWidth = sizeof(vertexData);
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexSubresourceData;
    vertexSubresourceData.pSysMem = vertexData;
    vertexSubresourceData.SysMemPitch = 0;
    vertexSubresourceData.SysMemSlicePitch = 0;

    if (!SUCCEEDED(g_D3DDevice->CreateBuffer(&vertexBufferDesc, &vertexSubresourceData, &g_vertexBuffer)))
    {
        OutputDebugStringA("Failed to create the vertex buffer!");
        return S_FALSE;
    }

    return S_OK;
}

void Update(double deltaInSeconds)
{
    static double desiredTime = 1.0f / 4.0f; // 1 second / desired number of seconds
	static double incrementor = desiredTime;
	if (g_clearColor[2] > 1.0f)
		incrementor = -1 * desiredTime;

	if (g_clearColor[2] < 0.0f)
		incrementor = desiredTime;

    // we want the range from 0 to 1 (and 1 to zero) to last for the desired number of seconds
    // in order to do this, the deltaInSeconds tell us how long the last frame took to process
    // we need to take that value and break it up based on our incrementor.
    // so, if we want to measure how much of that 'time' to use in each second, we need to take
    // the reciprocal of the desired number of seconds.
    // eg: 1/4 means that in 1 second, we will have traveled .25 of the desired 'unit' of travel.
    // in this case, we want that 'desired' unit of travel to be time.
	g_clearColor[2] += static_cast<float>(incrementor * deltaInSeconds);
}

void Render(
    ID3D11Device* device, 
    ID3D11DeviceContext* context, 
    IDXGISwapChain* swapChain, 
    ID3D11RenderTargetView* renderTargetView)
{
    // Clear the back buffer to the clear color
	context->ClearRenderTargetView(renderTargetView, g_clearColor);

    // This could be cached, as we don't intend to resize the window.
    RECT winRect;
    GetClientRect(g_hWnd, &winRect);
    D3D11_VIEWPORT viewport =
    {
        0.0f, 0.0f,
        static_cast<float>(winRect.right - winRect.left),
        static_cast<float>(winRect.bottom - winRect.top),
        0.0f, 1.0f
    };

    g_D3DContext->RSSetViewports(1, &viewport);
    g_D3DContext->OMSetRenderTargets(1, &g_D3DRenderTargetView, nullptr);
    g_D3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    g_D3DContext->IASetInputLayout(g_inputLayout);

    g_D3DContext->VSSetShader(g_vertexShader, nullptr, 0);
    g_D3DContext->PSSetShader(g_pixelShader, nullptr, 0);

    g_D3DContext->IASetVertexBuffers(0, 1, &g_vertexBuffer, &g_stride, &g_offset);

    g_D3DContext->Draw(g_numVerts, 0);

	// Present the back buffer to the screen
	swapChain->Present(1, 0);
}

HRESULT CreateRenderTargetView(ID3D11Device* device, ID3D11Texture2D* backBuffer, ID3D11RenderTargetView** renderTargetView)
{
	HRESULT hr;

	// Create render target view
	hr = device->CreateRenderTargetView(backBuffer, NULL, renderTargetView);
	if (FAILED(hr))
	{
        OutputDebugStringA("Failed to create the Render Target view. Aborting\n");
		return hr;
	}

	return S_OK;
}

ID3D11Texture2D* GetBackBuffer(IDXGISwapChain* swapChain)
{
	ID3D11Texture2D* backBuffer = nullptr;
	HRESULT hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	if (FAILED(hr))
	{
		// Handle error
        OutputDebugStringA("Failed to get the Back buffer from the swapchain. Aborting.\n");
		return nullptr;
	}

	return backBuffer;
}