#pragma once

#include "../Common.h"
#include "../Metadata/Metadata.h"
#include <map>
#include "../Memory/Allocators.h"
#include "../Utils/Log.h"
#include "../DataStructures/String.h"
#include <string>
#include "ECSLoader.h"
#include "ECSTypes.h"

namespace Eunoia
{
	EU_REFLECT()
	struct ECSComponent
	{
		virtual ~ECSComponent() {}

		virtual void OnDestroy() {}

		EU_PROPERTY() b32 enabled;
		EU_PROPERTY() EntityID parent;
	};

	struct ECSComponentContainer
	{
		metadata_typeid typeID;
		ECSComponent* actualComponent;
		u32 allocatorIndex;
	};

	typedef List<ECSComponentContainer> ComponentList;

	typedef u32 SystemIndex;
	struct ECSEntityContainer
	{
		b32 enabled;
		ComponentList components;
		String name;
		List<EntityID> children;
		EntityID parent;
		List<SystemIndex> compatibleSystems;
		b32 checkedForCompatibleSystems;
	};

	EU_REFLECT()
	struct ECSEvent
	{
		EU_PROPERTY()
		r32 time;
	};

	class ECS;
	EU_REFLECT()
	class ECSSystem
	{
	public:
		ECSSystem() :
			m_NumRequiredComponents(0),
			enabled(true)
		{}

		virtual void Init() {}

		virtual ~ECSSystem() {}

		virtual void PrePhysicsSimulation(EntityID entity, r32 dt) {}
		virtual void PostPhysicsSimulation(EntityID entity, r32 dt) {}

		virtual void PreUpdate(r32 dt) {}
		virtual void ProcessEntityOnUpdate(EntityID entity, r32 dt) {}
		virtual void PostUpdate(r32 dt) {}

		virtual void PreRender() {}
		virtual void ProcessEntityOnRender(EntityID entity) {}
		virtual void PostRender() {}

		b32 enabled;
	protected:
		template<class C>
		inline void AddComponentType() { m_RequiredComponets[m_NumRequiredComponents++] = Metadata::GetTypeID<C>(); }

		friend class ECS;
		ECS* m_ECS;
		metadata_typeid m_RequiredComponets[EU_ECS_MAX_COMPONENTS_A_SYSTEM_CAN_PROCESS];
		u32 m_NumRequiredComponents;
	};

	struct ECSSystemContainer
	{
		metadata_typeid typeID;
		ECSSystem* actualSystem;
	};

	struct ECSScene
	{
		String name;
		EntityID rootEntity;
		b32 created;
		List<ECSSystemContainer> systems;
	};

	typedef List<ECSLoadedScene> ECSResetPoint;

	enum ECSProcessType
	{
		ECS_PROCESS_UPDATE,
		ECS_PROCESS_RENDER,
		ECS_PROCESS_PRE_PHYSICS_SIM,
		ECS_PROCESS_POST_PHYSICS_SIM,

		NUM_ECS_PROCESS_TYPES
	};

	class EU_API ECS
	{
	public:
		inline ECS() :
			m_NextEntityID(2),
			m_ActiveScene(EU_ECS_INVALID_SCENE_ID),
			m_SystemAllocator(32, 512),
			m_EventAllocator(EU_KB(5))
		{
			
		}

		~ECS()
		{
			for (const auto& it_cta : m_ComponentTypeAllocators)
				delete it_cta.second;
		}

		inline void Begin()
		{
			for (u32 i = 0; i < m_EventsToReset.Size(); i++)
				m_ActiveEvents[m_EventsToReset[i]] = false;
		}

		inline EntityID CreateEntity(const String& name, EntityID parent = EU_ECS_ROOT_ENTITY)
		{
			if (m_ActiveScene == EU_ECS_INVALID_SCENE_ID)
			{
				EU_LOG_WARN("You need to set a scene before creating an entity");
				return EU_ECS_INVALID_ENTITY_ID;
			}

			ECSEntityContainer container;
			container.name = name;
			container.enabled = true;
			container.parent = parent;
			container.checkedForCompatibleSystems = false;

			if (parent == EU_ECS_ROOT_ENTITY)
			{
				ECSScene* scene = &m_CreatedScenes[m_ActiveScene - 1];
				container.parent = scene->rootEntity;
			}


			EntityID createdEntityID = EU_ECS_INVALID_ENTITY_ID;
			if (!m_FreeEntityIDs.Empty())
			{
				EntityID id = m_FreeEntityIDs.GetLastElement();
				m_FreeEntityIDs.Pop();
				m_CreatedEntities[id - 2] = container;
				createdEntityID = id;
			}
			else
			{
				m_CreatedEntities.Push(container);
				createdEntityID = m_NextEntityID++;
			}

			ECSEntityContainer* parentContainer = 0;
			if (parent == EU_ECS_ROOT_ENTITY)
			{
				ECSScene* scene = &m_CreatedScenes[m_ActiveScene - 1];
				parentContainer = &m_CreatedEntities[scene->rootEntity - 2];
			}
			else if (parent != EU_ECS_INVALID_ENTITY_ID)
			{
				parentContainer = &m_CreatedEntities[parent - 2];
			}

			if (parentContainer)
				parentContainer->children.Push(createdEntityID);

			return createdEntityID;
		}

		inline EntityID CreateEntity(EntityID parent = EU_ECS_ROOT_ENTITY)
		{
			String name = "Entity_";
			if (!m_FreeEntityIDs.Empty())
				name += std::to_string(m_FreeEntityIDs[m_FreeEntityIDs.Size() - 1]).c_str();
			else
				name += std::to_string(m_NextEntityID).c_str();

			return CreateEntity(name, parent);
		}

		template<class C>
		inline b32 DoesEntityHaveComponent(EntityID entity)
		{
			return DoesEntityHaveComponent(entity, Metadata::GetTypeID<C>());
		}

		inline b32 DoesEntityHaveComponent(EntityID entity, metadata_typeid componentType)
		{
			ECSEntityContainer* container = &m_CreatedEntities[entity - 2];
			for (u32 i = 0; i < container->components.Size(); i++)
				if (container->components[i].typeID == componentType)
					return true;

			return false;
		}

		inline void SetEntityName(EntityID entity, const String& name)
		{
			if (entity > EU_ECS_ROOT_ENTITY)
				m_CreatedEntities[entity - 2].name = name;
		}

