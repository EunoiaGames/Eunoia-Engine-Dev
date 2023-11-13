#pragma once

#include "../Math/Math.h"
#include <Bullet\btBulletCollisionCommon.h>
#include <Bullet\btBulletDynamicsCommon.h>
#include "../ECS/ECSTypes.h"
#include "../DataStructures/List.h"

namespace Eunoia {

	enum RayCastMode
	{
		RAY_CAST_MODE_FIRST_HIT,
		RAY_CAST_MODE_ALL,
		NUM_RAY_CAST_MODES
	};

	struct RayCastResult
	{
		b32 hit;
		List<EntityID> hitEntities;
		EntityID firstHitEntity;
	};

	class EU_API PhysicsEngine3D
	{
	public:
		void Init(const v3& gravity = v3(0.0f, -9.81f, 0.0f));
		void Destroy();


		b32 CastRay(const v3& from, const v3& to, RayCastMode mode, RayCastResult* result = 0);
		void SetGravity(r32 g);
		void SetGravity(const v3& gravity);
		
		void AddRigidBodyToWorld(btRigidBody* body);
		void RemoveRigidBodyFromWorld(btRigidBody* body);

		void ResetRigidBodyTransforms();
		void StepSimulation(r32 dt);
		void ClearForces();
	public:
		static btVector3 ToBulletVector(const v3& vec);
		static v3 ToEngineVector(const btVector3& vec);

		static btQuaternion ToBulletQuat(const quat& q);
		static quat ToEngineQuat(const btQuaternion& q);
	private:
		btDynamicsWorld* m_World;
		btDispatcher* m_Dispatcher;
		btBroadphaseInterface* m_BroadPhase;
		btConstraintSolver* m_ConstraintSolver;
		btCollisionConfiguration* m_CollisionConfiguration;
		List<btRigidBody*> m_RigidBodies;
	};

}