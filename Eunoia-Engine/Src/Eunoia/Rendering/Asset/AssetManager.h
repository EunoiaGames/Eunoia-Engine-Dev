#pragma once

#include "AssetTypeIDs.h"
#include "Material.h"
#include "Model.h"

namespace Eunoia {

	struct ModelPathInfo
	{
		String path;
		String relPath;
	};

	class EU_API AssetManager
	{
	public:
		static void Init();

		static MaterialID CreateMaterial(const LoadedMaterial& loadedMaterial, SamplerID sampler);
		static MaterialID CreateMaterial(const Material& material);
		static MaterialModifierID CreateMaterialModifier(const LoadedMaterialModifier& loadedMaterialModifier);
		static void CreateMaterials(const LoadedMaterialFile& loadedMaterialFile, SamplerID sampler, MaterialID* firstMatID = 0, MaterialModifierID* firstModID = 0);
		static void CreateMaterials(const String& eumtlFile, SamplerID sampler, MaterialID* firstMatID = 0, MaterialModifierID* firstModID = 0);
		static ModelID CreateModel(const LoadedModel& loadedModel);
		static ModelID CreateModel(const String& eumdlFile);

		static TextureID CreateTexture(const String& eutexFile);

		static SamplerID CreateSampler(const Sampler& sampler, const String& name);
		static SamplerID CreateSampler(const Sampler& sampler);

		static String GetTexturePath(TextureID tid);
		static String GetModelPath(ModelID mid);

		static SamplerID GetSampler(const String& name);

		static const Material& GetMaterial(MaterialID mid);
		static Material& GetMaterial_(MaterialID mid);
		static MaterialID GetMaterialID(const String& name);

		static String GetSamplerName(SamplerID sampler);

		static const MaterialModifier& GetMaterialModifier(MaterialModifierID mmid);
		static MaterialModifier& GetMaterialModifier_(MaterialModifierID mmid);
		static String GetMaterialModifierName(MaterialModifierID mmid);
		static String GetMaterialModifierPath(MaterialModifierID mmid);
		static MaterialModifierID GetMaterialModifierID(const String& name);

		static const Model& GetModel(ModelID mid);

		static const Map<String, TextureID>& GetTextureMap();
		static const Map<String, SamplerID>& GetSamplerMap();
		static const Map<ModelID, ModelPathInfo>& GetModelPathMap();
		static const List<LoadedMaterialModifier>& GetMaterialModifierList();
		static const List<Material>& GetMaterialList();
		static const List<Model>& GetModelList();

		static void DebugPrintBoneHierarchy(ModelID model);
		static void DebugPrintBoneList(ModelID model);
	private:
		static void InitDefaultSamplers();
		static void DebugPrintBoneHierarchyHelper(const Model& model, u32 boneIndex);
	};

}