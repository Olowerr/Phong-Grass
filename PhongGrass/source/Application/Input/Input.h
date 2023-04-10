#pragma once

#include "Keys.h"
#include <DirectXMath.h>

class Window;

namespace Okay
{
	class Input
	{
	public:
		friend class Window;

		static inline bool isKeyDown(Key key);
		static inline bool isKeyPressed(Key key);
		static inline bool isKeyReleased(Key key);

		static inline bool leftMouseDown();
		static inline bool leftMouseClicked();
		static inline bool leftMouseReleased();

		static inline bool rightMouseDown();
		static inline bool rightMouseClicked();
		static inline bool rightMouseReleased();

		static float getMouseXPos();
		static float getMouseYPos();
		static DirectX::XMFLOAT2 getMousePos();
		
		static void setMouseXPos(float xPos);
		static void setMouseYPos(float yPos);

		static float getDesktopMouseXPos();
		static float getDesktopMouseYPos();
		static DirectX::XMFLOAT2 getDesktopMousePos();

		static void setDesktopMouseXPos(float xPos);
		static void setDesktopMouseYPos(float yPos);

		static inline float getMouseXDelta();
		static inline float getMouseYDelta();

		static inline float getMouseWheelDir();

	private:
		static void update();
		static inline void setKeyDown(Key key);
		static inline void setKeyUp(Key key);

		static inline bool keys[Key::NUM_KEYS]{};
		static inline bool prevKeys[Key::NUM_KEYS]{};

		static inline bool mouseLeft = false, mouseRight = false;
		static inline bool prevMouseLeft = false, prevMouseRight = false;

		static inline float mouseXPos = 0.f, mouseYPos = 0.f;
		static inline float mousePrevXPos = 0.f, mousePrevYPos = 0.f;
		static inline float mouseXDelta = 0.f, mouseYDelta = 0.f;

		static inline float mouseWheelDir = 0.f;
	};

	inline bool Input::isKeyDown(Key key)	  { return Input::keys[key]; }
	inline bool Input::isKeyPressed(Key key)  { return Input::keys[key] && !Input::prevKeys[key]; }
	inline bool Input::isKeyReleased(Key key) { return !Input::keys[key] && Input::prevKeys[key]; }

	inline void Input::setKeyDown(Key key)	  { if (key >= Key::NUM_KEYS) return; keys[key] = true;  }
	inline void Input::setKeyUp(Key key)	  { if (key >= Key::NUM_KEYS) return; keys[key] = false; }
											  
	inline bool Input::leftMouseDown()		  { return Input::mouseLeft; }
	inline bool Input::leftMouseClicked()	  { return Input::mouseLeft && !Input::prevMouseLeft; }
	inline bool Input::leftMouseReleased()	  { return !Input::mouseLeft && Input::prevMouseLeft; }
											  
	inline bool Input::rightMouseDown()		  { return Input::mouseRight; }
	inline bool Input::rightMouseClicked()	  { return Input::mouseRight && !Input::prevMouseRight; }
	inline bool Input::rightMouseReleased()	  { return !Input::mouseRight && Input::prevMouseRight; }

	inline float Input::getMouseXDelta()	  { return Input::mouseXDelta; }
	inline float Input::getMouseYDelta()	  { return Input::mouseYDelta; }
	
	inline float Input::getMouseWheelDir()	  { return Input::mouseWheelDir; }
}