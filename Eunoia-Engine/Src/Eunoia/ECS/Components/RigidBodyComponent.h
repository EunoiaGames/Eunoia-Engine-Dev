#pragma once

#include "../ECS.h"
#include "../../Physics/RigidBody.h"

namespace Eunoia {

	EU_REFLECT(Component)
	struct RigidBodyComponent : public ECSComponent
	{
		EU_API RigidBodyComponent(const RigidBody& rigidBody, b32 debugDraw = false);

		RigidBodyComponent() :
			debugDraw(false),
			forceDraw(false)
		{}

		EU_API virtual void OnDestroy() override;

		EU_PROPERTY() RigidBody body;
		EU_PROPERTY(Bool32) b32 debugDraw;
		b32 forceDraw;
	};

}