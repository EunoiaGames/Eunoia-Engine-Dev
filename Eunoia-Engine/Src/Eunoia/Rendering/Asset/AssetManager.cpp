#include "AssetManager.h"
#include "../../Core/Engine.h"
#include "MaterialLoader.h"
#include "ModelLoader.h"
#include "../../Utils/Log.h"

namespace Eunoia {

	struct AssetManager_Data
	{
		List<Material> materials;
		List<LoadedMaterialModifier> modifiers;
		List<Model> models;
		Map<String, SamplerID> samplers;
		Map<String, TextureID> textures;
		Map<ModelID, ModelPathInfo> modelPaths;
	};

	static AssetManager_Data s_Data;

	void Eunoia::AssetManager::InitDefaultSamplers()
	{
		Sampler sampler;
		sampler.addressModes[0] = sampler.addressModes[1] = sampler.addressModes[2] = TEXTURE_ADDRESS_MODE_REPEAT;
		sampler.normalizedCoords = true;
		sampler.minFilter = sampler.magFilter = TEXTURE_FILTER_NEAREST;
		sampler.anisotropyEnabled = true;
		sampler.maxAnisotropy = 16.0f;
		sampler.borderColor = TEXTURE_BORDER_COLOR_BLACK_INT;

		SamplerID nraf = CreateSampler(sampler, "Nearest_Repeat_AF");
		sampler.addressModes[0] = sampler.addressModes[1] = sampler.addressModes[2] = TEXTURE_ADDRESS_MODE_CLAMP_TO_EDGE;
		SamplerID nceaf = CreateSampler(sampler, "Nearest_ClampToEdge_AF");
		sampler.minFilter = sampler.magFilter = TEXTURE_FILTER_LINEAR;
		SamplerID lceaf = CreateSampler(sampler, "Linear_ClampToEdge_AF");
		sampler.addressModes[0] = sampler.addressModes[1] = sampler.addressModes[2] = TEXTURE_ADDRESS_MODE_REPEAT;
		SamplerID lraf = CreateSampler(sampler, "Linear_Repeat_AF");
		sampler.minFilter = sampler.magFilter = TEXTURE_FILTER_NEAREST;
		sampler.maxAnisotropy = 0.0f;
		sampler.anisotropyEnabled = false;
		SamplerID nr = CreateSampler(sampler, "Nearest_Repeat");
		sampler.addressModes[0] = sampler.addressModes[1] = sampler.addressModes[2] = TEXTURE_ADDRESS_MODE_CLAMP_TO_EDGE;
		SamplerID nce = CreateSampler(sampler, "Nearest_ClampToEdge");
		sampler.minFilter = sampler.magFilter = TEXTURE_FILTER_LINEAR;
		SamplerID lce = CreateSampler(sampler, "Linear_ClampToEdge");
		sampler.addressModes[0] = sampler.addressModes[1] = sampler.addressModes[2] = TEXTURE_ADDRESS_MODE_REPEAT;
		SamplerID lr = CreateSampler(sampler, "Linear_Repeat");

		if (nraf != EU_SAMPLER_NEAREST_REPEAT_AF || nceaf != EU_SAMPLER_NEAREST_CLAMP_TO_EDGE_AF || lceaf != EU_SAMPLER_LINEAR_CLAMP_TO_EDGE_AF ||
			lraf != EU_SAMPLER_LINEAR_REPEAT_AF || nr != EU_SAMPLER_NEAREST_REPEAT || nce != EU_SAMPLER_NEAREST_CLAMP_TO_EDGE ||
			lce != EU_SAMPLER_LINEAR_CLAMP_TO_EDGE || lr != EU_SAMPLER_LINEAR_REPEAT)
		{
			EU_LOG_WARN("Preset SamplerID mismatch");
		}

		sampler.anisotropyEnabled = false;
		sampler.maxAnisotropy = 0.0f;
		
	}