		inline void DestroyEntity(EntityID entity)
		{
			DestroyEntity(entity, &m_CreatedEntities[entity - 2]);
		}

		inline void DestroyEntity(const String& name)
		{
			for (u32 i = 0; i < m_CreatedEntities.Size(); i++)
			{
				if (m_CreatedEntities[i].name == name)
				{
					DestroyEntity(i + 2);
					return;
				}
			}
		}


		inline void SetEntityEnabled(EntityID entity, b32 enabled)
		{
			if (entity > EU_ECS_ROOT_ENTITY)
				m_CreatedEntities[entity - 2].enabled = enabled;
		}

		inline void SetEntityEnabledOpposite(EntityID entity)
		{
			if (entity > EU_ECS_ROOT_ENTITY)
				m_CreatedEntities[entity - 2].enabled = !m_CreatedEntities[entity - 2].enabled;
		}


		inline void SetEntityEnabled(const String& name, b32 enabled)
		{
			for (u32 i = 0; i < m_CreatedEntities.Size(); i++)
			{
				if (m_CreatedEntities[i].name == name)
				{
					SetEntityEnabled(i + 2, enabled);
					break;
				}
			}
		}

		inline void SetEntityEnabledOpposite(const String& name)
		{
			for (u32 i = 0; i < m_CreatedEntities.Size(); i++)
			{
				if (m_CreatedEntities[i].name == name)
				{
					SetEntityEnabledOpposite(i + 2);
					break;
				}
			}
		}

		inline b32 IsEntityEnabled(EntityID entity) const
		{
			return m_CreatedEntities[entity - 2].enabled;
		}

		inline String GetEntityName(EntityID entity) const
		{
			return m_CreatedEntities[entity - 2].name;
		}

		inline EntityID GetEntityID(const String& name) const
		{
			for (u32 i = 0; i < m_CreatedEntities.Size(); i++)
				if (m_CreatedEntities[i].name == name)
					return i + 2;
		}

		inline EntityID GetParentEntity(EntityID entity) const
		{
			return m_CreatedEntities[entity - 2].parent;
		}

		inline EntityID GetRootEntity(SceneID scene) const
		{
			return m_CreatedScenes[scene - 1].rootEntity;
		}

		inline EntityID GetChildEntity(EntityID parent, const String& childName) const
		{
			const ECSEntityContainer* ent = &m_CreatedEntities[parent - 2];
			for (u32 i = 0; i < ent->children.Size(); i++)
				if (m_CreatedEntities[ent->children[i] - 2].name == childName)
					return ent->children[i];

			return EU_ECS_INVALID_ENTITY_ID;
		}

		inline b32 DoesEntityExist(EntityID entity)
		{
			if (entity > m_CreatedEntities.Size() + 1)
				return false;

			for (u32 i = 0; i < m_FreeEntityIDs.Size(); i++)
				if (entity == m_FreeEntityIDs[i])
					return false;

			return true;
		}

		template<class C, class...Args>
		inline C* CreateComponent(EntityID entity, Args&&... args)
		{
			if (entity <= EU_ECS_ROOT_ENTITY)
				return 0;

			ECSEntityContainer* entityContainer = &m_CreatedEntities[entity - 2];
			entityContainer->checkedForCompatibleSystems = false;

			ECSComponentContainer component;
			component.typeID = Metadata::GetTypeID<C>();

			DynamicPoolAllocator* typeAllocator = 0;

			const auto&& it = m_ComponentTypeAllocators.find(component.typeID);
			if (it == m_ComponentTypeAllocators.end())
			{
				typeAllocator = new DynamicPoolAllocator(sizeof(C), 16);
				m_ComponentTypeAllocators[component.typeID] = typeAllocator;
			}
			else
			{
				typeAllocator = it->second;
			}

			component.actualComponent = (ECSComponent*)typeAllocator->Allocate(&component.allocatorIndex);
			new(component.actualComponent) C(std::forward<Args>(args)...);
			component.actualComponent->enabled = true;
			component.actualComponent->parent = entity;

			entityContainer->components.Push(component);
			return (C*)component.actualComponent;
		}

		inline ECSComponent* CreateComponent(EntityID entity, metadata_typeid componentTypeID)
		{
			if (entity <= EU_ECS_ROOT_ENTITY);

			const MetadataInfo& info = Metadata::GetMetadata(componentTypeID);

			ECSEntityContainer* entityContainer = &m_CreatedEntities[entity - 2];
			entityContainer->checkedForCompatibleSystems = false;

			ECSComponentContainer component;
			component.typeID = info.id;

			DynamicPoolAllocator* typeAllocator = 0;

			const auto&& it = m_ComponentTypeAllocators.find(component.typeID);
			if (it == m_ComponentTypeAllocators.end())
			{
				typeAllocator = new DynamicPoolAllocator(info.cls->size, 16);
				m_ComponentTypeAllocators[component.typeID] = typeAllocator;
			}
			else
			{
				typeAllocator = it->second;
			}

			component.actualComponent = (ECSComponent*)typeAllocator->Allocate(&component.allocatorIndex);
			info.cls->DefaultConstructor(component.actualComponent);
			component.actualComponent->enabled = true;
			component.actualComponent->parent = entity;

			entityContainer->components.Push(component);
			return component.actualComponent;
		}

		template<class C>
		inline b32 DestroyComponent(EntityID entity)
		{
			metadata_typeid typeID = Metadata::GetTypeID<C>();
			return DestroyComponent(entity, typeID);
		}

		inline b32 DestroyComponent(EntityID entity, metadata_typeid typeID)
		{
			if (entity <= EU_ECS_ROOT_ENTITY)
				return false;

			ECSEntityContainer* entityContainer = &m_CreatedEntities[entity - 2];
			entityContainer->compatibleSystems.Clear();
			entityContainer->checkedForCompatibleSystems = false;

			const auto&& it = m_ComponentTypeAllocators.find(typeID);
			if (it == m_ComponentTypeAllocators.end())
			{
				EU_LOG_WARN("Tried to delete ECS component with invalid component");
				return false;
			}

			List<ECSComponentContainer>& components = entityContainer->components;
			for (u32 i = 0; i < components.Size(); i++)
			{
				if (components[i].typeID == typeID)
				{
					ECSComponent* actualComponent = (ECSComponent*)components[i].actualComponent;
					actualComponent->OnDestroy();
					//actualComponent->~ECSComponent();
					it->second->Free(components[i].actualComponent, components[i].allocatorIndex);
					components.Remove(i);
					return true;
				}
			}

			EU_LOG_WARN("Tried to delete ECS component with invalid component");
			return false;
		}

