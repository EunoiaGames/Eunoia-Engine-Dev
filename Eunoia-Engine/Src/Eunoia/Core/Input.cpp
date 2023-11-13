#include "Input.h"
#include <cstring>

#ifdef EU_PLATFORM_WINDOWS
#include "../Platform/Win32/DisplayWin32.h"
#include <Xinput.h>
#endif

namespace Eunoia {

	u8 EUInput::s_Keys[];
	u8 EUInput::s_LastKeys[];
	u8 EUInput::s_Buttons[];
	u8 EUInput::s_LastButtons[];
	u16 EUInput::s_GamepadButtons[];
	u16 EUInput::s_LastGamepadButtons[];
	b32 EUInput::s_ActiveGamepads[];
	r32 EUInput::s_GamepadTriggerValues[EU_MAX_GAMEPADS][EU_NUM_GAMEPAD_TRIGGERS];
	b32 EUInput::s_LastGamepadTriggers[EU_MAX_GAMEPADS][EU_NUM_GAMEPAD_TRIGGERS];
	v2 EUInput::s_GamepadThumbsticks[EU_MAX_GAMEPADS][EU_NUM_GAMEPAD_THUMBSTICKS];

	char EUInput::s_CharMap[];

	void EUInput::InitInput()
	{
		memset(s_Keys, false, EU_MAX_KEYS);
		memset(s_LastKeys, false, EU_MAX_KEYS);
		memset(s_Buttons, false, EU_MAX_MOUSE_BUTTONS);
		memset(s_LastButtons, false, EU_MAX_MOUSE_BUTTONS);
		memset(s_CharMap, 0, EU_MAX_KEYS);

		s_CharMap[EU_KEY_Q] = 'q';
		s_CharMap[EU_KEY_W] = 'w';
		s_CharMap[EU_KEY_E] = 'e';
		s_CharMap[EU_KEY_R] = 'r';
		s_CharMap[EU_KEY_T] = 't';
		s_CharMap[EU_KEY_Y] = 'y';
		s_CharMap[EU_KEY_U] = 'u';
		s_CharMap[EU_KEY_I] = 'i';
		s_CharMap[EU_KEY_O] = 'o';
		s_CharMap[EU_KEY_P] = 'p';
		s_CharMap[EU_KEY_A] = 'a';
		s_CharMap[EU_KEY_S] = 's';
		s_CharMap[EU_KEY_D] = 'd';
		s_CharMap[EU_KEY_F] = 'f';
		s_CharMap[EU_KEY_G] = 'g';
		s_CharMap[EU_KEY_H] = 'h';
		s_CharMap[EU_KEY_J] = 'j';
		s_CharMap[EU_KEY_K] = 'k';
		s_CharMap[EU_KEY_L] = 'l';
		s_CharMap[EU_KEY_Z] = 'z';
		s_CharMap[EU_KEY_X] = 'x';
		s_CharMap[EU_KEY_C] = 'c';
		s_CharMap[EU_KEY_V] = 'v';
		s_CharMap[EU_KEY_B] = 'b';
		s_CharMap[EU_KEY_N] = 'n';
		s_CharMap[EU_KEY_M] = 'm';
		s_CharMap[EU_KEY_SPACE] = ' ';

		s_CharMap[EU_KEY_1] = '1';
		s_CharMap[EU_KEY_2] = '2';
		s_CharMap[EU_KEY_3] = '3';
		s_CharMap[EU_KEY_4] = '4';
		s_CharMap[EU_KEY_5] = '5';
		s_CharMap[EU_KEY_6] = '6';
		s_CharMap[EU_KEY_7] = '7';
		s_CharMap[EU_KEY_8] = '8';
		s_CharMap[EU_KEY_9] = '9';
		s_CharMap[EU_KEY_0] = '0';

		s_CharMap[EU_KEY_PERIOD] = '.';
		s_CharMap[EU_KEY_COMMA] = ',';
		s_CharMap[EU_KEY_SLASH] = ',';
		s_CharMap[EU_KEY_SEMI_COLON] = ':';
		s_CharMap[EU_KEY_QUOTE] = '\'';
		s_CharMap[EU_KEY_LEFT_BRACKET] = '[';
		s_CharMap[EU_KEY_RIGHT_BRACKET] = ']';
		s_CharMap[EU_KEY_BACK_SLASH] = '\\';
		s_CharMap[EU_KEY_DASH] = '-';
		s_CharMap[EU_KEY_EQUALS] = '=';
		s_CharMap[EU_KEY_TILDE] = '`';
	}

