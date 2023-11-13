#pragma once

#include "../ECS.h"
#include "../../Rendering/Asset/AssetTypeIDs.h"

namespace Eunoia {

	EU_REFLECT(System)
	class PhysicsSystem : public ECSSystem
	{
	public:
		PhysicsSystem();

		virtual void Init() override;
		virtual void PrePhysicsSimulation(EntityID entity, r32 dt) override;
		virtual void PostPhysicsSimulation(EntityID entity, r32 dt) override;
		virtual void ProcessEntityOnRender(EntityID entity) override;
	private:
		ModelID m_BoundingSphere;
		ModelID m_BoundingBox;
	};

}