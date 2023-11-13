#pragma once

#include "../ECS.h"

namespace Eunoia {

	EU_REFLECT(System)
	class EU_API TransformHierarchy2DSystem : public ECSSystem
	{
	public:
		TransformHierarchy2DSystem();
		virtual void ProcessEntityOnUpdate(EntityID entity, r32 dt) override;
	};

}