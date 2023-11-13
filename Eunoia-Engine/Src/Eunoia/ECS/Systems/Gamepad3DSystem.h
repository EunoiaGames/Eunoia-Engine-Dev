#pragma once

#include "../ECS.h"
#include "../../Core/InputDefs.h"

namespace Eunoia {

	EU_REFLECT(System)
	class Gamepad3DSystem : public ECSSystem
	{
	public:
		Gamepad3DSystem();

		virtual void Init() override;
		virtual void PreUpdate(r32 dt) override;
		virtual void ProcessEntityOnUpdate(EntityID entity, r32 dt) override;
	public:
		GamepadButton toggleButton;
	private:
		b32 m_Toggled;
	};

}