	void AssetManager::Init()
	{
		RenderContext* rc = Engine::GetRenderContext();

		TextureID white = CreateTexture("Res/Textures/Defaults/White.eutex");
		TextureID black = CreateTexture("Res/Textures/Defaults/Black.eutex");

		InitDefaultSamplers();

		Material defaultMaterial;
		defaultMaterial.name = "EU_Default";
		defaultMaterial.sampler = EU_SAMPLER_NEAREST_REPEAT_AF;
		defaultMaterial.textures[MATERIAL_TEXTURE_ALBEDO] = white;
		defaultMaterial.textures[MATERIAL_TEXTURE_NORMAL] = CreateTexture("Res/Textures/Defaults/Normal.eutex");
		defaultMaterial.textures[MATERIAL_TEXTURE_AO] = white;
		defaultMaterial.textures[MATERIAL_TEXTURE_DISPLACEMENT] = black;
		defaultMaterial.textures[MATERIAL_TEXTURE_GLOSS] = black;
		defaultMaterial.textures[MATERIAL_TEXTURE_SPECULAR] = black;
		defaultMaterial.textures[MATERIAL_TEXTURE_METALLIC] = black;
		defaultMaterial.textures[MATERIAL_TEXTURE_ROUGHNESS] = black;

		LoadedMaterialModifier defaultModifier;
		defaultModifier.name = "EU_Default";
		defaultModifier.modifier.albedo = v3(1.0f, 1.0f, 1.0f);
		defaultModifier.modifier.ao = 1.0f;
		defaultModifier.modifier.dispOffset = 0.0f;
		defaultModifier.modifier.dispScale = 0.0f;
		defaultModifier.modifier.gloss = 0.0f;
		defaultModifier.modifier.specular = 0.0f;
		defaultModifier.modifier.metallic = 0.0f;
		defaultModifier.modifier.roughness = 0.0f;
		defaultModifier.modifier.texCoordScale = 1.0f;

		ModelID cubeModel = CreateModel("Res/Models/Cube.eumdl");
		ModelID sphereModel = CreateModel("Res/Models/Sphere.eumdl");
		ModelID planeModel = CreateModel("Res/Models/Plane.eumdl");

		if (cubeModel != EU_MODEL_CUBE_ID || sphereModel != EU_MODEL_SPHERE_ID || planeModel != EU_MODEL_PLANE_ID)
		{
			EU_LOG_WARN("Preset ModelID mismatch");
		}

		s_Data.materials.Push(defaultMaterial);
		s_Data.modifiers.Push(defaultModifier);
	}

	MaterialID AssetManager::CreateMaterial(const LoadedMaterial& loadedMaterial, SamplerID sampler)
	{
		Material material;
		material.name = loadedMaterial.name;
		material.sampler = sampler;
		material.path = loadedMaterial.path;

		for (u32 i = 0; i < NUM_MATERIAL_TEXTURE_TYPES; i++)
		{
			MaterialTextureType type = (MaterialTextureType)i;

			if (loadedMaterial.texturePaths[type].Empty())
				material.textures[type] = s_Data.materials[0].textures[i];
			else
				material.textures[type] = CreateTexture(loadedMaterial.texturePaths[type]);
		}

		s_Data.materials.Push(material);
		return s_Data.materials.Size();
	}

	MaterialID AssetManager::CreateMaterial(const Material& material)
	{
		s_Data.materials.Push(material);
		return s_Data.materials.Size();
	}

	MaterialModifierID AssetManager::CreateMaterialModifier(const LoadedMaterialModifier& loadedMaterialModifier)
	{
		s_Data.modifiers.Push(loadedMaterialModifier);
		return s_Data.modifiers.Size();
	}

	void AssetManager::CreateMaterials(const LoadedMaterialFile& loadedMaterialFile, SamplerID sampler, MaterialID* firstMatID, MaterialModifierID* firstModID)
	{
		if(!loadedMaterialFile.materials.Empty() && firstMatID) *firstMatID = CreateMaterial(loadedMaterialFile.materials[0], sampler);
		if(!loadedMaterialFile.modifiers.Empty() && firstModID) *firstModID = CreateMaterialModifier(loadedMaterialFile.modifiers[0]);

		for (u32 i = 1; i < loadedMaterialFile.materials.Size(); i++) CreateMaterial(loadedMaterialFile.materials[i], sampler);
		for (u32 i = 1; i < loadedMaterialFile.modifiers.Size(); i++) CreateMaterialModifier(loadedMaterialFile.modifiers[i]);
	}

