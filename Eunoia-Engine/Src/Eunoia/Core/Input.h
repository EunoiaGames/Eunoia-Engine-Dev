#pragma once

#include "../Math/Math.h"
#include "InputDefs.h"

#define EU_MAX_KEYS 128
#define EU_MAX_MOUSE_BUTTONS 5
#define EU_MAX_GAMEPAD_BUTTONS 16
#define EU_MAX_GAMEPADS 4

namespace Eunoia {

	class Display;
	class EU_API EUInput
	{
	public:
		static void InitInput();
		static void BeginInput();
		static void UpdateInput();
		static void DestroyInput();

		static b32 IsKeyDown(Key key);
		static b32 IsKeyPressed(Key key);
		static b32 IsKeyRelease(Key key);

		static b32 IsButtonDown(MouseButton button);
		static b32 IsButtonPressed(MouseButton button);
		static b32 IsButtonRelease(MouseButton button);

		static b32 IsGamepadActive(Gamepad gamepad);

		static b32 IsGamepadButtonDown(Gamepad gamepad, GamepadButton button);
		static b32 IsGamepadButtonPressed(Gamepad gamepad, GamepadButton button);
		static b32 IsGamepadButtonReleased(Gamepad gamepad, GamepadButton button);

		static r32 GetGamepadTriggerAmount(Gamepad gamepad, GamepadTrigger trigger);
		static v2 GetGamepadThumbstick(Gamepad gamepad, GamepadThumbstick thumbstick);

		static char GetChar(Key key);
	protected:
		friend class Display;
		static u8 s_Keys[EU_MAX_KEYS];
		static u8 s_LastKeys[EU_MAX_KEYS];
		static u8 s_Buttons[EU_MAX_MOUSE_BUTTONS];
		static u8 s_LastButtons[EU_MAX_MOUSE_BUTTONS];
		static u16 s_GamepadButtons[EU_MAX_GAMEPADS];
		static u16 s_LastGamepadButtons[EU_MAX_GAMEPADS];
		static b32 s_ActiveGamepads[EU_MAX_GAMEPADS];
		static r32 s_GamepadTriggerValues[EU_MAX_GAMEPADS][EU_NUM_GAMEPAD_TRIGGERS];
		static v2 s_GamepadThumbsticks[EU_MAX_GAMEPADS][EU_NUM_GAMEPAD_THUMBSTICKS];
		static b32 s_LastGamepadTriggers[EU_MAX_GAMEPADS][EU_NUM_GAMEPAD_TRIGGERS];
		static char s_CharMap[EU_MAX_KEYS];
	};

}