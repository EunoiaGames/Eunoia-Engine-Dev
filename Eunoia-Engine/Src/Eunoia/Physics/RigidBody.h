#pragma once

#include "../Math/Math.h"
#include "../DataStructures/List.h"
#include <Bullet\btBulletCollisionCommon.h>
#include <Bullet\btBulletDynamicsCommon.h>
#include "../ECS/ECSTypes.h"

namespace Eunoia {

	EU_REFLECT()
	enum RigidBodyShapeType
	{
		RIGID_BODY_SHAPE_STATIC_PLANE,
		RIGID_BODY_SHAPE_SPHERE,
		RIGID_BODY_SHAPE_BOX,

		NUM_RIGID_BODY_SHAPES
	};

	struct RigidBodyShapeInfo
	{
		RigidBodyShapeType shape;
		btTransform localTransform;
		v4 info;
	};

	struct RigidBodyProperties
	{
		r32 mass;
		v3 localInertia;
		r32 restitution;
		r32 friction;
	};

	struct RigidBodyInfo
	{
		RigidBodyProperties properties;
		List<RigidBodyShapeInfo> shapes;
	};

	EU_REFLECT()
	class EU_API RigidBody
	{
	public:
		RigidBody();
		~RigidBody();

		void RemoveFromPhysicsWorld();

		void AddStaticPlaneShape(const v3& normal, r32 constant);
		void AddSphereShape(r32 rad, const Transform3D& localTransform = Transform3D());
		void AddBoxShape(const v3& halfExtents, const Transform3D& localTransform = Transform3D());
		void CreateRigidBodyFromInfo(RigidBodyInfo info);

		void SetMass(r32 mass) const;
		void SetLocalInertia(const v3& inertia) const;
		void SetRestitution(r32 restitution) const;
		void SetFriction(r32 friction) const;
		void SetProperties(const RigidBodyProperties& properties) const;

		void SetKinematic(b32 kinematic);

		void ApplyCentralForce(const v3& force) const;
		void ApplyCentralImpulse(const v3& impulse) const;
		void ApplyForce(const v3& force, const v3& relPos) const;
		void ApplyImpulse(const v3& impulse, const v3& relPos) const;
		void ClearForces() const;

		r32 GetMass() const;
		v3 GetLocalInertia() const;
		r32 GetRestitution() const;
		r32 GetFriction() const;

		void SetEntityHandle(EntityID entity) const;

		void GetSerializableInfo(RigidBodyInfo* info) const;

		void SetRigidBodyAndAddToPhysicsWorld(btRigidBody* rigidBody);
		btRigidBody* GetRigidBody();
		btCompoundShape* GetShapes();
		b32 WasRigidBodyCreated();
		void DestroyRigidBody();
	private:
		void CreateRigidBody();
	private:
		btRigidBody* m_RigidBody;
		btMotionState* m_MotionState;
		btCompoundShape* m_Shapes;
	};

}