	void AssetManager::CreateMaterials(const String& eumtlFile, SamplerID sampler, MaterialID* firstMatID, MaterialModifierID* firstModID)
	{
		LoadedMaterialFile loadedMaterialFile;
		EumtlLoadError error = MaterialLoader::LoadEumtlMaterial(eumtlFile, &loadedMaterialFile);
		if (error == EUMTL_LOAD_SUCCESS)
			CreateMaterials(loadedMaterialFile, sampler, firstMatID, firstModID);
	}

	ModelID AssetManager::CreateModel(const LoadedModel& loadedModel)
	{
		RenderContext* rc = Engine::GetRenderContext();

		Model model;
		model.vertexBuffer = rc->CreateBuffer(BUFFER_TYPE_VERTEX, BUFFER_USAGE_STATIC, &loadedModel.vertices[0], sizeof(ModelVertex) * loadedModel.vertices.Size());
		model.indexBuffer = rc->CreateBuffer(BUFFER_TYPE_INDEX, BUFFER_USAGE_STATIC, &loadedModel.indices[0], sizeof(u32) * loadedModel.indices.Size());
		model.meshes = loadedModel.meshes;
		model.bones = loadedModel.bones;
		model.animations = loadedModel.animations;
		model.globalInverseTransform = loadedModel.globalInverseTransform;
		model.rootBone = loadedModel.rootBone;
		model.totalIndexCount = loadedModel.indices.Size();
		if (!loadedModel.materials.Empty())
		{
			model.materials.SetCapacityAndElementCount(loadedModel.materials.Size());
		}
		else
		{
			model.materials.SetCapacityAndElementCount(1);
			model.materials[0] = EU_DEFAULT_MATERIAL_ID;
		}
		if (!loadedModel.materialModifiers.Empty())
		{
			model.modifiers.SetCapacityAndElementCount(loadedModel.materialModifiers.Size());
		}
		else
		{
			model.modifiers.SetCapacityAndElementCount(1);
			model.modifiers[0] = EU_DEFAULT_MATERIAL_MODIFIER_ID;
		}

		for (u32 i = 0; i < loadedModel.materials.Size(); i++)
			model.materials[i] = GetMaterialID(loadedModel.materials[i]);

		for (u32 i = 0; i < loadedModel.materialModifiers.Size(); i++)
			model.modifiers[i] = GetMaterialModifierID(loadedModel.materialModifiers[i]);

		s_Data.models.Push(model);

		s_Data.modelPaths[s_Data.models.Size()].path = loadedModel.path;

		return s_Data.models.Size();
	}

	ModelID AssetManager::CreateModel(const String& eumdlFile)
	{
		const List<MapKeyPair<ModelID, ModelPathInfo>> createdModelPaths = s_Data.modelPaths.GetKeyPairList();
		for (u32 i = 0; i < createdModelPaths.Size(); i++)
			if (createdModelPaths[i].elem.path == eumdlFile)
				return createdModelPaths[i].key;

		LoadedModel loadedModel;
		EumdlLoadError error = ModelLoader::LoadEumdlModel(eumdlFile, &loadedModel);
		if (error == EUMDL_LOAD_SUCCESS)
			return CreateModel(loadedModel);
	}

	TextureID AssetManager::CreateTexture(const String& eutexFile)
	{
		TextureID id = EU_INVALID_TEXTURE_ID;
		if (!s_Data.textures.FindElement(eutexFile, &id))
		{
			id = Engine::GetRenderContext()->CreateTexture2D(eutexFile);
			s_Data.textures[eutexFile] = id;
		}

		return id;
	}

	SamplerID AssetManager::CreateSampler(const Sampler& sampler, const String& name)
	{
		SamplerID id = EU_INVALID_SAMPLER_ID;
		if (!s_Data.samplers.FindElement(name, &id))
		{
			id = Engine::GetRenderContext()->CreateSampler(sampler);
			s_Data.samplers[name] = id;
		}

		return id;
	}

	SamplerID AssetManager::CreateSampler(const Sampler& sampler)
	{
		const String& name = "EU_SAMPLER_" + s_Data.samplers.Size();
		return CreateSampler(sampler, name);
	}

	String AssetManager::GetTexturePath(TextureID tid)
	{
		const List<MapKeyPair<String, TextureID>>& keyPairs = s_Data.textures.GetKeyPairList();
		for (u32 i = 0; i < keyPairs.Size(); i++)
			if (keyPairs[i].elem == tid)
				return keyPairs[i].key;
		return "No found";
	}

