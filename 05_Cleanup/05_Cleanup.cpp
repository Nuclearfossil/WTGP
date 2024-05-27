// 04_3DBasicApp.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "framework.h"
#include "01_WindowsApp.h"

#include "GraphicsDX11.h"

#include <windowsx.h>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include "Camera3D.h"
#include "mathutils.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment( lib, "dxguid.lib")
#pragma comment( lib, "dxgi.lib")

#define MAX_LOADSTRING 1000

// Global Variables:
HINSTANCE hInst;                                // current instance
HWND g_hWnd;                                    // Handle to the main window
RECT g_winRect;                                 // Window rectangle
WCHAR g_szTitle[MAX_LOADSTRING];                // The title bar text
WCHAR g_szWindowClass[MAX_LOADSTRING];          // The main window class name


Camera3D g_Camera;								// Camera

int g_lastX = 0;
int g_lastY = 0;

int g_deltaMouseX = 0;
int g_deltaMouseY = 0;

double g_delta;

bool g_InvertYAxis = false;

float g_increment = 0;

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// [BEGIN] - Forward declarations of functions: ==============================================================================================

// Windows specific functions ------------------------------------------------
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);


HRESULT InitIMGUI(GraphicsDX11& graphics);
void DestroyIMGUI();

void DrawUI();

// Additional functions
void Update(double deltaInSeconds, GraphicsDX11& graphics);

// [END] - Forward declarations of functions: =============================================================================================
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

	GraphicsDX11 graphicsDX11;

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, g_szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_MY01WINDOWSAPP, g_szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
		return -1;

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY01WINDOWSAPP));

	g_Camera.Initialize();

	if (!SUCCEEDED(graphicsDX11.CreateD3D11DeviceAndContext(g_hWnd, 1024, 768)))
		return -2;

	if (!SUCCEEDED(graphicsDX11.CreateRenderTargetView()))
		return -3;

	if (!SUCCEEDED(graphicsDX11.CreateD3DResources()))
		return -4;

    if (!SUCCEEDED(InitIMGUI(graphicsDX11)))
        return -5;

	// Main message loop:
	MSG msg = { 0 };

	LARGE_INTEGER current = { 0 };
	LARGE_INTEGER lastStart = { 0 };
	LARGE_INTEGER frequency = { 0 };

	::QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&lastStart);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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
        Update(deltaSeconds, graphicsDX11);

		DrawUI();

		g_Camera.SetInvertY(g_InvertYAxis);

		graphicsDX11.Render(g_hWnd, g_winRect, deltaSeconds);
		lastStart = current;
	}

	DestroyIMGUI();

	graphicsDX11.Cleanup();

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
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

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
        if (ImGui::GetIO().WantCaptureMouse)
            break;

		int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);

        if (wParam == MK_LBUTTON)
		{
            g_deltaMouseX += g_lastX - xPos;
            g_deltaMouseY += g_InvertYAxis ? (g_lastY - yPos) : -(g_lastY - yPos);
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
        g_Camera.ChangeRadius(wheel);
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
/// @brief Initialize Everything relating to ImGui
/// @return S_OK if successful
HRESULT InitIMGUI(GraphicsDX11& graphics)
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
    ImGui_ImplWin32_Init(g_hWnd);
    ImGui_ImplDX11_Init(graphics.GetD3DDevice(), graphics.GetD3DDeviceContext());

    return S_OK;
}

/// @brief Draw our UI
void DrawUI()
{
	// Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();

    ImGui::Begin("App Settings");

    ImGui::Checkbox("Invert Y Axis", &g_InvertYAxis); // Edit if we want to invert the Y axis

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

/// @brief Application Update function
/// @param deltaInSeconds time between frames in seconds
void Update(double deltaInSeconds, GraphicsDX11& graphics)
{
    // This could be cached, as we don't intend to resize the window.
    RECT winRect;
    GetClientRect(g_hWnd, &winRect);

    D3D11_VIEWPORT viewport = {
        0.0f, 0.0f,
        static_cast<float>(winRect.right - winRect.left),
        static_cast<float>(winRect.bottom - winRect.top),
        0.0f, 1.0f
    };

    // Initialize the projection matrix
    float width = static_cast<float>(winRect.right - winRect.left);
    float height = static_cast<float>(winRect.bottom - winRect.top);
    float aspect = width / height;

    g_Camera.SetProjection(width, height, aspect);

	static double desiredTime = 1.0f / 4.0f; // 1 second / desired number of seconds
	static double incrementor = desiredTime;

	/// smoothly rotate our object
	g_increment += 0.1f * static_cast<float>(deltaInSeconds);
	if (g_increment > 360.0f) g_increment -= 360.0f;

	float polar = degreesToRadians(static_cast<float>(g_deltaMouseX)), azimuth = degreesToRadians(static_cast<float>(g_deltaMouseY));
    azimuth = clamp(azimuth, -DirectX::XM_PIDIV2, DirectX::XM_PIDIV2);
    DirectX::XMVECTOR At = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

    g_Camera.RotateAroundPoint(At, polar, azimuth);

	g_Camera.Update(deltaInSeconds);

	graphics.SetViewport(viewport);
	graphics.SetMVP(g_Camera.GetMVP());
    graphics.SetVP(g_Camera.GetVP());
}