		inline b32 DestroyComponentByIndex(EntityID entity, u32 componentIndex)
		{
			if (entity <= EU_ECS_ROOT_ENTITY)
				return false;

			ECSEntityContainer* entityContainer = &m_CreatedEntities[entity - 2];
			entityContainer->compatibleSystems.Clear();
			entityContainer->checkedForCompatibleSystems = false;
			ECSComponentContainer* component = &entityContainer->components[componentIndex];

			const auto&& it = m_ComponentTypeAllocators.find(component->typeID);
			if (it == m_ComponentTypeAllocators.end())
			{
				EU_LOG_WARN("Tried to delete ECS component with invalid component");
				return false;
			}
			component->actualComponent->OnDestroy();
			it->second->Free(component->actualComponent, component->allocatorIndex);
			entityContainer->components.Remove(componentIndex);
			return true;
		}

		template<class C>
		inline C* GetComponent(EntityID entity)
		{
			if (entity <= EU_ECS_ROOT_ENTITY)
				return false;

			ECSEntityContainer* entityContainer = &m_CreatedEntities[entity - 2];
			return GetComponent<C>(entityContainer->components);
		}

		inline ECSComponent* GetComponent(EntityID entity, metadata_typeid componentTypeID)
		{
			if (entity <= EU_ECS_ROOT_ENTITY)
				return false;

			ECSEntityContainer* entityContainer = &m_CreatedEntities[entity - 2];
			return GetComponent(entityContainer->components, componentTypeID);
		}

		template<class C>
		inline C* GetComponent(const ComponentList& components)
		{
			metadata_typeid typeID = Metadata::GetTypeID<C>();
			for (u32 i = 0; i < components.Size(); i++)
			{
				const ECSComponentContainer* component = &components[i];
				if (component->typeID == typeID)
					return (C*)component->actualComponent;
			}

			return 0;
		}

		inline ECSComponent* GetComponent(const ComponentList& components, metadata_typeid typeID)
		{
			for (u32 i = 0; i < components.Size(); i++)
			{
				const ECSComponentContainer* component = &components[i];
				if (component->typeID == typeID)
					return component->actualComponent;
			}

			return 0;
		}

		inline ECSComponent* GetComponentByIndex(EntityID entity, u32 index)
		{
			return m_CreatedEntities[entity - 2].components[index].actualComponent;
		}

		template<class C>
		inline void SetComponentEnabled(EntityID entity, b32 enabled)
		{
			SetComponentEnabled(entity, Metadata::GetTypeID<C>());
		}

		template<class C>
		inline void SetComponentEnabledOpposite(EntityID entity)
		{
			SetComponentEnabledOpposite(entity, Metadata::GetTypeID<C>());
		}

		inline void SetComponentEnabled(EntityID entity, metadata_typeid typeID, b32 enabled)
		{
			if (entity <= EU_ECS_ROOT_ENTITY)
				return;

			ECSEntityContainer* entityContainer = &m_CreatedEntities[entity - 2];

			List<ECSComponentContainer>& components = entityContainer->components;
			for (u32 i = 0; i < components.Size(); i++)
			{
				if (components[i].typeID == typeID)
					components[i].actualComponent->enabled = enabled;
			}
		}

		inline void SetComponentEnabledOpposite(EntityID entity, metadata_typeid typeID)
		{
			if (entity <= EU_ECS_ROOT_ENTITY)
				return;

			ECSEntityContainer* entityContainer = &m_CreatedEntities[entity - 2];

			List<ECSComponentContainer>& components = entityContainer->components;
			for (u32 i = 0; i < components.Size(); i++)
			{
				if (components[i].typeID == typeID)
					components[i].actualComponent->enabled = !components[i].actualComponent->enabled;
			}
		}

		template<class S, class... Args>
		inline S* CreateSystem(Args&& ... args)
		{
			if (m_ActiveScene == EU_ECS_INVALID_SCENE_ID)
			{
				EU_LOG_WARN("You must set an active scene before creating a system");
				return 0;
			}

			ECSScene* scene = &m_CreatedScenes[m_ActiveScene - 1];

			ECSSystemContainer system;
			system.typeID = Metadata::GetTypeID<S>();

			if (sizeof(S) > 512)
			{
				EU_LOG_FATAL("This system is to big. TODO: Allow for bigger systems...");
				return 0;
			}

			system.actualSystem = (ECSSystem*)m_SystemAllocator.Allocate();
			new(system.actualSystem) S(std::forward<Args>(args)...);
			system.actualSystem->m_ECS = this;
			system.actualSystem->enabled = true;
			system.actualSystem->Init();
			SystemIndex index = scene->systems.Size();
			scene->systems.Push(system);

			for (u32 i = 0; i < m_CreatedEntities.Size(); i++)
			{
				ECSEntityContainer* container = &m_CreatedEntities[i];
				CheckEntityAndSystemCompatiblity(container, index);
			}

			return (S*)system.actualSystem;
		}

		inline ECSSystem* CreateSystem(metadata_typeid typeID, b32 enabled = true)
		{
			if (m_ActiveScene == EU_ECS_INVALID_SCENE_ID)
			{
				EU_LOG_WARN("You must set an active scene before creating a system");
				return 0;
			}

			ECSScene* scene = &m_CreatedScenes[m_ActiveScene - 1];

			ECSSystemContainer system;
			system.typeID = typeID;

			if (Metadata::GetMetadata(typeID).cls->size > 512)
			{
				EU_LOG_FATAL("This system is to big. TODO: Allow for bigger systems...");
				return 0;
			}

			system.actualSystem = (ECSSystem*)m_SystemAllocator.Allocate();
			Metadata::CallDefaultConstructor(typeID, system.actualSystem);
			system.actualSystem->m_ECS = this;
			system.actualSystem->enabled = enabled;
			system.actualSystem->Init();
			SystemIndex index = scene->systems.Size();
			scene->systems.Push(system);

			for (u32 i = 0; i < m_CreatedEntities.Size(); i++)
			{
				ECSEntityContainer* container = &m_CreatedEntities[i];
				CheckEntityAndSystemCompatiblity(container, index);
			}

			return system.actualSystem;
		}

