#pragma once

#include "../../Rendering/Display.h"
#include "../../Math/Math.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Eunoia {

	class EU_API DisplayWin32 : public Display
	{
	public:
		DisplayWin32();

		virtual b32 Create(const String& title, u32 width, u32 height) override;
		virtual void Destroy() override;
		virtual void Update() override;
		virtual String GetTitle() const override;
		virtual u32 GetWidth() const override;
		virtual u32 GetHeigth() const override;
		virtual b32 IsMinimized() const override;
		virtual b32 IsFullscreened() const override;

		virtual void SetFullscreen(b32 fullscreen) override;
		virtual void ToggleFullscreen() override;

		virtual v2 GetMousePos() override;
		virtual v2 GetMouseDeltaPos() override;
		virtual void SetMousePos(const v2& Pos) override;
		virtual b32 IsCursorVisible() override;
		virtual void SetCursorVisible(b32 visuble) override;

		virtual b32 CheckForEvent(DisplayEventType type) const override;

		void AddWindowProc(WNDPROC proc);
		HWND GetHandle() const;

		friend LRESULT CALLBACK DisplayCallbackWin32(HWND windowHandle, UINT msg, WPARAM wparam, LPARAM lparam);
		friend void HandleInput(WPARAM wparam, LPARAM lparam, DisplayWin32* display);
	private:
		void InitRawInputs();
	private:
		HWND m_Handle;
		u32 m_Width;
		u32 m_Height;
		String m_Title;
		b32 m_IsFullscreened;
		WINDOWPLACEMENT m_PrevPos;

		WNDPROC m_ExtraWindowProcs[8];
		u32 m_NumExtraWindowProcs;

		b32 m_IsCursorVisible;
		v2 m_PrevCursorPos;

		b32 m_EventsThisFrame[NUM_DISPLAY_EVENT_TYPES];
	private:
		static b32 s_ClassRegistered;
		static LPCWSTR s_ClassName;
	};

}