#pragma once

#include "../ECS.h"
#include "../../Core/InputDefs.h"


namespace Eunoia {

	EU_REFLECT(System)
	class EU_API MouseLookAround3DSystem : public ECSSystem
	{
	public:
		MouseLookAround3DSystem();
		virtual void Init() override;
		virtual void PreUpdate(r32 dt) override;
		virtual void ProcessEntityOnUpdate(EntityID entity, r32 dt) override;
	public:
		Key toggleKey;
	};

}