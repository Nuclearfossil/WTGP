// 04_3DBasicApp.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "framework.h"
#include "01_WindowsApp.h"

#include <d3d11.h>
#include <directxmath.h>
#include <d3dcompiler.h>
#include <d3dcommon.h>
#include <dxgidebug.h>
#include <dxgi1_3.h>

#include <windowsx.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment( lib, "dxguid.lib")
#pragma comment( lib, "dxgi.lib")

#define MAX_LOADSTRING 1000

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND g_hWnd;                                    // Handle to the main window
RECT g_winRect;                                 // Window rectangle
IDXGISwapChain* g_SwapChain;                    // DXGI swapchain for double/triple buffering
WCHAR g_szTitle[MAX_LOADSTRING];                // The title bar text
WCHAR g_szWindowClass[MAX_LOADSTRING];          // The main window class name

ID3D11Device* g_D3DDevice;                      // The Direct3D Device
ID3D11DeviceContext* g_D3DContext;              // The Direct3D Device Context
ID3D11RenderTargetView* g_D3DRenderTargetView;  // The Render Target View

ID3D11VertexShader* g_vertexShader = nullptr;   // The Vertex Shader resource used in this example
ID3D11PixelShader* g_pixelShader = nullptr;     // The Pixel Shader resource used in this example
ID3D11InputLayout* g_inputLayout = nullptr;     // The Input layout resource used for the vertex shader

ID3D11Buffer* g_cubeVertexBuffer = nullptr;     // The D3D11 Buffer used to hold the vertex data for the cube.
ID3D11Buffer* g_cubeIndexBuffer = nullptr;      // The D3D11 Index Buffer for the cube

ID3D11Buffer* g_gridVertexBuffer = nullptr;		// The D3D11 Buffer used to hold the vertex data for the grid
ID3D11Buffer* g_gridIndexBuffer = nullptr;		// The D3D11 Index Buffer for the grid

ID3D11Buffer* g_mvpConstantBuffer = nullptr;    // The constant buffer for the WVP matrices
ID3D11DepthStencilView* g_depthBufferView;		// The Depth/Stencil view buffer
ID3D11DepthStencilState* g_depthStencilState;	// The Depth/Stencil State
ID3D11RasterizerState* g_rasterizerState;		// The Rasterizer State

DirectX::XMMATRIX g_mWorld;						// The Model transform matrix
DirectX::XMMATRIX g_mView;						// The View (Camera) matrix
DirectX::XMMATRIX g_mProjection;				// the Perspecitve projection matrix

float g_cameraRadius = 2.0f;						// the radius of the orbit camera

int g_lastX = 0;
int g_lastY = 0;

int g_deltaMouseX = 0;
int g_deltaMouseY = 0;


// Debug names for some of the D3D11 resources we'll be creating
#ifdef _DEBUG
const char c_vertexShaderID[] = "vertexShader";
const char c_pixelShaderID[] = "pixelShader";
const char c_inputLayoutID[] = "inputLayout";
const char c_vertexBufferID[] = "vertexBuffer";
const char c_gridVertexBufferID[] = "gridVertexBuffer";
const char c_gridIndexBufferID[] = "gridIndexBuffer";
const char c_constantBufferID[] = "constantBuffer";
const char c_indexBufferID[] = "indexBuffer";
const char c_depthStencilBufferID[] = "depthStencilBuffer";
const char c_rasterizerStateID[] = "rasterizerState";
#endif // DEBUG

float g_clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
double g_delta;

// Input Assembler and Vertex Buffer globals
UINT g_stride = 0;
UINT g_offset = 0;
UINT g_numCubeVerts = 0;
uint16_t g_numCubeIndices = 0;
UINT g_numGridVerts = 0;


uint16_t g_numGridIndices = 0;

float g_increment = 0;

/// @brief Structure defining the Constant buffer. This buffer will be used to pass data into the shader
struct ConstantBuffer
{
	DirectX::XMMATRIX mModelViewProjection;
};

// [BEGIN] - Forward declarations of functions: ==============================================================================================

// Windows specific functions ------------------------------------------------
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

