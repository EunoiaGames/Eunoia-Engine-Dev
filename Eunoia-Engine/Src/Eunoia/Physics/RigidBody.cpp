#include "RigidBody.h"
#include "../Core/Engine.h"

namespace Eunoia {

	RigidBody::RigidBody()
	{
		m_Shapes = new btCompoundShape();
		m_MotionState = new btDefaultMotionState();
		m_RigidBody = EU_NULL;
	}

	RigidBody::~RigidBody()
	{
		
	}

	void RigidBody::RemoveFromPhysicsWorld()
	{
		Engine::GetPhysicsEngine3D()->RemoveRigidBodyFromWorld(m_RigidBody);
	}

	void RigidBody::AddStaticPlaneShape(const v3& normal, r32 constant)
	{
		btStaticPlaneShape* planeShape = new btStaticPlaneShape(PhysicsEngine3D::ToBulletVector(normal), constant);
		btTransform transform;
		transform.setIdentity();
		m_Shapes->addChildShape(transform, planeShape);

		CreateRigidBody();
	}

	void RigidBody::AddSphereShape(r32 rad, const Transform3D& localTransform)
	{
		btSphereShape* sphereShape = new btSphereShape(rad);
		btTransform transform;
		transform.setOrigin(PhysicsEngine3D::ToBulletVector(localTransform.pos));
		transform.setRotation(PhysicsEngine3D::ToBulletQuat(localTransform.rot));
		m_Shapes->addChildShape(transform, sphereShape);

		CreateRigidBody();
	}

	void RigidBody::AddBoxShape(const v3& halfExtents, const Transform3D& localTransform)
	{
		btBoxShape* boxShape = new btBoxShape(PhysicsEngine3D::ToBulletVector(halfExtents));
		btTransform transform;
		transform.setOrigin(PhysicsEngine3D::ToBulletVector(localTransform.pos));
		transform.setRotation(PhysicsEngine3D::ToBulletQuat(localTransform.rot));
		m_Shapes->addChildShape(transform, boxShape);
		
		CreateRigidBody();
	}

	void RigidBody::CreateRigidBody()
	{
		if (m_RigidBody != EU_NULL)
		{
			Engine::GetPhysicsEngine3D()->RemoveRigidBodyFromWorld(m_RigidBody);
			delete m_RigidBody;
		}

		const r32 MASS = 1.0f;
		btVector3 btLocalInertia;
		m_Shapes->calculateLocalInertia(MASS, btLocalInertia);
		m_RigidBody = new btRigidBody(MASS, m_MotionState, m_Shapes, btLocalInertia);
		Engine::GetPhysicsEngine3D()->AddRigidBodyToWorld(m_RigidBody);
	}

	void RigidBody::CreateRigidBodyFromInfo(RigidBodyInfo info)
	{
		if (!m_MotionState)
			m_MotionState = new btDefaultMotionState();

		if (!m_Shapes)
			m_Shapes = new btCompoundShape();

		for (u32 i = 0; i < info.shapes.Size(); i++)
		{
			const RigidBodyShapeInfo& shapeInfo = info.shapes[i];
			switch (shapeInfo.shape)
			{
				case RIGID_BODY_SHAPE_STATIC_PLANE: {
					btStaticPlaneShape* staticPlane = new btStaticPlaneShape(PhysicsEngine3D::ToBulletVector(shapeInfo.info.xyz()), shapeInfo.info.w);
					m_Shapes->addChildShape(shapeInfo.localTransform, staticPlane);
				} break;
				case RIGID_BODY_SHAPE_SPHERE: {
					btSphereShape* sphere = new btSphereShape(shapeInfo.info.x);
					m_Shapes->addChildShape(shapeInfo.localTransform, sphere);
				} break;
				case RIGID_BODY_SHAPE_BOX: {
					btBoxShape* box = new btBoxShape(PhysicsEngine3D::ToBulletVector(shapeInfo.info.xyz()));
					m_Shapes->addChildShape(shapeInfo.localTransform, box);
				} break;
			}
		}

		btVector3 btLocalInertia = PhysicsEngine3D::ToBulletVector(info.properties.localInertia);
		if (info.properties.localInertia == v3(0.0f, 0.0f, 0.0f))
		{
			m_Shapes->calculateLocalInertia(info.properties.mass, btLocalInertia);
			info.properties.localInertia = PhysicsEngine3D::ToEngineVector(btLocalInertia);
		}
		m_RigidBody = new btRigidBody(info.properties.mass, m_MotionState, m_Shapes, btLocalInertia);
		SetProperties(info.properties);

		Engine::GetPhysicsEngine3D()->AddRigidBodyToWorld(m_RigidBody);
	}

	void RigidBody::SetMass(r32 mass) const
	{
		m_RigidBody->setMassProps(mass, m_RigidBody->getLocalInertia());
	}

	void RigidBody::SetLocalInertia(const v3& inertia) const
	{
		m_RigidBody->setMassProps(m_RigidBody->getMass(), PhysicsEngine3D::ToBulletVector(inertia));
	}

