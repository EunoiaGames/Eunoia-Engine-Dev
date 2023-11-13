#pragma once

#include "../ECS.h"

namespace Eunoia {

	EU_REFLECT(System)
	class EU_API ModelSubmissionSystem : public ECSSystem
	{
	public:
		ModelSubmissionSystem();
		virtual void ProcessEntityOnRender(EntityID entity) override;
	};

}