// D3D11 related functions ---------------------------------------------------
HRESULT CreateD3D11Context(ID3D11Device* device, ID3D11DeviceContext** context);
HRESULT CreateD3D11DeviceAndContext(HWND hWnd, UINT width, UINT height);
HRESULT CreateRenderTargetView(ID3D11Device* device, ID3D11RenderTargetView** renderTargetView);
ID3D11Texture2D* GetBackBuffer(IDXGISwapChain* swapChain);
HRESULT CreateD3DResources();
HRESULT LoadAndCompileShaders();
HRESULT CreateVertexAndIndexBuffers();
HRESULT CreateDepthStencilAndRasterizerState();
void Render(ID3D11Device* device, ID3D11DeviceContext* context, IDXGISwapChain* swapChain, ID3D11RenderTargetView* renderTargetView);

// Additional functions
void Update(double deltaInSeconds);
// [END] - Forward declarations of functions: =============================================================================================

/// @brief Utility function to convert units in degrees to radians
/// @param degs Degrees to convert to radians
/// @return radians
constexpr float degreesToRadians(float degs)
{
	return degs * (DirectX::XM_PI / 180.0f);
}

/// @brief Utility function to clamp float values
/// @param in input value
/// @param low low range to clamp to
/// @param high high range to clamp to
/// @return clamped value
constexpr float clamp(const float in, const float low, const float high)
{
    return in < low ? low :
		in > high ? high :
		in;
}

/// @brief Utility function for getting the Texture that represents the backbuffer
/// @param swapChain DXGI Swapchain to work from
/// @return a D3D11 Texture 2D to work with
ID3D11Texture2D* GetBackBuffer(IDXGISwapChain* swapChain)
{
    ID3D11Texture2D* backBuffer = nullptr;
    HRESULT hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
    if (FAILED(hr)) {
        // Handle error
        OutputDebugStringA("Failed to get the Back buffer from the swapchain. Aborting.\n");
        return nullptr;
    }

    return backBuffer;
}

/// @brief Windows Main entry point
/// @param hInstance handle to the application instance
/// @param hPrevInstance Previous instance, if there is one
/// @param lpCmdLine Pointer to the command line
/// @param nCmdShow Ignore
/// @return application result - 0 if we terminate successfully.
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
	if (!InitInstance(hInstance, nCmdShow))
		return -1;

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY01WINDOWSAPP));

	if (!SUCCEEDED(CreateD3D11DeviceAndContext(g_hWnd, 1024, 768)))
		return -2;

	if (!SUCCEEDED(CreateRenderTargetView(g_D3DDevice, &g_D3DRenderTargetView)))
		return -3;

	if (!SUCCEEDED(CreateD3DResources()))
		return -4;

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

		// Let's throttle the application so that we render at a constant speed, regardless of processor speed.
		Update(deltaSeconds);
		Render(g_D3DDevice, g_D3DContext, g_SwapChain, g_D3DRenderTargetView);
		lastStart = current;
	}

	// Release all our resources
	g_vertexShader->Release();
	g_pixelShader->Release();
	g_inputLayout->Release();
	g_cubeVertexBuffer->Release();
	g_cubeIndexBuffer->Release();
    g_gridVertexBuffer->Release();
    g_gridIndexBuffer->Release();
	g_mvpConstantBuffer->Release();
	g_depthBufferView->Release();
	g_depthStencilState->Release();
	g_rasterizerState->Release();

	g_SwapChain->Release();
	g_D3DRenderTargetView->Release();
	g_D3DContext->Release();
	g_D3DDevice->Release();

#ifdef _DEBUG
	// Check to see if we've cleaned up all the D3D 11 resources.
	{
		IDXGIDebug1* debug = { 0 };
		DXGIGetDebugInterface1(0, IID_IDXGIDebug1, (void**)(&debug));
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}
#endif // DEBUG

	return (int)msg.wParam;
}

/// @brief Standard windows class initialization
/// @param hInstance application instance handle
/// @return true if we are able to register this application class
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY01WINDOWSAPP));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MY01WINDOWSAPP);
	wcex.lpszClassName = g_szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

/// @brief Initialize the application Instance
/// @param hInstance application instance handle
/// @param nCmdShow Ignore
/// @return true if we are able to initialize the instance of this application
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	RECT windowSize = { 0, 0, 1024, 768 };
	AdjustWindowRect(&windowSize, WS_OVERLAPPEDWINDOW, FALSE);

	HWND hWnd = CreateWindowW(g_szWindowClass,
		g_szTitle,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		0, 0, windowSize.right - windowSize.left, windowSize.bottom - windowSize.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	if (!hWnd)
		return FALSE;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	g_hWnd = hWnd;

	return TRUE;
}

