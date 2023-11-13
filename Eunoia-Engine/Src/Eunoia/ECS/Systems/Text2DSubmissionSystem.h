#pragma once

#include "../ECS.h"

namespace Eunoia {

	EU_REFLECT(System)
	class EU_API Text2DSubmissionSystem : public ECSSystem
	{
	public:
		Text2DSubmissionSystem();
		virtual void ProcessEntityOnRender(EntityID entity) override;
	};

}