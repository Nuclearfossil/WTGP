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


#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment( lib, "dxguid.lib")
#pragma comment( lib, "dxgi.lib")

#define MAX_LOADSTRING 100

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
ID3D11Buffer* g_vertexBuffer = nullptr;         // The D3D11 Buffer used to hold the vertex data.
ID3D11Buffer* g_indexBuffer = nullptr;          // The D3D11 Index Buffer
ID3D11Buffer* g_mvpConstantBuffer = nullptr;    // The constant buffer for the WVP matrices
ID3D11DepthStencilView* g_depthBufferView;		// The Depth/Stencil view buffer
ID3D11DepthStencilState* g_depthStencilState;	// The Depth/Stencil State
ID3D11RasterizerState* g_rasterizerState;		// The Rasterizer State

DirectX::XMMATRIX g_mModel;						// The Model transform matrix
DirectX::XMMATRIX g_mView;						// The View (Camera) matrix
DirectX::XMMATRIX g_mProjection;				// the Perspecitve projection matrix

// Debug names for some of the D3D11 resources we'll be creating
const char c_vertexShaderID[] = "vertexShader";
const char c_pixelShaderID[] = "pixelShader";
const char c_inputLayoutID[] = "inputLayout";
const char c_vertexBufferID[] = "vertexBuffer";
const char c_constantBufferID[] = "constantBuffer";
const char c_indexBufferID[] = "indexBuffer";
const char c_depthStencilBufferID[] = "depthStencilBuffer";
const char c_rasterizerStateID[] = "rasterizerState";

float g_clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
double g_delta;

UINT g_numVerts = 0;
uint16_t g_numIndices = 0;

UINT g_stride = 0;
UINT g_offset = 0;

// This buffer will be used to pass data into the shader
struct ConstantBuffer
{
	DirectX::XMMATRIX mModelViewProjection;
};

// Forward declarations of functions included in this code module:

// Windows specific functions ------------------------------------------------
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

// D3D11 related functions ---------------------------------------------------
HRESULT CreateD3D11Context(ID3D11Device* device, ID3D11DeviceContext** context);
HRESULT CreateD3D11DeviceAndContext(HWND hWnd, UINT width, UINT height, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext, IDXGISwapChain** ppSwapChain);
HRESULT CreateRenderTargetView(ID3D11Device* device, ID3D11RenderTargetView** renderTargetView);
ID3D11Texture2D* GetBackBuffer(IDXGISwapChain* swapChain);
HRESULT CreateD3DResources();
void Update(double deltaInSeconds);
void Render(ID3D11Device* device, ID3D11DeviceContext* context, IDXGISwapChain* swapChain, ID3D11RenderTargetView* renderTargetView);

/// @brief Utility function to convert units in degrees to radians
/// @param degs
/// @return
inline float degreesToRadians(float degs)
{
	return degs * (DirectX::XM_PI / 180.0f);
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

	if (!SUCCEEDED(CreateD3D11DeviceAndContext(g_hWnd, 1024, 768, &g_D3DDevice, &g_D3DContext, &g_SwapChain)))
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

		// Let's throttle the application so that we render at a constant
		// speed, regardless of processor speed.
		Update(deltaSeconds);
		Render(g_D3DDevice, g_D3DContext, g_SwapChain, g_D3DRenderTargetView);
		lastStart = current;
	}

	// Release all our resources
	g_vertexShader->Release();
	g_pixelShader->Release();
	g_inputLayout->Release();
	g_vertexBuffer->Release();
	g_indexBuffer->Release();
	g_mvpConstantBuffer->Release();
	g_depthBufferView->Release();
	g_depthStencilState->Release();
	g_rasterizerState->Release();

	g_SwapChain->Release();
	g_D3DRenderTargetView->Release();
	g_D3DContext->Release();
	g_D3DDevice->Release();

	// Check to see if we've cleaned up all the D3D 11 resources.
	{
		IDXGIDebug1* debug = { 0 };
		DXGIGetDebugInterface1(0, IID_IDXGIDebug1, (void**)(&debug));
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}

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
		// 0,                          // Fourth Parameter - use D3D11_CREATE_DEVICE_DEBUG
		D3D11_CREATE_DEVICE_DEBUG,  // Fourth Parameter - use D3D11_CREATE_DEVICE_DEBUG
									// if you want additional debug
									// spew in the console.
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

