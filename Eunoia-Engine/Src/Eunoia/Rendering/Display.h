#pragma once

#include "../DataStructures/String.h"
#include "../DataStructures/List.h"
#include "../Core/InputDefs.h"
#include "../Math/Math.h"

namespace Eunoia {
	
	enum DisplayEventType
	{
		DISPLAY_EVENT_CREATE,
		DISPLAY_EVENT_CLOSE,
		DISPLAY_EVENT_RESIZE,
		DISPLAY_EVENT_KEY,
		DISPLAY_EVENT_MOUSE_BUTTON,
		NUM_DISPLAY_EVENT_TYPES
	};

	enum DisplayInputEventType
	{
		DISPLAY_INPUT_EVENT_PRESS,
		DISPLAY_INPUT_EVENT_RELEASE,

		NUM_DISPLAY_INPUT_EVENT_TYPES
	};


	struct DisplayEvent
	{
		DisplayEventType type;
		//Used for resize event.
		u32 width;
		//Used for resize event.
		u32 height;
		//Used for input event
		u32 input;
		//Used for input event
		DisplayInputEventType inputType;
	};

	typedef void (*DisplayEventFunction)(const DisplayEvent&, void*);

	class EU_API Display
	{
	public:
		virtual b32 Create(const String& title, u32 width, u32 heigth) = 0;
		virtual void Destroy() = 0;
		virtual void Update() = 0;
		virtual String GetTitle() const = 0;
		virtual u32 GetWidth() const = 0;
		virtual u32 GetHeigth() const = 0;
		virtual b32 IsMinimized() const = 0;
		virtual b32 IsFullscreened() const = 0;

		virtual void SetFullscreen(b32 fullscreen) = 0;
		virtual void ToggleFullscreen() = 0;

		virtual b32 CheckForEvent(DisplayEventType type) const = 0;

		virtual v2 GetMousePos() = 0;
		virtual v2 GetMouseDeltaPos() = 0;
		virtual void SetMousePos(const v2& Pos) = 0;
		virtual b32 IsCursorVisible() = 0;
		virtual void SetCursorVisible(b32 visuble) = 0;

		void AddDisplayEventCallback(DisplayEventFunction function, void* userPtr = 0);
	protected:
		void ProcessDisplayEvents(const DisplayEvent& info);
		static void SetKeyState(Key key, b32 state);
		static void SetMouseButtonState(MouseButton button, b32 state);
	public:
		static Display* CreateDisplay();
	private:
		struct DisplayEventCallback
		{
			void* userPtr;
			DisplayEventFunction function;
		};

		List<DisplayEventCallback> m_Callbacks;
	};

}