		template<class S>
		inline S* GetSystem()
		{
			return (S*)GetSystem(Metadata::GetTypeID<S>());
		}

		inline ECSSystem* GetSystem(metadata_typeid typeID)
		{
			if (m_ActiveScene == EU_ECS_INVALID_SCENE_ID)
			{
				EU_LOG_WARN("You must set an active scene before getting a system");
				return 0;
			}

			ECSScene* scene = &m_CreatedScenes[m_ActiveScene - 1];

			for (u32 i = 0; i < scene->systems.Size(); i++)
				if (scene->systems[i].typeID == typeID)
					return scene->systems[i].actualSystem;
			return 0;
		}

		template<class S>
		inline void DestroySystem()
		{
			DestroySystem(Metadata::GetTypeID<S>());
		}

		inline void DestroySystem(metadata_typeid typeID)
		{
			if (m_ActiveScene == EU_ECS_INVALID_SCENE_ID)
			{
				EU_LOG_WARN("You must set an active scene before destroying a system");
				return;
			}

			ECSScene* scene = &m_CreatedScenes[m_ActiveScene - 1];

			for (u32 i = 0; i < scene->systems.Size(); i++)
			{
				ECSSystemContainer* system = &scene->systems[i];
				if (system->typeID == typeID)
				{
					ECSSystem* actualSystem = (ECSSystem*)system->actualSystem;
					m_SystemAllocator.Free(actualSystem);
					scene->systems.Remove(i);
					return;
				}
			}

			for (u32 i = 0; i < m_CreatedEntities.Size(); i++)
			{
				ECSEntityContainer* entity = &m_CreatedEntities[i];
				entity->compatibleSystems.Clear();
				entity->checkedForCompatibleSystems = false;
			}

			EU_LOG_WARN("Unable to destroy ECS system");
		}

		inline void DestroySystemByIndex(u32 systemIndex)
		{
			if (m_ActiveScene == EU_ECS_INVALID_SCENE_ID)
			{
				EU_LOG_WARN("You must set an active scene before destroying a system");
				return;
			}

			ECSScene* scene = &m_CreatedScenes[m_ActiveScene - 1];

			ECSSystemContainer* system = &scene->systems[systemIndex];
			m_SystemAllocator.Free(system->actualSystem);
			scene->systems.Remove(systemIndex);

			for (u32 i = 0; i < m_CreatedEntities.Size(); i++)
			{
				ECSEntityContainer* entity = &m_CreatedEntities[i];
				entity->compatibleSystems.Clear();
				entity->checkedForCompatibleSystems = false;
			}
		}

		template<class S>
		inline b32 SetSystemEnabled(b32 enabled)
		{
			return SetSystemEnabled(Metadata::GetTypeID<S>(), enabled);
		}

		inline b32 SetSystemEnabled(metadata_typeid typeID, b32 enabled)
		{
			if (m_ActiveScene == EU_ECS_INVALID_SCENE_ID)
			{
				EU_LOG_WARN("You must set an active scene before enabling a system");
				return false;
			}

			ECSScene* scene = &m_CreatedScenes[m_ActiveScene - 1];

			for (u32 i = 0; i < scene->systems.Size(); i++)
			{
				ECSSystemContainer* system = &scene->systems[i];
				if (system->typeID == typeID)
				{
					ECSSystem* actualSystem = (ECSSystem*)system->actualSystem;
					actualSystem->enabled = enabled;
					return true;
				}
			}

			return false;
		}

		template<class S>
		inline b32 SetSystemEnabledOpposite()
		{
			return SetSystemEnabledOpposite(Metadata::GetTypeID<S>());
		}

		inline b32 SetSystemEnabledOpposite(metadata_typeid typeID)
		{
			if(m_ActiveScene == EU_ECS_INVALID_SCENE_ID)
			{
				EU_LOG_WARN("You must set an active scene before enabling a system");
				return false;
			}

			ECSScene* scene = &m_CreatedScenes[m_ActiveScene - 1];

			for (u32 i = 0; i < scene->systems.Size(); i++)
			{
				ECSSystemContainer* system = &scene->systems[i];
				if (system->typeID == typeID)
				{
					ECSSystem* actualSystem = (ECSSystem*)system->actualSystem;
					actualSystem->enabled = !actualSystem->enabled;
					return true;
				}
			}

			return false;
		}

		template<class E, class... Args>
		inline void DispatchEvent(Args&&... args)
		{
			metadata_typeid typeID = Metadata::GetTypeID<E>();
			const auto&& it = m_Events.find(typeID);
			if (it == m_Events.end())
			{
				ECSEvent* ecsEvent = (ECSEvent*)m_EventAllocator.Allocate(sizeof(E));
				new(ecsEvent) E(std::forward<Args>(args)...);
				ecsEvent->time = Engine::GetTime();
				m_Events[typeID] = ecsEvent;

				m_ActiveEvents[typeID] = true;
			}
			else
			{
				ECSEvent* ecsEvent = m_Events[typeID];
				new(m_Events[typeID]) E(std::forward<Args>(args)...);
				ecsEvent->time = Engine::GetTime();
				m_ActiveEvents[typeID] = true;
			}

			m_EventsToReset.Push(typeID);
		}

		template<class E, class... Args>
		inline void DispatchPendingEvent(Args&&... args)
		{
			metadata_typeid typeID = Metadata::GetTypeID<E>();
			const auto&& it = m_Events.find(typeID);
			if (it == m_Events.end())
			{
				ECSEvent* ecsEvent = (ECSEvent*)m_EventAllocator.Allocate(sizeof(E));
				new(ecsEvent) E(std::forward<Args>(args)...);
				ecsEvent->time = Engine::GetTime();
				m_Events[typeID] = ecsEvent;
				m_ActiveEvents[typeID] = true;
			}
			else
			{
				ECSEvent* ecsEvent = m_Events[typeID];
				new(ecsEvent) E(std::forward<Args>(args)...);
				ecsEvent->time = Engine::GetTime();
				m_ActiveEvents[typeID] = true;
			}
		}

		template<class E>
		inline void ResetPendingEventNextFrame()
		{
			metadata_typeid typeID = Metadata::GetTypeID<E>();
			m_EventsToReset.Push(typeID);
		}

		template<class E>
		inline void ResetPendingEvent()
		{
			metadata_typeid typeID = Metadata::GetTypeID<E>();
			m_ActiveEvents[typeID] = false;
		}

