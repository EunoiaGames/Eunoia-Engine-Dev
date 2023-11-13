#pragma once

#include "../ECS.h"

namespace Eunoia {

	EU_REFLECT(System)
	class EU_API ViewProjection2DSystem : public ECSSystem
	{
	public:
		ViewProjection2DSystem();
		virtual void ProcessEntityOnRender(EntityID entity) override;
	};

}