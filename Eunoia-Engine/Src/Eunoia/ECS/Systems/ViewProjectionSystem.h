#pragma once

#include "../ECS.h"

namespace Eunoia {

	EU_REFLECT(System)
	class EU_API ViewProjectionSystem : public ECSSystem
	{
	public:
		ViewProjectionSystem();
		virtual void ProcessEntityOnUpdate(EntityID entityID, r32 dt) override;
	};

}