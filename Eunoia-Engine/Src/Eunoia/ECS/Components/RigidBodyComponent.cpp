#include "RigidBodyComponent.h"

namespace Eunoia {

	RigidBodyComponent::RigidBodyComponent(const RigidBody& rigidBody, b32 debugDraw) :
		body(rigidBody),
		debugDraw(debugDraw),
		forceDraw(false)
	{
		body.SetEntityHandle(parent);
	}

	void RigidBodyComponent::OnDestroy()
	{
		body.RemoveFromPhysicsWorld();
	}

}