/// @brief Windows Callback procedure
/// @param hWnd Window Handle
/// @param message
/// @param wParam
/// @param lParam
/// @return
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
    case WM_MOUSEMOVE:
	{
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);

        if (wParam == MK_LBUTTON)
		{
            g_deltaMouseX += g_lastX - xPos;
            g_deltaMouseY += g_lastY - yPos;
        }

		g_lastX = xPos;
        g_lastY = yPos;
    }
	break;
    case WM_MOUSEWHEEL:
	{
        float modifier = 0.01f;
        if (MK_SHIFT == GET_KEYSTATE_WPARAM(wParam))
            modifier = 0.001f;

        float wheel = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) * modifier;
        g_cameraRadius += wheel;
	}
    break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
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

/// @brief Create the D3D11 Device and Device Context
/// @param hWnd Handle to the window
/// @param width Window Width
/// @param height Window Height
/// @param ppDevice Reference to the D3D11 device to populate
/// @param ppContext Reference to the D3D11 Context to populate
/// @param ppSwapChain Reference to the DXGI swapchain to populate
/// @return S_OK if we're successfully created the context and device
HRESULT CreateD3D11DeviceAndContext(HWND hWnd,
	UINT width,
	UINT height)
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
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Flags = 0;

	// Create device, context, and swap chain
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
	hr = D3D11CreateDeviceAndSwapChain(
		nullptr,                    // First Parameter
		D3D_DRIVER_TYPE_HARDWARE,   // Second Parameter
		nullptr,                    // Third Parameter
#ifdef _DEBUG
		D3D11_CREATE_DEVICE_DEBUG,  // Fourth Parameter - use D3D11_CREATE_DEVICE_DEBUG
									// if you want additional debug spew in the console.
#else
		0,                          // Fourth Parameter
#endif // DEBUG
		&featureLevel,              // Fifth Parameter
		1,                          // Sixth Parameter
		D3D11_SDK_VERSION,          // Seventh Parameter
		&swapChainDesc,             // Eighth Parameter
		&g_SwapChain,                // Ninth Parameter
		&g_D3DDevice,                   // Tenth Parameter
		nullptr,                    // Eleventh Parameter
		&g_D3DContext);                 // Twelfth Parameter

	if (FAILED(hr))
	{
		// TODO: Add logging as to why this failed.
		return hr;
	}

	return S_OK;
}

/// @brief Create the D3D11 Context
/// @param device D3D11 Device to use for creating the D3D11 Context from
/// @param context Reference to the context to populate
/// @return
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

/// @brief Load the Shader from file and compile the Vertex and Pixel shaders
/// @return S_OK if we were able to compile the shaders
HRESULT LoadAndCompileShaders()
{
	// creation of Shader Resources
	ID3DBlob* vsBlob;
	ID3DBlob* psBlob;
	ID3DBlob* shaderCompileErrorBlob;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows
	// the shaders to be optimized and to run exactly the way they will run in
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	// We compile the Vertex shader from the `vertexShaderSource` source string and check for validity
	if (!SUCCEEDED(D3DCompileFromFile(L"CombinedShader.hlsl", // Shader File
		nullptr,                        // Optional array of D3D_SHADER_MARCO defining macros used in compilation
		nullptr,                        // optional pointer to an ID3DInclude that defines how the compiler handles include files
		"vs_main",                      // Entry-point of the shader
		"vs_5_0",                       // String that specifies what the shader target is.
		dwShaderFlags,                  // Any flags that drive D3D compile constants. Things like `D3DCOMPILE_DEBUG`
		0,                              // Any flags for compiler effect constants. For now we can ignore
		&vsBlob,                        // An interface to the compiled shader
		&shaderCompileErrorBlob)))      // An interface to any errors from the compile process.
	{
		OutputDebugStringA(static_cast<const char*>(shaderCompileErrorBlob->GetBufferPointer()));
		shaderCompileErrorBlob->Release();
		return S_FALSE;
	}

	// We then create the appropriate Vertex Shader resource: `g_vertexShader`.
	if (!SUCCEEDED(g_D3DDevice->CreateVertexShader(
		vsBlob->GetBufferPointer(),     // A pointer to the compiled shader.
		vsBlob->GetBufferSize(),        // And the size of the compiled shader.
		nullptr,                        // A pointer to the Class Linkage (for now, let's use null).
		&g_vertexShader)))              // Address of the ID3D11VertexShader interface.
	{
		OutputDebugStringA("Failed to create the Vertex Shader!\n");
		return S_FALSE;
	}

#ifdef _DEBUG
	g_vertexShader->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_vertexShaderID) - 1, c_vertexShaderID);