	void RigidBody::SetRestitution(r32 restitution) const
	{
		m_RigidBody->setRestitution(restitution);
	}

	void RigidBody::SetFriction(r32 friction) const
	{
		m_RigidBody->setFriction(friction);
	}

	void RigidBody::SetProperties(const RigidBodyProperties& properties) const
	{
		SetMass(properties.mass);
		SetLocalInertia(properties.localInertia);
		SetRestitution(properties.restitution);
		SetFriction(properties.friction);
	}

	void RigidBody::SetKinematic(b32 kinematic)
	{
		//m_RigidBody->setCollisionFlags(m_RigidBody->getCollisionFlags() | btCollisionObject::CF_)
	}

	void RigidBody::ApplyCentralForce(const v3& force) const
	{
		m_RigidBody->applyCentralForce(PhysicsEngine3D::ToBulletVector(force));
	}

	void RigidBody::ApplyCentralImpulse(const v3& impulse) const
	{
		m_RigidBody->applyCentralImpulse(PhysicsEngine3D::ToBulletVector(impulse));
	}

	void RigidBody::ApplyForce(const v3& force, const v3& relPos) const
	{
		m_RigidBody->applyForce(PhysicsEngine3D::ToBulletVector(force), PhysicsEngine3D::ToBulletVector(relPos));
	}

	void RigidBody::ApplyImpulse(const v3& impulse, const v3& relPos) const
	{
		m_RigidBody->applyImpulse(PhysicsEngine3D::ToBulletVector(impulse), PhysicsEngine3D::ToBulletVector(relPos));
	}

	void RigidBody::ClearForces() const
	{
		m_RigidBody->clearForces();
	}

	r32 RigidBody::GetRestitution() const
	{
		return m_RigidBody->getRestitution();
	}

	r32 RigidBody::GetFriction() const
	{
		return m_RigidBody->getFriction();
	}

	void RigidBody::SetEntityHandle(EntityID entity) const
	{
		m_RigidBody->setUserIndex(entity);
	}

	void RigidBody::GetSerializableInfo(RigidBodyInfo* info) const
	{
		info->properties.mass = GetMass();
		info->properties.localInertia = GetLocalInertia();
		info->properties.restitution = GetRestitution();
		info->properties.friction = GetFriction();

		btCompoundShape* shapes = (btCompoundShape*)m_RigidBody->getCollisionShape();
		info->shapes.SetCapacityAndElementCount(shapes->getNumChildShapes());
		for (u32 i = 0; i < shapes->getNumChildShapes(); i++)
		{
			RigidBodyShapeInfo* shapeInfo = &info->shapes[i];
			btCollisionShape* shape = shapes->getChildShape(i);
			shapeInfo->localTransform = shapes->getChildTransform(i);

			switch (shape->getShapeType())
			{
				case STATIC_PLANE_PROXYTYPE: {
					btStaticPlaneShape* staticPlaneShape = (btStaticPlaneShape*)shape;
					shapeInfo->shape = RIGID_BODY_SHAPE_STATIC_PLANE;
					shapeInfo->info = v4(PhysicsEngine3D::ToEngineVector(staticPlaneShape->getPlaneNormal()), staticPlaneShape->getPlaneConstant());
				} break;
				case SPHERE_SHAPE_PROXYTYPE: {
					btSphereShape* sphereShape = (btSphereShape*)shape;
					shapeInfo->shape = RIGID_BODY_SHAPE_SPHERE;
					shapeInfo->info.x = sphereShape->getRadius();
				} break;
				case BOX_SHAPE_PROXYTYPE: {
					btBoxShape* boxShape = (btBoxShape*)shape;
					shapeInfo->shape = RIGID_BODY_SHAPE_BOX;
					shapeInfo->info = v4(PhysicsEngine3D::ToEngineVector(boxShape->getHalfExtentsWithMargin()), 0.0f);
				} break;
			}
		}
	}

	r32 RigidBody::GetMass() const
	{
		return m_RigidBody->getMass();
	}

	v3 RigidBody::GetLocalInertia() const
	{
		return PhysicsEngine3D::ToEngineVector(m_RigidBody->getLocalInertia());
	}

	void RigidBody::SetRigidBodyAndAddToPhysicsWorld(btRigidBody* rigidBody)
	{
		m_RigidBody = rigidBody;
		Engine::GetPhysicsEngine3D()->AddRigidBodyToWorld(m_RigidBody);
	}

	btRigidBody* RigidBody::GetRigidBody()
	{
		return m_RigidBody;
	}

	btCompoundShape* RigidBody::GetShapes()
	{
		return m_Shapes;
	}

	b32 RigidBody::WasRigidBodyCreated()
	{
		return m_RigidBody != EU_NULL;
	}

	void RigidBody::DestroyRigidBody()
	{
		delete m_RigidBody;
		delete m_MotionState;
		delete m_Shapes;
		m_MotionState = new btDefaultMotionState();
		m_Shapes = new btCompoundShape();
	}

}
