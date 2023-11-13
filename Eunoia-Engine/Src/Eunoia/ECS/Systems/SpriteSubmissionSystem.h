#pragma once

#include "../ECS.h"

namespace Eunoia {

	EU_REFLECT(System)
	class EU_API SpriteSubmissionSystem : public ECSSystem
	{
	public:
		SpriteSubmissionSystem();
		virtual void ProcessEntityOnRender(EntityID entity) override;
	};

	class EU_API SpriteGroupSubmissionSystem : public ECSSystem
	{
	public:
		SpriteGroupSubmissionSystem();
		virtual void ProcessEntityOnRender(EntityID entity) override;
	};

}