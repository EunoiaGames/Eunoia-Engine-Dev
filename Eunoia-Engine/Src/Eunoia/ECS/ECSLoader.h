#pragma once

#include "../DataStructures/List.h"
#include "../DataStructures/String.h"
#include "../Metadata/MetadataInfo.h"

namespace Eunoia {

	struct ECSLoadedSystem
	{
		metadata_typeid typeID;
		b32 enabled;
		List<u8> data;
	};

	struct ECSLoadedComponent
	{
		metadata_typeid typeID;
		b32 enabled;
		List<u8> data;
	};

	struct ECSLoadedEntity
	{
		String name;
		b32 enabled;
		List<ECSLoadedComponent> components;
		u32 numChildren;
	};

	struct ECSLoadedScene
	{
		String name;
		List<ECSLoadedEntity> entities;
		List<ECSLoadedSystem> systems;
	};

	typedef void (*WriteUnsafeRuntimeMemberFunction)(List<u8>* safeRuntimeData, const void* unsafeRuntimeData);
	typedef void (*ReadUnsafeRuntimeMemberFunction)(void* unsafeRuntimeData, const List<u8>& safeRuntimeData);

	struct ECSLoaderUnsafeRuntimeMember
	{
		String typeName;
		ReadUnsafeRuntimeMemberFunction ReadUnsafeMember;
		WriteUnsafeRuntimeMemberFunction WriteUnsafeMember;
	};

	enum ECSLoadError
	{
		ECS_LOAD_ERROR_FILE_NOT_FOUND,
		ECS_LOAD_ERROR_NOT_EUECS_FILE,
		ECS_LOAD_ERROR_UNSUPPORTED_VERSION,
		ECS_LOAD_ERROR_UNFINISHED,
		ECS_LOAD_ERROR_NONE
	};

	class EU_API ECSLoader
	{
	public:
		static void Init();
		static ECSLoadError LoadECSSceneFromFile(ECSLoadedScene* loadedScene, const String& path);
		static ECSLoadError LoadECSSceneFromMemory(ECSLoadedScene* loadedScene, const List<u8>& memory);
		static void AddUnsafeRuntimeMember(const ECSLoaderUnsafeRuntimeMember& member);

		static void WriteLoadedSceneToFile(const ECSLoadedScene& loadedScene, const String& path);
	private:
		static void ReadBuffer(void* dst, const List<u8>& buffer, mem_size* offset, mem_size size);
	};
}
