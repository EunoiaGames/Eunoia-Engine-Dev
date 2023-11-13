#include "PhysicsSystem.h"
#include "../Components/Transform3DComponet.h"
#include "../Components/RigidBodyComponent.h"
#include "../../Physics/PhysicsEngine3D.h"
#include "../Events/RigidBodyTransformModifiedEvent.h"
#include "../../Rendering/Asset/AssetManager.h"
#include "../../Core/Engine.h"
namespace Eunoia {

	PhysicsSystem::PhysicsSystem()
	{
		AddComponentType<RigidBodyComponent>();
		AddComponentType<Transform3DComponent>();
	}

	void PhysicsSystem::Init()
	{
		m_BoundingSphere = AssetManager::CreateModel("Res/Models/BoundingSphere.eumdl");
		m_BoundingBox = EU_MODEL_CUBE_ID;
	}

	void PhysicsSystem::PrePhysicsSimulation(EntityID entity, r32 dt)
	{
		//if (m_ECS->CheckForEvent<RigidBodyTransformModifiedEvent>())
		//{
			btRigidBody* rigidBody = m_ECS->GetComponent<RigidBodyComponent>(entity)->body.GetRigidBody();
			Transform3D* transform = &m_ECS->GetComponent<Transform3DComponent>(entity)->worldTransform;

			if (!rigidBody)
				return;

			btTransform rt;
			rt.setOrigin(PhysicsEngine3D::ToBulletVector(transform->pos));
			rt.setRotation(PhysicsEngine3D::ToBulletQuat(transform->rot));
			rigidBody->setWorldTransform(rt);

			//m_ECS->ResetPendingEvent<RigidBodyTransformModifiedEvent>();
		//}
	}

	void PhysicsSystem::PostPhysicsSimulation(EntityID entity, r32 dt)
	{
		btRigidBody* rigidBody = m_ECS->GetComponent<RigidBodyComponent>(entity)->body.GetRigidBody();
		Transform3D* transform = &m_ECS->GetComponent<Transform3DComponent>(entity)->localTransform;
		rigidBody->setActivationState(ACTIVE_TAG);

		if (!rigidBody)
			return;

		const btTransform& rt = rigidBody->getWorldTransform();
		transform->pos = PhysicsEngine3D::ToEngineVector(rt.getOrigin());
		transform->rot = PhysicsEngine3D::ToEngineQuat(rt.getRotation());
	}

	void PhysicsSystem::ProcessEntityOnRender(EntityID entity)
	{
		RigidBodyComponent* rigidBodyComponent = m_ECS->GetComponent<RigidBodyComponent>(entity);

		if (!rigidBodyComponent->body.WasRigidBodyCreated())
			return;

		if (rigidBodyComponent->debugDraw || rigidBodyComponent->forceDraw)
		{
			const Transform3D& transform = m_ECS->GetComponent<Transform3DComponent>(entity)->worldTransform;

			btRigidBody* btRigidBody = rigidBodyComponent->body.GetRigidBody();
			btCompoundShape* shapes = (btCompoundShape*)btRigidBody->getCollisionShape();
			Renderer3D* renderer = Engine::GetRenderer()->GetRenderer3D();
			for (u32 i = 0; i < shapes->getNumChildShapes(); i++)
			{
				btCollisionShape* shape = shapes->getChildShape(i);
				const btTransform& localTransform = shapes->getChildTransform(i);
				Transform3D finalTransform(PhysicsEngine3D::ToEngineVector(localTransform.getOrigin()), v3(1.0f, 1.0f, 1.0f), PhysicsEngine3D::ToEngineQuat(localTransform.getRotation()));
				finalTransform = transform * finalTransform;

				switch (shape->getShapeType())
				{
					case SPHERE_SHAPE_PROXYTYPE: {
						const Model& sphereModel = AssetManager::GetModel(m_BoundingSphere);
						finalTransform.Scale(((btSphereShape*)shape)->getRadius());
						renderer->SubmitWireframeModel(sphereModel, finalTransform.CreateTransformMatrix());
					} break;
					case BOX_SHAPE_PROXYTYPE: {
						const Model& cubeModel = AssetManager::GetModel(m_BoundingBox);
						finalTransform.Scale(PhysicsEngine3D::ToEngineVector(((btBoxShape*)shape)->getHalfExtentsWithoutMargin()));
						renderer->SubmitWireframeModel(cubeModel, finalTransform.CreateTransformMatrix());
					} break;
				}
			}
		}
	}

}
