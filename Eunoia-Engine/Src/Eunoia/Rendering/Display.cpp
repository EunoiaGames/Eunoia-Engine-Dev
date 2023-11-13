#include "Display.h"

#include "../Core/Input.h"

#ifdef EU_PLATFORM_WINDOWS
#include "../Platform/Win32/DisplayWin32.h"
#endif

namespace Eunoia {

	void Display::AddDisplayEventCallback(DisplayEventFunction function, void* userPtr)
	{
		DisplayEventCallback callback;
		callback.function = function;
		callback.userPtr = userPtr;

		m_Callbacks.Push(callback);
	}

	void Display::ProcessDisplayEvents(const DisplayEvent& info)
	{
		for (u32 i = 0; i < m_Callbacks.Size(); i++)
			m_Callbacks[i].function(info, m_Callbacks[i].userPtr);
	}

	void Display::SetKeyState(Key key, b32 state)
	{
		EUInput::s_Keys[key] = state;
	}

	void Display::SetMouseButtonState(MouseButton button, b32 state)
	{
		EUInput::s_Buttons[button] = state;
	}

	Display* Display::CreateDisplay()
	{
#ifdef EU_PLATFORM_WINDOWS
		return new DisplayWin32();
#endif
		return 0;
	}
}
