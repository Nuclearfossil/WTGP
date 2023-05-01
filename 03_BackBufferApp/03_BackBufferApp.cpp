// 01_WindowsApp.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "framework.h"
#include "01_WindowsApp.h"
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND g_hWnd;
IDXGISwapChain* g_SwapChain;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

ID3D11Device* g_D3DDevice;
ID3D11DeviceContext* g_D3DContext;
ID3D11RenderTargetView* g_D3DRenderTargetView;
float g_clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

HRESULT CreateD3D11Context(ID3D11Device* device, ID3D11DeviceContext** context);
HRESULT CreateD3D11DeviceAndContext(HWND hWnd, UINT width, UINT height, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext, IDXGISwapChain** ppSwapChain);
HRESULT CreateRenderTargetView(ID3D11Device* device, ID3D11Texture2D* backBuffer, ID3D11RenderTargetView** renderTargetView);
ID3D11Texture2D* GetBackBuffer(IDXGISwapChain* swapChain);
void Render(ID3D11Device* device, ID3D11DeviceContext* context, IDXGISwapChain* swapChain, ID3D11RenderTargetView* renderTargetView);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY01WINDOWSAPP, szWindowClass, MAX_LOADSTRING);
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

    // Main message loop:
    MSG msg = { 0 };

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
            {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
            }
        }

        Render(g_D3DDevice, g_D3DContext, g_SwapChain, g_D3DRenderTargetView);
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
    wcex.lpszClassName  = szWindowClass;
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

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
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

void Render(
    ID3D11Device* device, 
    ID3D11DeviceContext* context, 
    IDXGISwapChain* swapChain, 
    ID3D11RenderTargetView* renderTargetView)
{
    static float incrementor = 0.01f;
	if (g_clearColor[2] > 1.0f)
        incrementor = -0.01f;

    if (g_clearColor[2] < 0.0f)
        incrementor = 0.01f;

    g_clearColor[2] += incrementor;
    
	// Clear the back buffer to the clear color
	context->ClearRenderTargetView(renderTargetView, g_clearColor);

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
		return nullptr;
	}

	return backBuffer;
}