		template<class E>
		inline ECSEvent* CheckForEvent()
		{
			metadata_typeid typeID = Metadata::GetTypeID<E>();
			const auto&& it_event = m_Events.find(typeID);
			if (it_event == m_Events.end())
				return 0;

			const auto&& it = m_ActiveEvents.find(typeID);
			if (it == m_ActiveEvents.end())
				return 0;

			if (!it->second)
				return 0;

			return it_event->second;
		}

		inline SceneID CreateScene(const String& name, b32 setActive = false, b32 addRequiredSystems = true)
		{
			ECSScene scene;
			scene.name = name;
			scene.created = true;

			SceneID activeScene = m_ActiveScene;

			SceneID sceneID;
			if (!m_FreeSceneIDs.Empty())
			{
				sceneID = m_FreeSceneIDs.Pop();
				m_CreatedScenes[sceneID - 1] = scene;
				SetActiveScene(sceneID);
				m_CreatedScenes[sceneID - 1].rootEntity = CreateEntity("Root", EU_ECS_INVALID_ENTITY_ID);
			}
			else
			{
				m_CreatedScenes.Push(scene);
				sceneID = m_CreatedScenes.Size();
				SetActiveScene(sceneID);
				m_CreatedScenes[sceneID - 1].rootEntity = CreateEntity("Root", EU_ECS_INVALID_ENTITY_ID);
			}

			if (addRequiredSystems)
				AddRequiredSystems();

			if (!setActive)
				SetActiveScene(activeScene);

			return sceneID;
		}

		void AddRequiredSystems();

		inline void SetActiveScene(SceneID scene)
		{
			m_ActiveScene = scene;
		}

		inline void DestroyScene(SceneID sceneID)
		{
			if (sceneID == EU_ECS_INVALID_SCENE_ID)
				return;

			ECSScene* scene = &m_CreatedScenes[sceneID - 1];
			scene->created = false;
			DestroyEntity(scene->rootEntity);
			m_FreeSceneIDs.Push(sceneID);
		}

		inline void SetSceneName(SceneID scene, const String& name)
		{
			m_CreatedScenes[scene - 1].name = name;
		}

		inline SceneID GetSceneID(const String& name) const
		{
			for (u32 i = 0; i < m_CreatedScenes.Size(); i++)
				if (m_CreatedScenes[i].name == name)
					return (SceneID)(i + 1);

			return EU_ECS_INVALID_ID;
		}

		inline String GetSceneName(SceneID scene)
		{
			return m_CreatedScenes[scene - 1].name;
		}

		inline EntityID GetRootEntity()
		{
			return m_CreatedScenes[m_ActiveScene - 1].rootEntity;
		}

		inline String GetActiveSceneName()
		{
			if (m_ActiveScene == EU_ECS_INVALID_SCENE_ID)
				return "No Selected Scene";

			return m_CreatedScenes[m_ActiveScene - 1].name;
		}

		inline SceneID GetActiveScene() { return m_ActiveScene; }

		inline b32 HasActiveScene() { return m_ActiveScene; }

		inline List<ECSScene>& GetAllScenes_() { return m_CreatedScenes; }
		inline List<ECSSystemContainer>& GetSystems_() { return m_CreatedScenes[m_ActiveScene - 1].systems; }
		inline List<ECSEntityContainer>& GetAllEntities_() { return m_CreatedEntities; }

		inline void UpdateSystems(r32 dt)
		{
			//ECSScene* gs = &m_CreatedScenes[m_SceneStack[m_SceneStack.Size() - 1]];
			//ProcessSystemsHelper(gs, 0, dt);
			ProcessEntities(ECS_PROCESS_UPDATE, dt);
		}

		inline void RenderSystems()
		{
			//ECSScene* gs = &m_CreatedScenes[m_SceneStack[m_SceneStack.Size() - 1]];
			//ProcessSystemsHelper(gs, 1, 0.0f);
			ProcessEntities(ECS_PROCESS_RENDER);
		}

		inline void PrePhysicsSystems(r32 dt)
		{
			//ECSScene* gs = &m_CreatedScenes[m_SceneStack[m_SceneStack.Size() - 1]];
			//ProcessSystemsHelper(gs, 2, dt);
			ProcessEntities(ECS_PROCESS_PRE_PHYSICS_SIM);
		}

		inline void PostPhysicsSystems(r32 dt)
		{
			//ECSScene* gs = &m_CreatedScenes[m_SceneStack[m_SceneStack.Size() - 1]];
			//ProcessSystemsHelper(gs, 3, dt);
			ProcessEntities(ECS_PROCESS_POST_PHYSICS_SIM);
		}

		inline void CreateResetPoint(ECSResetPoint* resetPoint)
		{
			resetPoint->Clear();
			for (u32 i = 0; i < m_CreatedScenes.Size(); i++)
			{
				ECSLoadedScene loadedScene;
				ConvertSceneToLoadedDataFormat(&loadedScene, (SceneID)(i + 1));
				resetPoint->Push(loadedScene);
			}
		}

		inline void RestoreResetPoint(const ECSResetPoint& resetPoint)
		{
			FreeAndResetECS();
			for (u32 i = 0; i < resetPoint.Size(); i++)
			{
				LoadSceneFromLoadedDataFormat(resetPoint[i]);
			}
		}

		inline void ConvertSceneToLoadedDataFormat(ECSLoadedScene* loadedScene)
		{
			ConvertSceneToLoadedDataFormat(loadedScene, m_ActiveScene);
		}

		inline void ConvertSceneToLoadedDataFormat(ECSLoadedScene* loadedScene, SceneID sceneID)
		{
			ECSScene* scene = &m_CreatedScenes[sceneID - 1];

			loadedScene->name = scene->name;
			loadedScene->entities.Clear();
			loadedScene->systems.Clear();

			for (u32 i = 0; i < scene->systems.Size(); i++)
			{
				const ECSSystemContainer& system = scene->systems[i];
				ECSLoadedSystem loadedSystem;
				loadedSystem.enabled = system.actualSystem->enabled;
				loadedSystem.typeID = system.typeID;

				mem_size systemSize = Metadata::GetMetadata(system.typeID).cls->size;
				mem_size dataSize = systemSize - sizeof(ECSSystem);
				if (dataSize > 0)
				{
					loadedSystem.data.SetCapacityAndElementCount(systemSize);
					memcpy(&loadedSystem.data[0], (u8*)system.actualSystem + sizeof(ECSSystem), dataSize);
				}

				loadedScene->systems.Push(loadedSystem);
			}

			ECSEntityContainer* rootEntity = &m_CreatedEntities[scene->rootEntity - 2];
			ConvertSceneEntityToLoadedDataFormat(loadedScene, rootEntity);
		}

