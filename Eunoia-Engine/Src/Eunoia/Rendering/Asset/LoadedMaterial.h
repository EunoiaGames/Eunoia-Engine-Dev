#pragma once

#include "../../Math/Math.h"
#include "../../DataStructures/Map.h"
#include "../../DataStructures/String.h"

namespace Eunoia {

	enum MaterialType
	{
		MATERIAL_TYPE_PHONG = 1,
		MATERIAL_TYPE_PBR = 2,
		NUM_MATERIAL_TYPES,
	};

	enum MaterialTextureType
	{
		//PBR or PHONG
		MATERIAL_TEXTURE_ALBEDO,
		MATERIAL_TEXTURE_NORMAL,
		MATERIAL_TEXTURE_DISPLACEMENT,
		MATERIAL_TEXTURE_AO,
		//PHONG
		MATERIAL_TEXTURE_SPECULAR,
		MATERIAL_TEXTURE_GLOSS,
		//PBR
		MATERIAL_TEXTURE_METALLIC,
		MATERIAL_TEXTURE_ROUGHNESS,
		//Meta
		NUM_MATERIAL_TEXTURE_TYPES,
		NUM_PHONG_MATERIAL_TEXTURE_TYPES = 6,
		NUM_PBR_MATERIAL_TEXTURE_TYPES = 6
	};

	struct LoadedMaterial
	{
		String name;
		String path;
		Map<MaterialTextureType, String> texturePaths;
	};

	struct MaterialModifier
	{
		v3 albedo;
		r32 texCoordScale;
		r32 ao;
		r32 dispScale;
		r32 dispOffset;
		r32 specular;
		r32 gloss;
		r32 metallic;
		r32 roughness;
	};

	struct LoadedMaterialModifier
	{
		String name;
		String path;
		MaterialModifier modifier;
	};

	struct LoadedMaterialFile
	{
		List<LoadedMaterial> materials;
		List<LoadedMaterialModifier> modifiers;
	};
}
