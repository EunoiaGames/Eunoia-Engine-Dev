#pragma once

#include "MetadataInfo.h"
#include "../DataStructures/List.h"
#include "../Memory/Allocators.h"

#define EU_METADATA_TYPEID_CHAR Eunoia::Metadata::GetTypeID<char>();

namespace Eunoia {

	class EU_API Metadata
	{
	public:
		template<typename T>
		static metadata_typeid GetTypeID();

		static void CallDefaultConstructor(metadata_typeid id, void* dst);

		static metadata_typeid GetClassTypeID(const String& name);

		static const MetadataInfo& GetMetadata(metadata_typeid id);
		static void RegisterMetadataInfo(const MetadataInfo& metadataInfo);
		//static void RegisterMetadataClassDefaultConstructor(metadata_typeid id);
		static void UnregisterProjectMetadata();

		static const List<MetadataInfo>& GetComponentMetadataList();
		static const List<MetadataInfo>& GetSystemMetadataList();
		static const List<MetadataInfo>& GetEventMetadataList();

		static void Init();

		static MetadataClass* AllocateClass(b32 isEngine);
		static MetadataEnum* AllocateEnum(b32 isEngine);
		static MetadataPrimitive* AllocatePrimitive();

		template<typename T>
		static MetadataInfo ConstructMetadataInfo();

		static const metadata_typeid LastEngineTypeID;
		static metadata_typeid LastProjectTypeID;
	private:
		static void InitMetadataPrimitives();
		static void InitMetadataInfos();
	private:
		static List<MetadataInfo> s_RegisteredMetadataInfos;
		static List<MetadataInfo> s_ComponentMetadataClasses;
		static List<MetadataInfo> s_SystemMetadataClasses;
		static List<MetadataInfo> s_EventMetadataClasses;

		static LinearAllocator s_EngineAllocator;
		static LinearAllocator s_ProjectAllocator;
	};

}