		inline void ConvertSceneEntityToLoadedDataFormat(ECSLoadedScene* loadedScene, ECSEntityContainer* entity)
		{
			ECSLoadedEntity loadedEntity;
			loadedEntity.name = entity->name;
			loadedEntity.enabled = entity->enabled;
			loadedEntity.numChildren = entity->children.Size();

			for (u32 i = 0; i < entity->components.Size(); i++)
			{
				ECSComponentContainer* component = &entity->components[i];
				ECSLoadedComponent loadedComponent;
				loadedComponent.typeID = component->typeID;
				
				mem_size componentSize = Metadata::GetMetadata(component->typeID).cls->size;
				mem_size dataSize = componentSize - sizeof(ECSComponent);
				if (dataSize > 0)
				{
					loadedComponent.data.SetCapacityAndElementCount(dataSize);
					memcpy(&loadedComponent.data[0], (u8*)component->actualComponent + sizeof(ECSComponent), dataSize);
				}

				loadedEntity.components.Push(loadedComponent);
			}

			loadedScene->entities.Push(loadedEntity);

			for (u32 i = 0; i < entity->children.Size(); i++)
				ConvertSceneEntityToLoadedDataFormat(loadedScene, &m_CreatedEntities[entity->children[i] - 2]);
		}

		inline SceneID LoadSceneFromLoadedDataFormat(const ECSLoadedScene& loadedScene, b32 setActive = false)
		{
			SceneID activeScene = m_ActiveScene;
			SceneID newScene = CreateScene(loadedScene.name, true, false);

			for (u32 i = 0; i < loadedScene.systems.Size(); i++)
			{
				const ECSLoadedSystem& loadedSystem = loadedScene.systems[i];
				ECSSystem* system = CreateSystem(loadedSystem.typeID, loadedSystem.enabled);
				MetadataClass* systemMetadata = Metadata::GetMetadata(loadedSystem.typeID).cls;
				mem_size systemSize = systemMetadata->size - sizeof(ECSSystem);
				if (systemSize > 0 && !systemMetadata->members.Empty())
					memcpy((u8*)system + sizeof(ECSSystem), &loadedSystem.data[0], systemSize);
				system->enabled = loadedSystem.enabled;
				system->Init();
			}

			if (!loadedScene.entities.Empty())
			{
				u32 entityIndex = 0;
				LoadSceneEntityFromLoadedDataFormat(loadedScene.entities, entityIndex, EU_ECS_INVALID_ENTITY_ID);
			}

			if (!setActive)
				SetActiveScene(activeScene);

			return newScene;
		}

		inline void LoadSceneEntityFromLoadedDataFormat(const List<ECSLoadedEntity>& loadedEntities, u32& entityIndex, EntityID parent)
		{
			const ECSLoadedEntity& loadedEntity = loadedEntities[entityIndex];

			EntityID entity;
			if (entityIndex == 0)
				entity = GetRootEntity();
			else
				entity = CreateEntity(loadedEntity.name, parent);

			SetEntityEnabled(entity, loadedEntity.enabled);

			for (u32 i = 0; i < loadedEntity.components.Size(); i++)
			{
				const ECSLoadedComponent& loadedComponent = loadedEntity.components[i];
				ECSComponent* component = CreateComponent(entity, loadedComponent.typeID);
				mem_size componentSize = Metadata::GetMetadata(loadedComponent.typeID).cls->size - sizeof(ECSComponent);
				if (componentSize > 0)
					memcpy((u8*)component + sizeof(ECSComponent), &loadedComponent.data[0], componentSize);
				component->enabled = loadedComponent.enabled;
			}

			entityIndex++;

			for (u32 i = 0; i < loadedEntity.numChildren; i++)
			{
				LoadSceneEntityFromLoadedDataFormat(loadedEntities, entityIndex, entity);
			}
		}

		/*inline void ConvertToLoadedDataFormat(ECSLoadedData* loadedData)
		{
			loadedData->entities.Clear();
			loadedData->scenes.Clear();
			loadedData->systems.Clear();

			for (u32 i = 0; i < m_Systems.Size(); i++)
			{
				const ECSSystemContainer& system = m_Systems[i];
				ECSLoadedSystem loadedSystem;
				loadedSystem.enabled = system.actualSystem->enabled;
				loadedSystem.typeID = system.typeID;
				
				mem_size systemSize = Metadata::GetMetadata(system.typeID).cls->size;
				mem_size dataSize = systemSize - sizeof(ECSSystem);
				if (dataSize > 0)
				{
					loadedSystem.data.SetCapacityAndElementCount(systemSize);
					memcpy(&loadedSystem.data[0], (u8*)system.actualSystem + sizeof(ECSSystem), dataSize);
				}

				loadedData->systems.Push(loadedSystem);
			}

			for (u32 i = 0; i < m_CreatedEntities.Size(); i++)
			{
				const ECSEntityContainer& entity = m_CreatedEntities[i];
				ECSLoadedEntity loadedEntity;
				loadedEntity.name = entity.name;
				loadedEntity.enabled = entity.enabled;
				if(!entity.components.Empty())
					loadedEntity.components.SetCapacityAndElementCount(entity.components.Size());

				for (u32 j = 0; j < entity.components.Size(); j++)
				{
					const ECSComponentContainer& component = entity.components[j];
					ECSLoadedComponent* loadedComponent = &loadedEntity.components[j];
					loadedComponent->enabled = component.actualComponent->enabled;
					loadedComponent->typeID = component.typeID;
					
					mem_size componentSize = Metadata::GetMetadata(component.typeID).cls->size;
					mem_size dataSize = componentSize - sizeof(ECSComponent);
					if (dataSize > 0)
					{
						loadedComponent->data.SetCapacityAndElementCount(dataSize);
						memcpy(&loadedComponent->data[0], (u8*)component.actualComponent + sizeof(ECSComponent), dataSize);
					}
				}
				
				for (u32 j = 0; j < entity.children.Size(); j++)
					loadedEntity.children.Push(entity.children[j] - 2);

				loadedData->entities.Push(loadedEntity);
			}

			for (u32 i = 0; i < m_CreatedScenes.Size(); i++)
			{
				const ECSScene& scene = m_CreatedScenes[i];
				if (!scene.created)
					continue;

				ECSLoadedScene loadedScene;
				loadedScene.name = scene.name;
				loadedScene.rootEntity = scene.rootEntity - 2;

				loadedData->scenes.Push(loadedScene);
			}
		}

		inline void InitFromLoadedDataFormat(const ECSLoadedData& loadedData, b32 freeData = true)
		{
			if(freeData)
				FreeAndResetECS();

			for (u32 i = 0; i < loadedData.scenes.Size(); i++)
			{
				SceneID sceneID;

				const ECSLoadedScene& loadedScene = loadedData.scenes[i];

				sceneID = CreateScene(loadedScene.name);

				const ECSLoadedEntity& loadedRootEntity = loadedData.entities[loadedScene.rootEntity];
				InitEntityFromLoadedData(loadedData, loadedRootEntity, 0, sceneID);
			}

			for (u32 i = 0; i < loadedData.systems.Size(); i++)
			{
				const ECSLoadedSystem& loadedSystem = loadedData.systems[i];
				ECSSystem* system = CreateSystem(loadedSystem.typeID, loadedSystem.enabled);
				MetadataClass* systemMetadata = Metadata::GetMetadata(loadedSystem.typeID).cls;
				mem_size systemSize = systemMetadata->size - sizeof(ECSSystem);
				if(systemSize > 0 && !systemMetadata->members.Empty())
					memcpy((u8*)system + sizeof(ECSSystem), &loadedSystem.data[0], systemSize);
				system->enabled = loadedSystem.enabled;
				system->Init();
			}
		}*/

