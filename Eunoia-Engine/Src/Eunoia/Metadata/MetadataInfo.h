#pragma once

#include "../DataStructures/String.h"
#include "../DataStructures/List.h"
#include "../Math/Math.h"

namespace Eunoia {

	template<typename T> static void MetadataCreateInstance(void* dst) { new(dst) T(); }

	typedef u32 metadata_typeid;

	enum MetadataAccessModifier
	{
		METADATA_ACCESS_MODIFIER_PUBLIC,
		METADATA_ACCESS_MODIFIER_PRIVATE,
		METADATA_ACCESS_MODIFIER_PROTECTED,
	};

	struct MetadataEnumValue
	{
		String name;
		u32 value;
	};

	struct MetadataEnum
	{
		String name;
		List<MetadataEnumValue> values;
	};

	struct MetadataMember
	{
		MetadataMember()
		{}

		String name;
		String typeName;
		metadata_typeid typeID;
		MetadataAccessModifier accessModifier;
		mem_size offset;
		mem_size size;
		b32 isStatic;
		b32 isConst;
		b32 isPointer;
		u32 arrayLength;
		v4 uiSliderMin;
		v4 uiSliderMax;
		r32 uiSliderSpeed;
		b32 is32BitBool;
	};

	struct MetadataClass
	{
		MetadataClass()
		{}

		String name;
		String baseClassName;
		mem_size baseClassSize;
		List<MetadataMember> members;
		mem_size size;
		b32 isComponent;
		b32 isSystem;
		b32 isEvent;
		void (*DefaultConstructor)(void*);
	};

	enum MetadataPrimitveType
	{
		METADATA_PRIMITIVE_CHAR,
		METADATA_PRIMITIVE_U8,
		METADATA_PRIMITIVE_U16,
		METADATA_PRIMITIVE_U32,
		METADATA_PRIMITIVE_S8,
		METADATA_PRIMITIVE_S16,
		METADATA_PRIMITIVE_S32,
		METADATA_PRIMITIVE_BOOL,
		METADATA_PRIMITIVE_R32,
		METADATA_PRIMITIVE_R64
	};

	struct MetadataPrimitive
	{
		String name;
		MetadataPrimitveType type;
		u32 size;
	};

	enum MetadataType
	{
		METADATA_CLASS,
		METADATA_ENUM,
		METADATA_PRIMITIVE,

		NUM_METADATA_TYPES
	};

	struct MetadataInfo
	{
		metadata_typeid id;
		MetadataType type;

		MetadataClass* cls;
		MetadataEnum* enm;
		MetadataPrimitive* primitive;
	};

}