#endif // DEBUG


	// We compile the Pixel shader from the `pixelShaderSource` source string and check for validity
	if (!SUCCEEDED(D3DCompileFromFile(L"CombinedShader.hlsl",
		nullptr,
		nullptr,
		"ps_main",
		"ps_5_0",
		0,
		0,
		&psBlob,
		&shaderCompileErrorBlob)))
	{
		OutputDebugStringA(static_cast<const char*>(shaderCompileErrorBlob->GetBufferPointer()));
		shaderCompileErrorBlob->Release();
		return S_FALSE;
	}

	// We then create the appropriate Pixel Shader resource: `g_pixelShader`
	if (!SUCCEEDED(g_D3DDevice->CreatePixelShader(
		psBlob->GetBufferPointer(),
		psBlob->GetBufferSize(),
		nullptr,
		&g_pixelShader)))
	{
		OutputDebugStringA("Failed to create the Pixel Shader\n");
		return S_FALSE;
	}

	psBlob->Release();

#ifdef _DEBUG
	g_pixelShader->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_pixelShaderID) - 1, c_pixelShaderID);
#endif // DEBUG

	// Create Input Layout - this describes the format of the vertex data we will use.
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	if (!SUCCEEDED(g_D3DDevice->CreateInputLayout(
		inputElementDesc,               // An array of D3D11_INPUT_ELEMENT_DESC describing the vertex data
		ARRAYSIZE(inputElementDesc),    // How big is the array
		vsBlob->GetBufferPointer(),     // The compiled vertex shader
		vsBlob->GetBufferSize(),        // And the size of the vertex shader
		&g_inputLayout)))               // The resultant input layout
	{
		OutputDebugStringA("Failed to create the Input Layout");
		return S_FALSE;
	}

	vsBlob->Release();

#ifdef _DEBUG
	g_inputLayout->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_inputLayoutID) - 1, c_inputLayoutID);
#endif // DEBUG
    return S_OK;
}