		inline void OnlyUpdateRequiredSystems(r32 dt)
		{
			ECSScene* scene = &m_CreatedScenes[m_ActiveScene - 1];
			for (u32 i = 0; i < m_CreatedEntities.Size(); i++)
			{
				ECSEntityContainer* entity = &m_CreatedEntities[i];
				if (!entity->checkedForCompatibleSystems)
				{
					for (u32 j = 0; j < scene->systems.Size(); j++)
						CheckEntityAndSystemCompatiblity(entity, j);

					entity->checkedForCompatibleSystems = true;
				}

				for (u32 j = 0; j < entity->compatibleSystems.Size(); j++)
				{
					if (entity->compatibleSystems[j] == 0 ||
						entity->compatibleSystems[j] == 1)
						scene->systems[entity->compatibleSystems[j]].actualSystem->ProcessEntityOnUpdate(i + 2, dt);
				}
			}
		}

	private:
		inline void DestroyEntitiesInList()
		{
			for (u32 i = 0; i < m_EntitiesToDelete.Size(); i++)
				DestroyEntityHelper2(m_EntitiesToDelete[i]);

			m_EntitiesToDelete.Clear();
		}

		inline void DestroyEntityHelper2(EntityID entity)
		{
			if (entity <= EU_ECS_ROOT_ENTITY)
				return;

			ECSEntityContainer& container = m_CreatedEntities[entity - 2];

			if (container.parent != EU_ECS_INVALID_ENTITY_ID)
			{
				ECSEntityContainer* parent = &m_CreatedEntities[container.parent - 2];
				for (u32 i = 0; i < parent->children.Size(); i++)
				{
					if (parent->children[i] == entity)
					{
						parent->children.Remove(i);
						break;
					}
				}
			}

			u32 startIndex = m_FreeEntityIDs.Size();
			DestroyEntityHelper(entity);
			RemoveDestroyChildEntities(entity, startIndex);
		}

		inline void DestroyEntityHelper(EntityID entity)
		{
			ECSEntityContainer* container = &m_CreatedEntities[entity - 2];
			DestroyEntity(entity, container);

			for (u32 i = 0; i < container->children.Size(); i++)
				DestroyEntityHelper(container->children[i]);
		}

		inline void RemoveDestroyChildEntities(EntityID entity, u32 startIndex)
		{
			ECSEntityContainer* container = &m_CreatedEntities[entity - 2];
			for (u32 i = 0; i < container->children.Size(); i++)
			{
				for (u32 j = startIndex; j < m_FreeEntityIDs.Size(); j++)
				{
					if (container->children[i] == m_FreeEntityIDs[j])
					{
						container->children.Remove(i);
						i--;
						break;
					}
				}
			}

			for (u32 i = 0; i < container->children.Size(); i++)
				RemoveDestroyChildEntities(container->children[i], startIndex);
		}

		inline void CheckEntityAndSystemCompatiblity(ECSEntityContainer* entity, SystemIndex systemIndex)
		{
			ECSScene* scene = &m_CreatedScenes[m_ActiveScene - 1];

			b32 systemCompatible = true;
			ECSSystemContainer* system = &scene->systems[systemIndex];
			for (u32 i = 0; i < system->actualSystem->m_NumRequiredComponents; i++)
			{
				b32 requiredComponentFound = false;
				for (u32 j = 0; j < entity->components.Size(); j++)
				{
					if (system->actualSystem->m_RequiredComponets[i] == entity->components[j].typeID)
					{
						requiredComponentFound = true;
						break;
					}
				}

				if (!requiredComponentFound)
				{
					systemCompatible = false;
					break;
				}
			}

			if (systemCompatible)
			{
				b32 systemAlreadyInList = false;
				for (u32 i = 0; i < entity->compatibleSystems.Size(); i++)
				{
					if (entity->compatibleSystems[i] == systemIndex)
					{
						systemAlreadyInList = true;
						break;
					}
				}

				if(!systemAlreadyInList)
					entity->compatibleSystems.Push(systemIndex);
			}
		}