	String AssetManager::GetModelPath(ModelID mid)
	{
		return s_Data.modelPaths[mid].path;
	}

	SamplerID AssetManager::GetSampler(const String& name)
	{
		return s_Data.samplers[name];
	}

	const Material& AssetManager::GetMaterial(MaterialID mid)
	{
		return s_Data.materials[mid - 1];
	}

	Material& AssetManager::GetMaterial_(MaterialID mid)
	{
		return s_Data.materials[mid - 1];
	}

	MaterialID AssetManager::GetMaterialID(const String& name)
	{
		for (u32 i = 0; i < s_Data.materials.Size(); i++)
			if (s_Data.materials[i].name == name)
				return MaterialID(i + 1);

		return EU_INVALID_MATERIAL_ID;
	}

	String AssetManager::GetSamplerName(SamplerID sampler)
	{
		const List<MapKeyPair<String, SamplerID>>& keyPairs = s_Data.samplers.GetKeyPairList();
		for (u32 i = 0; i < keyPairs.Size(); i++)
		{
			if (keyPairs[i].elem == sampler)
				return keyPairs[i].key;
		}

		return "Invalid Sampler ID";
	}

	const MaterialModifier& AssetManager::GetMaterialModifier(MaterialModifierID mmid)
	{
		return s_Data.modifiers[mmid - 1].modifier;
	}

	MaterialModifier& AssetManager::GetMaterialModifier_(MaterialModifierID mmid)
	{
		return s_Data.modifiers[mmid - 1].modifier;
	}

	String AssetManager::GetMaterialModifierName(MaterialModifierID mmid)
	{
		return s_Data.modifiers[mmid - 1].name;
	}

	String AssetManager::GetMaterialModifierPath(MaterialModifierID mmid)
	{
		return s_Data.modifiers[mmid - 1].path;
	}

	MaterialModifierID AssetManager::GetMaterialModifierID(const String& name)
	{
		for (u32 i = 0; i < s_Data.modifiers.Size(); i++)
			if (s_Data.modifiers[i].name == name)
				return MaterialModifierID(i + 1);

		return EU_INVALID_MATERIAL_MODIFIER_ID;
	}

	const Model& AssetManager::GetModel(ModelID mid)
	{
		return s_Data.models[mid - 1];
	}

	const const Map<String, TextureID>& AssetManager::GetTextureMap()
	{
		return s_Data.textures;
	}

	const Map<String, SamplerID>& AssetManager::GetSamplerMap()
	{
		return s_Data.samplers;
	}

	const Map<ModelID, ModelPathInfo>& AssetManager::GetModelPathMap()
	{
		return s_Data.modelPaths;
	}

	const List<LoadedMaterialModifier>& AssetManager::GetMaterialModifierList()
	{
		return s_Data.modifiers;
	}

	const List<Material>& AssetManager::GetMaterialList()
	{
		return s_Data.materials;
	}

	const List<Model>& AssetManager::GetModelList()
	{
		return s_Data.models;
	}

	void AssetManager::DebugPrintBoneHierarchy(ModelID mid)
	{
		const Model& model = s_Data.models[mid - 1];
		DebugPrintBoneHierarchyHelper(model, model.rootBone);
	}

	void AssetManager::DebugPrintBoneHierarchyHelper(const Model& model, u32 boneIndex)
	{
		//std::cout << model.bones[boneIndex].name << std::endl << "\t";
		//for (u32 i = 0; i < model.bones[boneIndex].children.Size(); i++)
		//{
			//DebugPrintBoneHierarchyHelper(model, model.bones[boneIndex].children[i]);
		//}
	}

	void AssetManager::DebugPrintBoneList(ModelID mid)
	{
		const Model& model = s_Data.models[mid - 1];
		for (u32 i = 0; i < model.bones.Size(); i++)
		{
			std::cout << model.bones[i].name << std::endl << "\t";
			for (u32 j = 0; j < model.bones[i].children.Size(); j++)
			{
				u32 childIndex = model.bones[i].children[j];
				std::cout << model.bones[childIndex].name << ", ";
			}
			std::cout << std::endl << std::endl;
		}
	}

}