/// @brief Create the Vertex and Index buffers, as well as the input layout
/// @return S_OK if we are able to create the buffers and input layout
HRESULT CreateVertexAndIndexBuffers()
{
	// Populate the array representing the vertex data. In this case, we are going to have
	// 6 elements per vertex:
	//   - X and Y co-ordinates
	//   - Colours for each vertex representing the Red, Green, Blue and Alpha channels.
	float vertexData[] =
	{
		//    x,     y,     z,    r,    g,    b,    a
		  -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f,
		  -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
		  -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
		  -0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
		   0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f,
		   0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,
		   0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f,
		   0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f, 1.0f,
	};

	float gridVertexData[]
	{
    //      x,    y,     z,    r,    g,    b,    a
		// columns
		-5.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		-5.0f, 0.0f,  5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		-4.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		-4.0f, 0.0f,  5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		-3.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		-3.0f, 0.0f,  5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		-2.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		-2.0f, 0.0f,  5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		-1.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		-1.0f, 0.0f,  5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		 0.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		 0.0f, 0.0f,  5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		 1.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		 1.0f, 0.0f,  5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		 2.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		 2.0f, 0.0f,  5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		 3.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		 3.0f, 0.0f,  5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		 4.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		 4.0f, 0.0f,  5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		 5.0f, 0.0f, -5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		 5.0f, 0.0f,  5.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		 // rows
		-5.0f, 0.0f, -5.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		 5.0f, 0.0f, -5.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-5.0f, 0.0f, -4.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		 5.0f, 0.0f, -4.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-5.0f, 0.0f, -3.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		 5.0f, 0.0f, -3.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-5.0f, 0.0f, -2.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		 5.0f, 0.0f, -2.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-5.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		 5.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-5.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		 5.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-5.0f, 0.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		 5.0f, 0.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-5.0f, 0.0f,  2.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		 5.0f, 0.0f,  2.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-5.0f, 0.0f,  3.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		 5.0f, 0.0f,  3.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-5.0f, 0.0f,  4.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		 5.0f, 0.0f,  4.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		-5.0f, 0.0f,  5.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		 5.0f, 0.0f,  5.0f, 1.0f, 0.0f, 0.0f, 1.0f
	};

	WORD gridIndices[] =
	{
		0, 1,
		2, 3,
		4, 5,
		6, 7,
		8, 9,
		10, 11,
		12, 13,
		14, 15,
		16, 17,
		18, 19,
		20, 21,
		22, 23,
		24, 25,
		26, 27,
		28, 29,
		30, 31,
		32, 33,
		34, 35,
		36, 37,
		38, 39,
		40, 41,
		42, 43
	};

	// Create index buffer
	WORD indices[] =
	{
		0, 6, 4,
		0, 2, 6,

		0, 3, 2,
		0, 1, 3,

		2, 7, 6,
		2, 3, 7,

		4, 6, 7,
		4, 7, 5,

		0, 4, 5,
		0, 5, 1,

		1, 5, 7,
		1, 7, 3
	};

	// The stride represents the _actual_ width of the data for each vertex.
	// In this case, we now have 7 elements per vertex. Three for the position, four for the colour.
	g_stride = 3 * sizeof(float) + 4 * sizeof(float);
	g_offset = 0;
	g_numCubeVerts = sizeof(indices) / sizeof(WORD);

	g_numCubeIndices = sizeof(indices) / sizeof(indices[0]);

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.ByteWidth = sizeof(vertexData);
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexSubresourceData;
	vertexSubresourceData.pSysMem = vertexData;
	vertexSubresourceData.SysMemPitch = 0;
	vertexSubresourceData.SysMemSlicePitch = 0;

	if (!SUCCEEDED(g_D3DDevice->CreateBuffer(
		&vertexBufferDesc,          // The Vertex buffer description
		&vertexSubresourceData,     // And then the sub-resource data
		&g_cubeVertexBuffer)))          // Finally, the interface that is the vertex buffer
	{
		OutputDebugStringA("Failed to create the vertex buffer!");
		return S_FALSE;
	}

#ifdef _DEBUG
	g_cubeVertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_vertexBufferID) - 1, c_vertexBufferID);
#endif // DEBUG

	g_numGridVerts = sizeof(gridIndices) / sizeof(WORD);
    g_numGridIndices = sizeof(gridIndices) / sizeof(gridIndices[0]);

	D3D11_BUFFER_DESC gridVertexBufferDesc = {};
    gridVertexBufferDesc.ByteWidth = sizeof(gridVertexData);
    gridVertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    gridVertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA gridVertexSubresourceData = {};
    gridVertexSubresourceData.pSysMem = gridVertexData;
    gridVertexSubresourceData.SysMemPitch = 0;
    gridVertexSubresourceData.SysMemSlicePitch = 0;

	if (!SUCCEEDED(g_D3DDevice->CreateBuffer(
		&gridVertexBufferDesc,
		&gridVertexSubresourceData,
		&g_gridVertexBuffer)))
	{
        OutputDebugStringA("Failed to create the Grid vertex buffer!");
        return S_FALSE;
    }

#ifdef _DEBUG
    g_gridVertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_gridVertexBufferID) - 1, c_gridVertexBufferID);
#endif // _DEBUG


	D3D11_BUFFER_DESC constantBufferDesc = {};
	constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(g_D3DDevice->CreateBuffer(&constantBufferDesc, nullptr, &g_mvpConstantBuffer)))
	{
		OutputDebugStringA("Failed to create a new constant buffer.");
		return S_FALSE;
	}

#ifdef _DEBUG
	g_mvpConstantBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_constantBufferID) - 1, c_constantBufferID);
#endif // DEBUG


	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.ByteWidth = sizeof(indices);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = indices;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	if (FAILED(g_D3DDevice->CreateBuffer(&indexBufferDesc, &initData, &g_cubeIndexBuffer)))
	{
		OutputDebugStringA("Failed to create a new index buffer.");
		return S_FALSE;
	}

#ifdef _DEBUG
	g_cubeIndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_indexBufferID) - 1, c_indexBufferID);