	void EUInput::BeginInput()
	{
#ifdef EU_PLATFORM_WINDOWS
		XINPUT_STATE state{};
		for (u32 i = 0; i < XUSER_MAX_COUNT; i++)
		{
			if (XInputGetState(i, &state) != ERROR_SUCCESS)
			{
				s_ActiveGamepads[i] = false;
				continue;
			}

			s_ActiveGamepads[i] = true;
			s_GamepadButtons[i] = state.Gamepad.wButtons;
			s_GamepadTriggerValues[i][EU_GAMEPAD_XBOX360_TRIGGER_LT] = (r32)state.Gamepad.bLeftTrigger / 255.0f;
			s_GamepadTriggerValues[i][EU_GAMEPAD_XBOX360_TRIGGER_RT] = (r32)state.Gamepad.bRightTrigger / 255.0f;
			s_GamepadThumbsticks[i][EU_GAMEPAD_XBOX360_THUMBSTICK_LEFT] = v2(EU_MAX(-1.0f, (r32)state.Gamepad.sThumbLX / 32767.0f), EU_MAX(-1.0f, (r32)state.Gamepad.sThumbLY / 32767.0f));
			s_GamepadThumbsticks[i][EU_GAMEPAD_XBOX360_THUMBSTICK_RIGHT] = v2(EU_MAX(-1.0f, (r32)state.Gamepad.sThumbRX / 32767.0f), EU_MAX(-1.0f, (r32)state.Gamepad.sThumbRY / 32767.0f));
		} 
#endif
	}

	void EUInput::UpdateInput()
	{
		memcpy(s_LastKeys, s_Keys, EU_MAX_KEYS);
		memcpy(s_LastButtons, s_Buttons,EU_MAX_MOUSE_BUTTONS);
		for (u32 i = 0; i < EU_MAX_GAMEPADS; i++)
		{
			s_LastGamepadButtons[i] = s_GamepadButtons[i];
			s_LastGamepadTriggers[i][EU_GAMEPAD_XBOX360_TRIGGER_LT] = s_GamepadTriggerValues[i][EU_GAMEPAD_XBOX360_TRIGGER_LT] >= 0.0f;
			s_LastGamepadTriggers[i][EU_GAMEPAD_XBOX360_TRIGGER_RT] = s_GamepadTriggerValues[i][EU_GAMEPAD_XBOX360_TRIGGER_RT] >= 0.0f;
		}
	}

	void EUInput::DestroyInput() {  }

	b32 EUInput::IsKeyDown(Key key) { return s_Keys[key]; }
	b32 EUInput::IsKeyPressed(Key key) { return s_Keys[key] && !s_LastKeys[key]; }
	b32 EUInput::IsKeyRelease(Key key) { return !s_Keys[key] && s_LastKeys[key]; }

	b32 EUInput::IsButtonDown(MouseButton button) { return s_Buttons[button]; }
	b32 EUInput::IsButtonPressed(MouseButton button) { return s_Buttons[button] && !s_LastButtons[button]; }
	b32 EUInput::IsButtonRelease(MouseButton button) { return !s_Buttons[button] && s_LastButtons[button]; }

	b32 EUInput::IsGamepadActive(Gamepad gamepad)
	{
		return s_ActiveGamepads[gamepad];
	}

