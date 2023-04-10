#pragma once
#include "Okay/Okay.h"
#include "Graphics/RenderTexture.h"

#include <Windows.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <unordered_map>

class Window
{
public:
	static const size_t MAX_FILENAME_LENGTH = 256ull;

	Window();
	Window(uint32_t width, uint32_t height, const wchar_t* windowName, uint32_t renderTexFlags = Okay::INVALID_UINT);
	~Window();
	void shutdown();
	
	void create(uint32_t width, uint32_t height, const wchar_t* windowName, uint32_t renderTexFlags = Okay::INVALID_UINT);

	void createRenderTexture(uint32_t flags);
	inline Okay::Ref<Okay::RenderTexture> getRenderTexture();
	inline const Okay::Ref<Okay::RenderTexture> getRenderTexture() const;
	inline void clear();
	inline void present();

	DirectX::XMUINT2 getDimensions() const;

	void show();
	void close();
	inline bool isOpen() const;
	void setName(const wchar_t* name);

	HWND getHWnd() const;
	void update();

	static inline Window* getActiveWindow();
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	MSG msg;
	HWND hWnd;
	bool open;

	IDXGISwapChain* pSwapChain;
	Okay::Ref<Okay::RenderTexture> renderTexture;

	void getAndSetBackBuffer(uint32_t flags);

private: // Window handling through HWND
	static std::unordered_map<HWND, Window*> windows;
	static void onResize(HWND hWnd, WPARAM wParam);
};

inline bool Window::isOpen() const { return open; }

inline Okay::Ref<Okay::RenderTexture> Window::getRenderTexture() { return renderTexture; }
inline const Okay::Ref<Okay::RenderTexture> Window::getRenderTexture() const { return renderTexture; }

inline void Window::clear() { renderTexture->clear(); }

inline void Window::present() { pSwapChain->Present(0u, 0u); }

inline HWND Window::getHWnd() const { return hWnd; }

inline Window* Window::getActiveWindow() 
{
	auto it = Window::windows.find(GetActiveWindow());
	return it != Window::windows.end() ? it->second : nullptr;
}