#endif // DEBUG

	D3D11_BUFFER_DESC gridIndexBufferDesc = {};
    gridIndexBufferDesc.ByteWidth = sizeof(gridIndices);
    gridIndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    gridIndexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    gridIndexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA gridInitData = {};
    gridInitData.pSysMem = gridIndices;
    gridInitData.SysMemPitch = 0;
    gridInitData.SysMemSlicePitch = 0;

	if (FAILED(g_D3DDevice->CreateBuffer(&gridIndexBufferDesc, &gridInitData, &g_gridIndexBuffer)))
	{
        OutputDebugStringA("Failed to create a new grid index buffer");
        return S_FALSE;
	}

#ifdef _DEBUG
    g_gridIndexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_gridIndexBufferID) - 1, c_gridIndexBufferID);
#endif

	return S_OK;
}

/// @brief Create the depth/stencil buffer as well as the rasterizer state objects
/// @return S_OK if we are able to create these objects
HRESULT CreateDepthStencilAndRasterizerState()
{
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	g_D3DDevice->CreateDepthStencilState(&depthStencilDesc, &g_depthStencilState);

#ifdef _DEBUG
	g_depthStencilState->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_depthStencilBufferID) - 1, c_depthStencilBufferID);
#endif // DEBUG

	D3D11_RASTERIZER_DESC rasterizerDesc = {};

	rasterizerDesc.FrontCounterClockwise = FALSE;

	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	// rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;

	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	// rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	//rasterizerDesc.CullMode = D3D11_CULL_NONE;

	g_D3DDevice->CreateRasterizerState(&rasterizerDesc, &g_rasterizerState);

#ifdef _DEBUG
	g_rasterizerState->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_rasterizerStateID) - 1, c_rasterizerStateID);
#endif // DEBUG

	return S_OK;
}

/// @brief Create all D3D11 resources we will need for this application
/// @return S_OK if successful
HRESULT CreateD3DResources()
{
	if (!SUCCEEDED(LoadAndCompileShaders())) return S_FALSE;

	if (!SUCCEEDED(CreateVertexAndIndexBuffers())) return S_FALSE;

	if (!SUCCEEDED(CreateDepthStencilAndRasterizerState())) return S_FALSE;

	return S_OK;
}

/// @brief Application Update function
/// @param deltaInSeconds time between frames in seconds
void Update(double deltaInSeconds)
{
	static double desiredTime = 1.0f / 4.0f; // 1 second / desired number of seconds
	static double incrementor = desiredTime;

	/// smoothly rotate our object
	g_increment += 0.1f * static_cast<float>(deltaInSeconds);
	if (g_increment > 360.0f) g_increment -= 360.0f;
}