	b32 EUInput::IsGamepadButtonDown(Gamepad gamepad, GamepadButton button)
	{
		if (button == EU_GAMEPAD_XBOX360_BUTTON_LT)
			return s_GamepadTriggerValues[gamepad][EU_GAMEPAD_XBOX360_TRIGGER_LT] > 0.0f;
		else if (button == EU_GAMEPAD_XBOX360_BUTTON_RT)
			return s_GamepadTriggerValues[gamepad][EU_GAMEPAD_XBOX360_TRIGGER_RT] > 0.0f;

		return (s_GamepadButtons[gamepad] & button);
	}

	b32 EUInput::IsGamepadButtonPressed(Gamepad gamepad, GamepadButton button)
	{
		if (button == EU_GAMEPAD_XBOX360_BUTTON_LT)
			return s_GamepadTriggerValues[gamepad][EU_GAMEPAD_XBOX360_TRIGGER_LT] > 0.0f && !s_LastGamepadTriggers[EU_GAMEPAD_XBOX360_TRIGGER_LT];
		else if (button == EU_GAMEPAD_XBOX360_BUTTON_RT)
			return s_GamepadTriggerValues[gamepad][EU_GAMEPAD_XBOX360_TRIGGER_RT] > 0.0f && !s_LastGamepadTriggers[EU_GAMEPAD_XBOX360_TRIGGER_RT];

		return (s_GamepadButtons[gamepad] & button) && !(s_LastGamepadButtons[gamepad] & button);
	}

	b32 EUInput::IsGamepadButtonReleased(Gamepad gamepad, GamepadButton button)
	{
		if (button == EU_GAMEPAD_XBOX360_BUTTON_LT)
			return !(s_GamepadTriggerValues[gamepad][EU_GAMEPAD_XBOX360_TRIGGER_LT] > 0.0f) && s_LastGamepadTriggers[EU_GAMEPAD_XBOX360_TRIGGER_LT];
		else if (button == EU_GAMEPAD_XBOX360_BUTTON_RT)
			return !(s_GamepadTriggerValues[gamepad][EU_GAMEPAD_XBOX360_TRIGGER_RT] > 0.0f) && s_LastGamepadTriggers[EU_GAMEPAD_XBOX360_TRIGGER_RT];

		return !(s_GamepadButtons[gamepad] & button) && (s_LastGamepadButtons[gamepad] & button);
	}

	r32 EUInput::GetGamepadTriggerAmount(Gamepad gamepad, GamepadTrigger trigger)
	{
		return s_GamepadTriggerValues[gamepad][trigger];
	}

	v2 EUInput::GetGamepadThumbstick(Gamepad gamepad, GamepadThumbstick thumbstick)
	{
		return s_GamepadThumbsticks[gamepad][thumbstick];
	}

	char EUInput::GetChar(Key key)
	{
		char c = s_CharMap[key];

		if (s_Keys[EU_KEY_LEFT_SHIFT] || s_Keys[EU_KEY_RIGHT_SHIFT])
		{
			if (c >= 'a' && c <= 'z') return toupper(c);
			else if (c == '1') return '!';
			else if (c == '2') return '@';
			else if (c == '3') return '#';
			else if (c == '4') return '$';
			else if (c == '5') return '%';
			else if (c == '6') return '^';
			else if (c == '7') return '&';
			else if (c == '8') return '*';
			else if (c == '9') return '(';
			else if (c == '0') return ')';
			else if (c == '`') return '~';
			else if (c == '-') return '_';
			else if (c == '=') return '+';
			else if (c == '[') return '{';
			else if (c == ']') return '}';
			else if (c == '\\') return '|';
			else if (c == ';') return ':';
			else if (c == '\'') return '"';
			else if (c == ',') return '<';
			else if (c == '.') return '>';
			else if (c == '/') return '?';
			else return c;
		}
		else
		{
			return c;
		}
	}

}