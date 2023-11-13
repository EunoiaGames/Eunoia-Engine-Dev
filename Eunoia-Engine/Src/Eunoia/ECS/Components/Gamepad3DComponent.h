#pragma once

#include "../ECS.h"
#include "../../Core/InputDefs.h"

namespace Eunoia {

	EU_REFLECT(Component)
	struct Gamepad3DComponent : public ECSComponent
	{
		Gamepad3DComponent(Gamepad gamepad = EU_GAMEPAD_0, r32 speed = 1.0f, r32 sensitivity = 1.0f, r32 sprintSpeedMultiplier = 2.0f,
			GamepadButton sprintButton = EU_GAMEPAD_XBOX360_BUTTON_LEFT_THUMB, GamepadButton toggleButton = EU_GAMEPAD_XBOX360_BUTTON_BACK, b32 invertY = false) :
			gamepad(gamepad),
			speed(speed),
			sensitivity(sensitivity),
			sprintSpeedMultiplier(sprintSpeedMultiplier),
			sprintButton(sprintButton),
			toggleButton(toggleButton),
			invertY(invertY)
		{}

		EU_PROPERTY() Gamepad gamepad;
		EU_PROPERTY() r32 speed;
		EU_PROPERTY() r32 sensitivity;
		EU_PROPERTY() r32 sprintSpeedMultiplier;
		EU_PROPERTY() GamepadButton sprintButton;
		EU_PROPERTY() GamepadButton toggleButton;
		EU_PROPERTY(Bool32) b32 invertY;
	};

}