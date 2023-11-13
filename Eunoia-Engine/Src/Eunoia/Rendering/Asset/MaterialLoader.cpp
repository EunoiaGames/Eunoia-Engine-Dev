#include "MaterialLoader.h"

namespace Eunoia {

	struct EumtlMetadata
	{
		u32 numMaterials;
		u32 numMaterialModifiers;
	};

	EumtlLoadError MaterialLoader::LoadEumtlMaterial(const String& path, LoadedMaterialFile* loadedMaterialFile)
	{
		String p = path;
		FILE* file = fopen(p.C_Str(), "r");
		if (!file)
			return EUMTL_LOAD_ERROR_NOT_FOUND;

		char header[5];
		fread(header, 1, 5, file);
		if (header[0] != 'e' && header[1] != 'u' && header[2] != 'm' && header[3] != 't' && header[4] != 'l')
			return EUMTL_LOAD_ERROR_NOT_EUMTL_FORMAT;

		u8 version[3];
		fread(version, sizeof(u8), 3, file);
		if (version[0] != 1 && version[1] != 0 && version[2] != 0)
			return EUMTL_LOAD_ERROR_UNSUPPORTED_VERSION;

		EumtlMetadata metadata;
		fread(&metadata, sizeof(EumtlMetadata), 1, file);

		char materialName[64];
		char texturePath[512];
		for (u32 i = 0; i < metadata.numMaterials; i++)
		{
			LoadedMaterial material;
			material.path = path;

			u32 materialNameLength;
			fread(&materialNameLength, sizeof(u32), 1, file);
			fread(materialName, 1, materialNameLength, file);
			materialName[materialNameLength] = 0;
			material.name = materialName;

			u32 textureCount;
			fread(&textureCount, sizeof(u32), 1, file);
			for (u32 j = 0; j < textureCount; j++)
			{
				MaterialTextureType type;
				fread(&type, sizeof(MaterialTextureType), 1, file);
				u32 texturePathLength;
				fread(&texturePathLength, sizeof(u32), 1, file);
				fread(texturePath, 1, texturePathLength, file);
				texturePath[texturePathLength] = 0;
				material.texturePaths[type] = texturePath;
			}

			loadedMaterialFile->materials.Push(material);
		}

		
		for (u32 i = 0; i < metadata.numMaterialModifiers; i++)
		{
			LoadedMaterialModifier modifier;
			modifier.path = path;

			u32 modifierNameLength;
			fread(&modifierNameLength, sizeof(u32), 1, file);
			fread(materialName, 1, modifierNameLength, file);
			materialName[modifierNameLength] = 0;
			modifier.name = materialName;

			fread(&modifier.modifier, sizeof(MaterialModifier), 1, file);

			loadedMaterialFile->modifiers.Push(modifier);
		}
	}

	void MaterialLoader::WriteEumtlMaterial(const String& path, const LoadedMaterialFile& data)
	{
		char header[6] = "eumtl";
		u8 version[3] = {1, 0, 0};
		
		FILE* file = fopen(path.C_Str(), "w");

		fwrite(header, 1, 5, file);
		fwrite(version, 1, 3, file);

		EumtlMetadata metadata;
		metadata.numMaterials = data.materials.Size();
		metadata.numMaterialModifiers = data.modifiers.Size();

		fwrite(&metadata, sizeof(EumtlMetadata), 1, file);
		for (u32 i = 0; i < data.materials.Size(); i++)
		{
			const LoadedMaterial& material = data.materials[i];
			u32 length = material.name.Length();
			fwrite(&length, sizeof(u32), 1, file);
			fwrite(material.name.C_Str(), 1, length, file);
			u32 textureCount = NUM_MATERIAL_TEXTURE_TYPES;
			fwrite(&textureCount, sizeof(u32), 1, file);
			for (u32 j = 0; j < textureCount; j++)
			{
				fwrite(&j, sizeof(MaterialTextureType), 1, file);
				length = material.texturePaths[(MaterialTextureType)j].Length();
				fwrite(&length, sizeof(u32), 1, file);
				fwrite(material.texturePaths[(MaterialTextureType)j].C_Str(), 1, length, file);
			}
		}

		for (u32 i = 0; i < data.modifiers.Size(); i++)
		{
			const LoadedMaterialModifier& modifier = data.modifiers[i];
			u32 length = modifier.name.Length();
			fwrite(&length, sizeof(u32), 1, file);
			fwrite(modifier.name.C_Str(), 1, length, file);
			fwrite(&modifier.modifier, sizeof(MaterialModifier), 1, file);
		}

		fclose(file);
	}

}
