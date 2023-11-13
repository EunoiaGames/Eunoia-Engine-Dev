#include "PhysicsEngine3D.h"
#include "../DataStructures/List.h"

namespace Eunoia {

	
	void PhysicsEngine3D::Init(const v3& gravity)
	{
		m_CollisionConfiguration = new btDefaultCollisionConfiguration();
		m_Dispatcher = new btCollisionDispatcher(m_CollisionConfiguration);
		m_BroadPhase = new btDbvtBroadphase();
		m_ConstraintSolver = new btSequentialImpulseConstraintSolver();
		m_World = new btDiscreteDynamicsWorld(m_Dispatcher, m_BroadPhase, m_ConstraintSolver, m_CollisionConfiguration);
		SetGravity(gravity);
	}

	void PhysicsEngine3D::Destroy()
	{
		delete m_World;
		delete m_Dispatcher;
		delete m_BroadPhase;
		delete m_ConstraintSolver;
		delete m_CollisionConfiguration;

		for (u32 i = 0; i < m_RigidBodies.Size(); i++)
			delete m_RigidBodies[i];
	}

	b32 PhysicsEngine3D::CastRay(const v3& from, const v3& to, RayCastMode mode, RayCastResult* result)
	{
		btVector3 btFrom = ToBulletVector(from);
		btVector3 btTo = ToBulletVector(to);
		btDynamicsWorld::RayResultCallback* callback;
		if (mode == RAY_CAST_MODE_FIRST_HIT)
		{
			callback = new btDynamicsWorld::ClosestRayResultCallback(btFrom, btTo);
		}
		else if (mode == RAY_CAST_MODE_ALL)
		{
			callback = new btDynamicsWorld::AllHitsRayResultCallback(btFrom, btTo);
		}
		
		m_World->rayTest(btFrom, btTo, *callback);
		if(result)
			result->hit = true;

		b32 hasHit = callback->hasHit();

		if (!hasHit)
		{
			if(result)
				result->hit = false;

			return false;
		}
		if (result)
		{
			if (mode == RAY_CAST_MODE_FIRST_HIT)
			{
				btDynamicsWorld::ClosestRayResultCallback* closest = (btDynamicsWorld::ClosestRayResultCallback*)callback;
				result->hitEntities.SetCapacityAndElementCount(1);
				result->hitEntities.Push(closest->m_collisionObject->getUserIndex());
				result->firstHitEntity = result->hitEntities[0];
			}
			else if (mode == RAY_CAST_MODE_ALL)
			{
				btDynamicsWorld::AllHitsRayResultCallback* all = (btDynamicsWorld::AllHitsRayResultCallback*)callback;
				result->hitEntities.SetCapacityAndElementCount(all->m_collisionObjects.size());
				result->firstHitEntity = all->m_collisionObjects[0]->getUserIndex();

				for (u32 i = 0; i < all->m_collisionObjects.size(); i++)
				{
					const btCollisionObject* object = all->m_collisionObjects[i];
					result->hitEntities[i] = object->getUserIndex();
				}
			}
		}

		delete callback;

		return true;
	}

	void PhysicsEngine3D::SetGravity(r32 g)
	{
		m_World->setGravity(btVector3(0.0f, g, 0.0f));
	}

	void PhysicsEngine3D::SetGravity(const v3& gravity)
	{
		m_World->setGravity(ToBulletVector(gravity));
	}

	void PhysicsEngine3D::AddRigidBodyToWorld(btRigidBody* body)
	{
		m_World->addRigidBody(body);
		m_RigidBodies.Push(body);
	}

	void PhysicsEngine3D::RemoveRigidBodyFromWorld(btRigidBody* body)
	{
		m_World->removeRigidBody(body);
	}

	void PhysicsEngine3D::ResetRigidBodyTransforms()
	{
		for (u32 i = 0; i < m_RigidBodies.Size(); i++)
		{
			btRigidBody* rb = m_RigidBodies[i];
			rb->getWorldTransform().setIdentity();
		}
	}

	void PhysicsEngine3D::StepSimulation(r32 dt)
	{
		m_World->stepSimulation(dt);
	}

	void PhysicsEngine3D::ClearForces()
	{
		m_World->clearForces();
	}

	btVector3 PhysicsEngine3D::ToBulletVector(const v3& vec)
	{
		return btVector3(vec.x, vec.y, vec.z);
	}

	v3 PhysicsEngine3D::ToEngineVector(const btVector3& vec)
	{
		return v3(vec.getX(), vec.getY(), vec.getZ());
	}

	btQuaternion PhysicsEngine3D::ToBulletQuat(const quat& q)
	{
		return btQuaternion(q.x, q.y, q.z, q.w);
	}

	quat PhysicsEngine3D::ToEngineQuat(const btQuaternion& q)
	{
		return quat(q.getX(), q.getY(), q.getZ(), q.getW());
	}

}
