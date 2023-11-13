#pragma once

#include "../ECS.h"

namespace Eunoia {

	EU_REFLECT(System)
	class EU_API TransformHierarchy3DSystem : public ECSSystem
	{
	public:
		TransformHierarchy3DSystem();
		virtual void ProcessEntityOnUpdate(EntityID entity, r32 dt) override;
	};

}