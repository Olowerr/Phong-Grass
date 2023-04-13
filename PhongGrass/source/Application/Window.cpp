#include "Window.h"
#include "Input/Input.h"
#include "imgui/imgui_impl_win32.h"
#include "DirectX/DX11.h"

#include <windowsx.h>

#ifndef DIST
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // 


std::unordered_map<HWND, Window*> Window::windows;

Window::Window()
	:hWnd(nullptr), msg(), open(false), pSwapChain(nullptr)
{
}

Window::Window(uint32_t width, uint32_t height, const wchar_t* windowName, uint32_t renderTexFlags)
	:open(false), msg(), pSwapChain(nullptr)
{
	create(width, height, windowName, renderTexFlags);
}

void Window::create(uint32_t width, uint32_t height, const wchar_t* windowName, uint32_t renderTexFlags)
{
	WNDCLASS winClass = {};
	winClass.lpfnWndProc = WindowProc;
	winClass.hInstance = GetModuleHandle(NULL);
	winClass.lpszClassName = windowName;
	winClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	winClass.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClass(&winClass);

	RECT rect = {};
	rect.right = (LONG)width;
	rect.bottom = (LONG)height;

	RECT desktop;
	GetWindowRect(GetDesktopWindow(), &desktop);
	LONG xDiff = desktop.right - rect.right;
	LONG yDiff = desktop.bottom - rect.bottom;

	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	hWnd = CreateWindowEx(0, windowName, windowName, WS_OVERLAPPEDWINDOW,
		xDiff / 2, yDiff / 4, rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, GetModuleHandle(NULL), nullptr);

	OKAY_ASSERT(hWnd != nullptr, "Failed creating window");
	windows.insert({ hWnd, this });

	// Create before show, or onResize is called and swapChain is nullptr
	createRenderTexture(renderTexFlags == Okay::INVALID_UINT ? Okay::RenderTexture::B_RENDER : renderTexFlags);
	show();
	present();
}

Window::~Window()
{
	shutdown();
}

void Window::shutdown()
{
	if (hWnd)
	{
		DestroyWindow(hWnd);
		windows.erase(hWnd);
	}
	hWnd = nullptr;

	UnregisterClass(L"WinClass", GetModuleHandle(NULL));

	DX11_RELEASE(pSwapChain);
}

void Window::show()
{
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	open = true;
}

void Window::close()
{
	CloseWindow(hWnd);
	open = false;
}

void Window::setName(const wchar_t* name)
{
	SetWindowText(hWnd, name);
}

DirectX::XMUINT2 Window::getDimensions() const
{
	RECT rect{};
	GetClientRect(hWnd, &rect);
	return DirectX::XMUINT2(uint32_t(rect.right), uint32_t(rect.bottom));
}

void Window::update()
{
	Okay::Input::update();

	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if (msg.message == WM_QUIT)
		open = false;
}

void Window::createRenderTexture(uint32_t flags)
{
	DX11::createSwapChain(&pSwapChain, hWnd,
		(CHECK_BIT(flags, Okay::RenderTexture::BitPos::B_RENDER)		? DXGI_USAGE_RENDER_TARGET_OUTPUT	: 0u) |
		(CHECK_BIT(flags, Okay::RenderTexture::BitPos::B_SHADER_READ)	? DXGI_USAGE_SHADER_INPUT			: 0u) |
		(CHECK_BIT(flags, Okay::RenderTexture::BitPos::B_SHADER_WRITE)	? DXGI_USAGE_UNORDERED_ACCESS		: 0u) );

	OKAY_ASSERT(pSwapChain, "Failed creating swapchain");

	renderTexture = Okay::createRef<Okay::RenderTexture>();
	getAndSetBackBuffer(flags);
}

void Window::getAndSetBackBuffer(uint32_t flags)
{
	ID3D11Texture2D* backBuffer = nullptr;
	pSwapChain->GetBuffer(0u, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	OKAY_ASSERT(backBuffer, "Failed getting backBuffer");

	renderTexture->create(backBuffer, flags);
	DX11_RELEASE(backBuffer);
}

void Window::resize(uint32_t width, uint32_t height)
{
	// Release all external references to the backBuffer before resizing the buffer
	const uint32_t flags = renderTexture->getFlags();
	renderTexture->shutdown();

	pSwapChain->ResizeBuffers(0u, width, height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	getAndSetBackBuffer(flags);
}

void Window::setFullscreen(bool fullscreen)
{
	RECT rect;
	GetWindowRect(GetDesktopWindow(), &rect);
	resize((uint32_t)rect.right, (uint32_t)rect.bottom);

	pSwapChain->SetFullscreenState(fullscreen, nullptr);
}

LRESULT Window::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#ifndef DIST
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;
#endif

	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;

	case WM_KEYDOWN:
		Okay::Input::setKeyDown((Key)wParam);
		return 0;

	case WM_KEYUP:
		Okay::Input::setKeyUp((Key)wParam);
		return 0;

	case WM_SYSKEYDOWN:
		Okay::Input::setKeyDown((Key)wParam);
		return 0;
		
	case WM_SYSKEYUP:
		Okay::Input::setKeyUp((Key)wParam);
		return 0;

	case WM_MOUSEMOVE:
		Okay::Input::mouseXPos = (float)GET_X_LPARAM(lParam);
		Okay::Input::mouseYPos = (float)GET_Y_LPARAM(lParam);
		return 0;

	case WM_MOUSEWHEEL:
		Okay::Input::mouseWheelDir = GET_WHEEL_DELTA_WPARAM(wParam) * (1.f / 120.f);
		return 0;

	case WM_LBUTTONDOWN:
		Okay::Input::mouseLeft = true;
		return 0;

	case WM_LBUTTONUP:
		Okay::Input::mouseLeft = false;
		return 0;

	case WM_RBUTTONDOWN:
		Okay::Input::mouseRight = true;
		return 0;

	case WM_RBUTTONUP:
		Okay::Input::mouseRight = false;
		return 0;

	case WM_SIZE:
		Window::onResize(hWnd, wParam); // TODO: Only call when resizing is finished
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void Window::onResize(HWND hWnd, WPARAM wParam)
{
	if (wParam == SIZE_MINIMIZED)
		return;

	auto it = windows.find(hWnd);
	OKAY_ASSERT(it != windows.end(), "Resized invalid window (?)");
	it->second->resize();
}
