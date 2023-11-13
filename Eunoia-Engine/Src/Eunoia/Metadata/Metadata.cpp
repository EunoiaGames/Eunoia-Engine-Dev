#include "Metadata.h"

namespace Eunoia {

	List<MetadataInfo> Metadata::s_RegisteredMetadataInfos(512, 512);
	List<MetadataInfo> Metadata::s_ComponentMetadataClasses;
	List<MetadataInfo> Metadata::s_SystemMetadataClasses;
	List<MetadataInfo> Metadata::s_EventMetadataClasses;

	LinearAllocator Metadata::s_EngineAllocator(sizeof(MetadataClass) * 256);
	LinearAllocator Metadata::s_ProjectAllocator(sizeof(MetadataClass) * 256);

	metadata_typeid Metadata::LastProjectTypeID = Metadata::LastEngineTypeID;

	void Metadata::CallDefaultConstructor(metadata_typeid id, void* dst)
	{
		return s_RegisteredMetadataInfos[id].cls->DefaultConstructor(dst);
	}

	metadata_typeid Metadata::GetClassTypeID(const String& name)
	{
		for (u32 i = 0; i < s_RegisteredMetadataInfos.Size(); i++)
			if (s_RegisteredMetadataInfos[i].type == METADATA_CLASS &&
				s_RegisteredMetadataInfos[i].cls->name == name)
				return s_RegisteredMetadataInfos[i].id;
		return 1024;
	}

	const MetadataInfo& Metadata::GetMetadata(metadata_typeid id)
	{
		return s_RegisteredMetadataInfos[id];
	}

	void Metadata::RegisterMetadataInfo(const MetadataInfo& info)
	{
		s_RegisteredMetadataInfos[info.id] = info;

		if (info.type == METADATA_CLASS)
		{
			if (info.cls->isComponent)
				s_ComponentMetadataClasses.Push(info);
			if (info.cls->isSystem)
				s_SystemMetadataClasses.Push(info);
			if (info.cls->isEvent)
				s_EventMetadataClasses.Push(info);
		}
	}

	void Metadata::UnregisterProjectMetadata()
	{
		if (Metadata::LastProjectTypeID == Metadata::LastEngineTypeID)
			return;

		s_ProjectAllocator.Reset();

		Metadata::LastProjectTypeID = Metadata::LastEngineTypeID;

		s_ComponentMetadataClasses.Clear();
		s_SystemMetadataClasses.Clear();
		s_EventMetadataClasses.Clear();

		for (u32 i = 0; i <= Metadata::LastEngineTypeID; i++)
		{
			const MetadataInfo& info = s_RegisteredMetadataInfos[i];
			if (info.type == METADATA_CLASS)
			{
				if (info.cls->isComponent)
					s_ComponentMetadataClasses.Push(info);
				if (info.cls->isSystem)
					s_SystemMetadataClasses.Push(info);
				if (info.cls->isEvent)
					s_EventMetadataClasses.Push(info);
			}
		}
	}

	const List<MetadataInfo>& Metadata::GetComponentMetadataList()
	{
		return s_ComponentMetadataClasses;
	}

	const List<MetadataInfo>& Metadata::GetSystemMetadataList()
	{
		return s_SystemMetadataClasses;
	}

	const List<MetadataInfo>& Metadata::GetEventMetadataList()
	{
		return s_EventMetadataClasses;
	}

	void Metadata::Init()
	{
		InitMetadataPrimitives();
		InitMetadataInfos();
	}

	MetadataClass* Metadata::AllocateClass(b32 isEngine)
	{
		MetadataClass* cls = 0;

		if (isEngine) 
			cls = (MetadataClass*)s_EngineAllocator.Allocate(sizeof(MetadataClass));
		else
			cls = (MetadataClass*)s_ProjectAllocator.Allocate(sizeof(MetadataClass));

		new(cls) MetadataClass();

		return cls;
	}

	MetadataEnum* Metadata::AllocateEnum(b32 isEngine)
	{
		MetadataEnum* enm = 0;

		if (isEngine)
			enm = (MetadataEnum*)s_EngineAllocator.Allocate(sizeof(MetadataEnum));
		else
			enm = (MetadataEnum*)s_ProjectAllocator.Allocate(sizeof(MetadataEnum));

		new(enm) MetadataEnum();

		return enm;
	}

	MetadataPrimitive* Metadata::AllocatePrimitive()
	{
		MetadataPrimitive* prim = (MetadataPrimitive*)s_EngineAllocator.Allocate(sizeof(MetadataPrimitive));
		new(prim) MetadataPrimitive();
		return prim;
	}

}