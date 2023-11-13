#pragma once

#include "../ECS.h"

namespace Eunoia {

	EU_REFLECT(System)
	class EU_API LightSubmissionSystem : public ECSSystem
	{
	public:
		LightSubmissionSystem();
		virtual void ProcessEntityOnRender(EntityID entity) override;
	};

}