/// @brief Render off a frame
/// @param device D3D11 Device
/// @param context D3D11 Device Context
/// @param swapChain DXGI Swapchain
/// @param renderTargetView D3D11 Render Target view
void Render(
	ID3D11Device* device,
	ID3D11DeviceContext* context,
	IDXGISwapChain* swapChain,
	ID3D11RenderTargetView* renderTargetView)
{
	// This could be cached, as we don't intend to resize the window.
	GetClientRect(g_hWnd, &g_winRect);
	D3D11_VIEWPORT viewport =
	{
		0.0f, 0.0f,
		static_cast<float>(g_winRect.right - g_winRect.left),
		static_cast<float>(g_winRect.bottom - g_winRect.top),
		0.0f, 1.0f
	};

	g_mWorld = DirectX::XMMatrixIdentity();
	g_mView = DirectX::XMMatrixIdentity();
	g_mProjection = DirectX::XMMatrixIdentity();

    float polar = degreesToRadians(static_cast<float>(g_deltaMouseX)), azimuth = degreesToRadians(static_cast<float>(g_deltaMouseY));
    azimuth = clamp(azimuth, -DirectX::XM_PIDIV2, DirectX::XM_PIDIV2);
    g_cameraRadius = clamp(g_cameraRadius, 1.0f, 10.0f);

	float y = g_cameraRadius * sinf(azimuth);
    float r = g_cameraRadius * cosf(azimuth);
    float x = r * cosf(polar);
    float z = r * sinf(polar);


    DirectX::XMVECTOR Eye = DirectX::XMVectorSet(x, y, z, 1.0f);
	DirectX::XMVECTOR At = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	DirectX::XMVECTOR Up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	g_mView = DirectX::XMMatrixLookAtLH(Eye, At, Up);

	// Initialize the projection matrix
	float width = static_cast<float>(g_winRect.right - g_winRect.left);
	float height = static_cast<float>(g_winRect.bottom - g_winRect.top);
	float aspect = width / height;
	g_mProjection = DirectX::XMMatrixPerspectiveFovLH(degreesToRadians(78), aspect, 0.01f, 100.0f);

	// Update constant buffer
    {
        DirectX::XMMATRIX mvp = g_mWorld * g_mView * g_mProjection;
        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        g_D3DContext->Map(g_mvpConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
        ConstantBuffer* constants = (ConstantBuffer*)(mappedSubresource.pData);
        constants->mModelViewProjection = DirectX::XMMatrixTranspose(mvp);
        g_D3DContext->Unmap(g_mvpConstantBuffer, 0);
    }
	// Clear the back buffer to the clear color
	g_D3DContext->ClearRenderTargetView(renderTargetView, g_clearColor);
	g_D3DContext->ClearDepthStencilView(g_depthBufferView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	g_D3DContext->RSSetViewports(1, &viewport);
	g_D3DContext->RSSetState(g_rasterizerState);
	g_D3DContext->OMSetDepthStencilState(g_depthStencilState, 0);

	g_D3DContext->OMSetRenderTargets(1, &g_D3DRenderTargetView, nullptr);

	g_D3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
    g_D3DContext->IASetInputLayout(g_inputLayout);

	g_D3DContext->VSSetShader(g_vertexShader, nullptr, 0);
    g_D3DContext->PSSetShader(g_pixelShader, nullptr, 0);
    g_D3DContext->VSSetConstantBuffers(0, 1, &g_mvpConstantBuffer);

    g_D3DContext->IASetVertexBuffers(0, 1, &g_gridVertexBuffer, &g_stride, &g_offset);
    g_D3DContext->IASetIndexBuffer(g_gridIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    g_D3DContext->DrawIndexed(g_numGridIndices, 0, 0);

	g_mWorld = DirectX::XMMatrixRotationY(g_increment) * DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
    {
        DirectX::XMMATRIX mvp = g_mWorld * g_mView * g_mProjection;
        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        g_D3DContext->Map(g_mvpConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
        ConstantBuffer* constants = (ConstantBuffer*)(mappedSubresource.pData);
        constants->mModelViewProjection = DirectX::XMMatrixTranspose(mvp);
        g_D3DContext->Unmap(g_mvpConstantBuffer, 0);
	}

	g_D3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    g_D3DContext->IASetInputLayout(g_inputLayout);

	g_D3DContext->VSSetShader(g_vertexShader, nullptr, 0);
	g_D3DContext->PSSetShader(g_pixelShader, nullptr, 0);
	g_D3DContext->VSSetConstantBuffers(0, 1, &g_mvpConstantBuffer);

	g_D3DContext->IASetVertexBuffers(0, 1, &g_cubeVertexBuffer, &g_stride, &g_offset);
	g_D3DContext->IASetIndexBuffer(g_cubeIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	g_D3DContext->DrawIndexed(g_numCubeIndices, 0, 0);

	// Present the back buffer to the screen
	swapChain->Present(1, 0);

	g_D3DContext->ClearState();
	g_D3DContext->Flush();
}

/// @brief Create the Render Target view from the backbuffer
/// @param device D3D11 Device
/// @param renderTargetView Reference to the D3D11 Render Target View
/// @return S_OK if successful
HRESULT CreateRenderTargetView(ID3D11Device* device, ID3D11RenderTargetView** renderTargetView)
{
	HRESULT hr = S_OK;

	auto backBuffer = GetBackBuffer(g_SwapChain);

	// Create render target view
	hr = device->CreateRenderTargetView(backBuffer, NULL, renderTargetView);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create the Render Target view. Aborting\n");
		return hr;
	}

	D3D11_TEXTURE2D_DESC depthBufferDesc;
	backBuffer->GetDesc(&depthBufferDesc);

	backBuffer->Release();

	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	ID3D11Texture2D* depthBuffer;
	if (!SUCCEEDED(device->CreateTexture2D(&depthBufferDesc, nullptr, &depthBuffer)))
		return S_FALSE;
	if (depthBuffer == nullptr)
		return S_FALSE;

	device->CreateDepthStencilView(depthBuffer, nullptr, &g_depthBufferView);

	depthBuffer->Release();

	return S_OK;
}
