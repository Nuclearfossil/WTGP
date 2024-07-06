// 04_3DBasicApp.cpp : Defines the entry point for the application.
//

#include <windowsx.h>

#include "pch.h"
#include "Game.h"
#include "framework.h"
#include "01_WindowsApp.h"

#include "GraphicsDX11.h"

#include "OrbitCamera.h"
#include "UserInterface.h"
#include "mathutils.h"

#include "ResourceManager.h"

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

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

// [BEGIN] - Forward declarations of functions: ==============================================================================================

// Windows specific functions ------------------------------------------------
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int, GameData* gameDataPtr);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

void DrawUI(GameData& data);

// Additional functions
void Update(double deltaInSeconds, GraphicsDX11& graphics, OrbitCamera& camera, GameData& data);

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

	static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
    static plog::DebugOutputAppender<plog::TxtFormatter> debugConsoleAppender;
	plog::init(plog::debug, "05_cleanup.log").addAppender(&consoleAppender).addAppender(&debugConsoleAppender); // Initializing Logging

	PLOG_INFO << "=================================================== Beginning of Run ===================================================";

	GameData data;	// data related to the application

    OrbitCamera camera;			// Camera
	GraphicsDX11 graphicsDX11;  // Graphics system

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, g_szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_MY01WINDOWSAPP, g_szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow, &data))
		return -1;

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY01WINDOWSAPP));

	camera.Initialize();

	if (!SUCCEEDED(graphicsDX11.CreateD3D11DeviceAndContext(g_hWnd, 1024, 768)))
		return -2;

	if (!SUCCEEDED(graphicsDX11.CreateRenderTargetView()))
		return -3;

	if (!SUCCEEDED(graphicsDX11.CreateD3DResources()))
		return -4;

    if (!SUCCEEDED(InitIMGUI(g_hWnd, graphicsDX11)))
        return -5;

	if (!SUCCEEDED(InitResources(graphicsDX11.GetD3DDeviceContext())))
        return -6;

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
        Update(deltaSeconds, graphicsDX11, camera, data);

		DrawUI(data);

		camera.SetInvertY(data.m_InvertYAxis);

		graphicsDX11.Render(g_hWnd, g_winRect, data, deltaSeconds);
		lastStart = current;
	}

	DestroyIMGUI();

    ClearResources();

	graphicsDX11.Cleanup();

    PLOG_INFO << "====================================================== End of Run ======================================================";

#ifdef _DEBUG
	IDXGIDebug* debugDev;
    DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debugDev));

	debugDev->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
#endif // DEBUG


	return (int)msg.wParam;
}

/// @brief Standard windows class initialization
/// @param hInstance application instance handle
/// @return true if we are able to register this application class
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    PLOG_INFO << "Windows Class Registration";

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
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow,  GameData* gameDataPtr)
{
    PLOG_INFO << "Initializing the Application instance";
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
        gameDataPtr);

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
    if (HandleWindowsMessages(hWnd, message, wParam, lParam))
        return true;

	switch (message)
	{
    case WM_CREATE:
		{
		/// A little trick to avoid using a singleton: https://learn.microsoft.com/en-us/windows/win32/learnwin32/managing-application-state-
			CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
			GameData* pState = reinterpret_cast<GameData*>(pCreate->lpCreateParams);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pState);
		}
        break;

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
        if (CheckGuiTrapsMouse())
            break;

		int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);

		LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
        GameData* gameData = reinterpret_cast<GameData*>(ptr);

		if (wParam == MK_LBUTTON)
		{
            gameData->m_deltaMouseX += gameData->m_lastX - xPos;
            gameData->m_deltaMouseY += gameData->m_InvertYAxis ? (gameData->m_lastY - yPos) : -(gameData->m_lastY - yPos);
        }

		gameData->m_lastX = xPos;
        gameData->m_lastY = yPos;
    }
	break;
    case WM_MOUSEWHEEL:
	{
        LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
        GameData* gameData = reinterpret_cast<GameData*>(ptr);

		float modifier = 0.01f;

        if (MK_SHIFT == GET_KEYSTATE_WPARAM(wParam))
            modifier = 0.001f;

        gameData->m_wheelDelta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) * modifier;
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

/// @brief Application Update function
/// @param deltaInSeconds time between frames in seconds
void Update(double deltaInSeconds, GraphicsDX11& graphics, OrbitCamera& camera, GameData& data)
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

    camera.SetProjection(width, height, aspect);

	static double desiredTime = 1.0f / 4.0f; // 1 second / desired number of seconds
	static double incrementor = desiredTime;

	/// smoothly rotate our object
    data.m_increment += 0.1f * static_cast<float>(deltaInSeconds);
    if (data.m_increment > 360.0f)
        data.m_increment -= 360.0f;

	float polar = degreesToRadians(static_cast<float>(data.m_deltaMouseX));
	float azimuth = degreesToRadians(static_cast<float>(data.m_deltaMouseY));

	azimuth = clamp(azimuth, -DirectX::XM_PIDIV2, DirectX::XM_PIDIV2);

	DirectX::XMVECTOR At = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

    camera.RotateAroundPoint(At, polar, azimuth);
    camera.ChangeRadius(data.m_wheelDelta);

	camera.Update(deltaInSeconds);

	graphics.SetViewport(viewport);
	graphics.SetMVP(camera.GetMVP());
    graphics.SetVP(camera.GetVP());

	data.m_wheelDelta = 0.f;
}