		inline void ProcessEntity(EntityID entityID, ECSProcessType processType, r32 dt)
		{
			ECSScene* scene = &m_CreatedScenes[m_ActiveScene - 1];
			ECSEntityContainer* entity = &m_CreatedEntities[entityID - 2];

			if (!entity->checkedForCompatibleSystems)
			{
				for (u32 j = 0; j < scene->systems.Size(); j++)
					CheckEntityAndSystemCompatiblity(entity, j);

				entity->checkedForCompatibleSystems = true;
			}

			if (!entity->enabled)
				return;

			for (u32 j = 0; j < entity->compatibleSystems.Size(); j++)
			{
				SystemIndex index = entity->compatibleSystems[j];
				ECSSystemContainer* system = &scene->systems[index];

				if (!system->actualSystem->enabled)
					continue;

				if (processType == ECS_PROCESS_UPDATE)
					system->actualSystem->ProcessEntityOnUpdate(entityID, dt);
				else if (processType == ECS_PROCESS_RENDER)
					system->actualSystem->ProcessEntityOnRender(entityID);
				else if (processType == ECS_PROCESS_PRE_PHYSICS_SIM)
					system->actualSystem->PrePhysicsSimulation(entityID, dt);
				else if (processType == ECS_PROCESS_POST_PHYSICS_SIM)
					system->actualSystem->PostPhysicsSimulation(entityID, dt);
			}

			for (u32 i = 0; i < entity->children.Size(); i++)
				ProcessEntity(entity->children[i], processType, dt);
		}

		inline void ProcessEntities(ECSProcessType processType, r32 dt = 0.0f)
		{
			if (m_ActiveScene == EU_ECS_INVALID_SCENE_ID)
				return;

			ECSScene* scene = &m_CreatedScenes[m_ActiveScene - 1];

			for (u32 i = 0; i < scene->systems.Size(); i++)
			{
				ECSSystemContainer* system = &scene->systems[i];

				if (!system->actualSystem->enabled)
					continue;

				switch (processType)
				{
				case ECS_PROCESS_UPDATE: system->actualSystem->PreUpdate(dt); break;
				case ECS_PROCESS_RENDER: system->actualSystem->PreRender(); break;
				}
			}

			ProcessEntity(scene->rootEntity, processType, dt);

			for (u32 i = 0; i < scene->systems.Size(); i++)
			{
				ECSSystemContainer* system = &scene->systems[i];

				if (!system->actualSystem->enabled)
					continue;

				switch (processType)
				{
				case ECS_PROCESS_UPDATE: system->actualSystem->PostUpdate(dt); break;
				case ECS_PROCESS_RENDER: system->actualSystem->PostRender(); break;
				}
			}
		}

		/*inline void InitEntityFromLoadedData(const ECSLoadedData& loadedData, const ECSLoadedEntity& loadedEntity, EntityID parent, SceneID fromScene)
		{
			EntityID entityID = EU_ECS_INVALID_ENTITY_ID;

			SceneID activeScene = m_ActiveScene;
			SetActiveScene(fromScene);

			if (parent != EU_ECS_INVALID_ENTITY_ID)
				entityID = CreateEntity(loadedEntity.name, parent);
			else
				entityID = GetRootEntity(fromScene); //Root entity was already created when loading scene data

			SetActiveScene(activeScene);

			ECSEntityContainer* entity = &m_CreatedEntities[entityID - 2];
			entity->enabled = loadedEntity.enabled;
			entity->name = loadedEntity.name;
			entity->parent = parent;

			for (u32 i = 0; i < loadedEntity.components.Size(); i++)
			{
				const ECSLoadedComponent& loadedComponent = loadedEntity.components[i];
				ECSComponent* component = CreateComponent(entityID, loadedComponent.typeID);
				mem_size componentSize = Metadata::GetMetadata(loadedComponent.typeID).cls->size - sizeof(ECSComponent);
				if(componentSize > 0)
					memcpy((u8*)component + sizeof(ECSComponent), &loadedComponent.data[0], componentSize);
				component->enabled = loadedComponent.enabled;
			}
	

			for (u32 i = 0; i < loadedEntity.children.Size(); i++)
			{
				const ECSLoadedEntity& loadedChildEntity = loadedData.entities[loadedEntity.children[i]];
				InitEntityFromLoadedData(loadedData, loadedChildEntity, entityID, fromScene);
			}
		}*/

		inline void DestroyEntity(EntityID id, ECSEntityContainer* entity)
		{
			const List<ECSComponentContainer>& components = entity->components;
			for (u32 i = 0; i < components.Size(); i++)
			{
				const ECSComponentContainer* component = &components[i];
				const auto&& it = m_ComponentTypeAllocators.find(component->typeID);
				if (it == m_ComponentTypeAllocators.end())
				{
					EU_LOG_WARN("Tried to delete an invalid component");
					continue;
				}
				//component->actualComponent->~ECSComponent();
				component->actualComponent->OnDestroy();
				it->second->Free(component->actualComponent, component->allocatorIndex);
			}

			if (entity->parent != EU_ECS_INVALID_ENTITY_ID)
			{
				ECSEntityContainer* parent = &m_CreatedEntities[entity->parent - 2];
				for (u32 i = 0; i < parent->children.Size(); i++)
				{
					if (parent->children[i] == id)
						parent->children.Remove(i);
				}
			}

			entity->children.Clear();
			entity->components.Clear();
			entity->enabled = false;
			entity->name = "";
			entity->parent = EU_ECS_INVALID_ENTITY_ID;
			m_FreeEntityIDs.Push(id);
		}

		inline void FreeAndResetECS()
		{
			for (const auto& it_cta : m_ComponentTypeAllocators)
				delete it_cta.second;

			m_EventAllocator.Reset();
			m_SystemAllocator.Reset();

			m_CreatedEntities.Clear();
			m_CreatedScenes.Clear();
			m_FreeSceneIDs.Clear();
			m_FreeEntityIDs.Clear();
			m_NextEntityID = 2;
			m_ComponentTypeAllocators.clear();
			m_Events.clear();
			m_ActiveEvents.clear();
			m_EventsToReset.Clear();
		}
	private:
		List<ECSEntityContainer> m_CreatedEntities;
		List<ECSScene> m_CreatedScenes;
		List<SceneID> m_FreeSceneIDs;
		SceneID m_ActiveScene;
		List < EntityID > m_FreeEntityIDs;
		EntityID m_NextEntityID;
		std::map < metadata_typeid, DynamicPoolAllocator* > m_ComponentTypeAllocators;
		PoolAllocator m_SystemAllocator;
		LinearAllocator m_EventAllocator;
		std::map<metadata_typeid, ECSEvent*> m_Events;
		std::map<metadata_typeid, b32> m_ActiveEvents;
		List<metadata_typeid> m_EventsToReset;
		List<EntityID> m_EntitiesToDelete;
	};
}