/// @brief Create all D3D11 resources we will need for this application
/// @return S_OK if successful
HRESULT CreateD3DResources()
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

	g_vertexShader->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_vertexShaderID) - 1, c_vertexShaderID);

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
	g_pixelShader->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_pixelShaderID) - 1, c_pixelShaderID);

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
	g_inputLayout->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_inputLayoutID) - 1, c_inputLayoutID);

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
	//g_numVerts = sizeof(vertexData) / g_stride;
	g_numVerts = sizeof(indices) / sizeof(WORD);

	g_numIndices = sizeof(indices) / sizeof(indices[0]);

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
		&g_vertexBuffer)))          // Finally, the interface that is the vertex buffer
	{
		OutputDebugStringA("Failed to create the vertex buffer!");
		return S_FALSE;
	}

	g_vertexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_vertexBufferID) - 1, c_vertexBufferID);

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

	g_mvpConstantBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_constantBufferID) - 1, c_constantBufferID);

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.ByteWidth = sizeof(indices);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = indices;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	if (FAILED(g_D3DDevice->CreateBuffer(&indexBufferDesc, &initData, &g_indexBuffer)))
	{
		OutputDebugStringA("Failed to create a new index buffer.");
		return S_FALSE;
	}
	g_indexBuffer->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_indexBufferID) - 1, c_indexBufferID);

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	g_D3DDevice->CreateDepthStencilState(&depthStencilDesc, &g_depthStencilState);
	g_depthStencilState->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_depthStencilBufferID) - 1, c_depthStencilBufferID);

	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	// rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.CullMode = D3D11_CULL_FRONT;
	rasterizerDesc.FrontCounterClockwise = TRUE;

	g_D3DDevice->CreateRasterizerState(&rasterizerDesc, &g_rasterizerState);
	g_rasterizerState->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(c_rasterizerStateID) - 1, c_rasterizerStateID);


	return S_OK;
}

/// @brief Application Update function
/// @param deltaInSeconds time between frames in seconds
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

	static float increment = 0;
	g_mModel = DirectX::XMMatrixRotationY(increment) * DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	g_mView = DirectX::XMMatrixIdentity();
	g_mProjection = DirectX::XMMatrixIdentity();

	DirectX::XMVECTOR Eye = DirectX::XMVectorSet(0.0f, 0.0f, 2.0f, 1.0f);
	DirectX::XMVECTOR At = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	DirectX::XMVECTOR Up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	g_mView = DirectX::XMMatrixLookAtLH(Eye, At, Up);

	increment += 0.001f;

	// Initialize the projection matrix
	float width = (float)(g_winRect.right - g_winRect.left);
	float height = (float)(g_winRect.bottom - g_winRect.top);
	float aspect = width / height;
	g_mProjection = DirectX::XMMatrixPerspectiveFovLH(degreesToRadians(84), aspect, 0.01f, 100.0f);

	DirectX::XMMATRIX mvp = g_mModel * g_mView * g_mProjection;

	// Update constant buffer
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	g_D3DContext->Map(g_mvpConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	ConstantBuffer* constants = (ConstantBuffer*)(mappedSubresource.pData);
	constants->mModelViewProjection = DirectX::XMMatrixTranspose(mvp);
	g_D3DContext->Unmap(g_mvpConstantBuffer, 0);

	// Clear the back buffer to the clear color
	g_D3DContext->ClearRenderTargetView(renderTargetView, g_clearColor);
	g_D3DContext->ClearDepthStencilView(g_depthBufferView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	g_D3DContext->RSSetViewports(1, &viewport);
	g_D3DContext->RSSetState(g_rasterizerState);
	g_D3DContext->OMSetDepthStencilState(g_depthStencilState, 0);

	g_D3DContext->OMSetRenderTargets(1, &g_D3DRenderTargetView, nullptr);

	g_D3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_D3DContext->IASetInputLayout(g_inputLayout);

	g_D3DContext->VSSetShader(g_vertexShader, nullptr, 0);
	g_D3DContext->PSSetShader(g_pixelShader, nullptr, 0);
	g_D3DContext->VSSetConstantBuffers(0, 1, &g_mvpConstantBuffer);

	g_D3DContext->IASetVertexBuffers(0, 1, &g_vertexBuffer, &g_stride, &g_offset);
	g_D3DContext->IASetIndexBuffer(g_indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	g_D3DContext->DrawIndexed(g_numIndices, 0, 0);

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
	device->CreateTexture2D(&depthBufferDesc, nullptr, &depthBuffer);

	device->CreateDepthStencilView(depthBuffer, nullptr, &g_depthBufferView);

	depthBuffer->Release();

	return S_OK;
}

/// @brief Utility function for getting the Texture that represents the backbuffer
/// @param swapChain DXGI Swapchain to work from
/// @return a D3D11 Texture 2D to work with
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