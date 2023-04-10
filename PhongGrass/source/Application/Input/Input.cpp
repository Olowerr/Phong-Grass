#include "Input.h"
#include "../Window.h"

namespace Okay
{
	float Input::getDesktopMouseXPos()
	{
		POINT pos{};
		GetCursorPos(&pos);
		return (float)pos.x;
	}

	float Input::getDesktopMouseYPos()
	{
		POINT pos{};
		GetCursorPos(&pos);
		return (float)pos.y;
	}

	DirectX::XMFLOAT2 Input::getDesktopMousePos()
	{
		POINT pos{};
		GetCursorPos(&pos);
		return DirectX::XMFLOAT2((float)pos.x, (float)pos.y);
	}

	void Input::setDesktopMouseXPos(float xPos) 
	{
		SetCursorPos((int)xPos, (int)Input::getDesktopMouseYPos());
	}
	
	void Input::setDesktopMouseYPos(float yPos) 
	{
		SetCursorPos((int)Input::getDesktopMouseXPos(), (int)yPos);
	}

	void Input::setMouseXPos(float xPos)
	{
		Window* pWindow = Window::getActiveWindow();
		if (!pWindow)
			return;

		HWND hWnd = pWindow->getHWnd();
		RECT rect{};
		GetClientRect(hWnd, &rect);
		ClientToScreen(hWnd, (LPPOINT)&rect.left);

		SetCursorPos((int)xPos + rect.left, (int)Input::getDesktopMouseYPos());
	}

	void Input::setMouseYPos(float yPos)
	{
		Window* pWindow = Window::getActiveWindow();
		if (!pWindow)
			return;

		HWND hWnd = pWindow->getHWnd();
		RECT rect{};
		GetClientRect(hWnd, &rect);
		ClientToScreen(hWnd, (LPPOINT)&rect.left);

		SetCursorPos((int)Input::getDesktopMouseXPos(), (int)yPos + rect.top);
	}

	float Input::getMouseXPos()
	{
		Window* pWindow = Window::getActiveWindow();
		if (!pWindow)
			return 0.f;

		POINT p;
		GetCursorPos(&p);
		ScreenToClient(pWindow->getHWnd(), &p);
		return (float)p.x;
	}
	
	float Input::getMouseYPos()
	{
		Window* pWindow = Window::getActiveWindow();
		if (!pWindow)
			return 0.f;

		POINT p;
		GetCursorPos(&p);
		ScreenToClient(pWindow->getHWnd(), &p);
		return (float)p.y;
	}

	DirectX::XMFLOAT2 Input::getMousePos()
	{
		Window* pWindow = Window::getActiveWindow();
		if (!pWindow)
			return DirectX::XMFLOAT2();

		POINT p;
		GetCursorPos(&p);
		ScreenToClient(pWindow->getHWnd(), &p);
		return DirectX::XMFLOAT2((float)p.x, (float)p.y);
	}

	void Input::update()
	{
		memcpy(Input::prevKeys, Input::keys, size_t(Key::NUM_KEYS));
		Input::prevMouseLeft = Input::mouseLeft;
		Input::prevMouseRight = Input::mouseRight;

		Input::mouseXDelta = Input::mouseXPos - Input::mousePrevXPos;
		Input::mouseYDelta = Input::mouseYPos - Input::mousePrevYPos;
		Input::mousePrevXPos = Input::mouseXPos;
		Input::mousePrevYPos = Input::mouseYPos;

		Input::mouseWheelDir = 0.f;
	}
}