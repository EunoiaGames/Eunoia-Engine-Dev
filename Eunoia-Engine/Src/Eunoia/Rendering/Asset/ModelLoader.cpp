#include "ModelLoader.h"

namespace Eunoia {

	struct EumdlMetadata
	{
		u32 numVertices;
		u32 numIndices;
		u32 numMaterial;
		u32 numMeshes;
		u32 numBones;
		u32 numAnimations;
	};

	EumdlLoadError ModelLoader::LoadEumdlModel(const String& path, LoadedModel* loadedModel)
	{
		String p = path;
		loadedModel->path = p;
		FILE* file = fopen(p.C_Str(), "rb");
		if (!file)
			return EUMDL_LOAD_ERROR_NOT_FOUND;

		//Load Header
		char header[8];
		fread(header, 1, 8, file);
		if (!(header[0] == 'e' && header[1] == 'u' && header[2] == 'm' && header[3] == 'd' && header[4] == 'l'))
			return EUMDL_LOAD_ERROR_NOT_EUMDL_FORMAT;

		if (!(header[5] == 1 && header[6] == 1 && header[7] == 0))
			return EUMDL_LOAD_ERROR_UNSUPPORTED_EUMDL_VERSION;
		//Load Metadata
		EumdlMetadata metadata;
		fread(&metadata, sizeof(EumdlMetadata), 1, file);

		if (metadata.numVertices == 0 || metadata.numIndices == 0)
			return EUMDL_LOAD_ERROR_INVALID_VERTEX_OR_INDEX_COUNT;

		loadedModel->vertices.SetCapacityAndElementCount(metadata.numVertices);
		loadedModel->indices.SetCapacityAndElementCount(metadata.numIndices);
		loadedModel->meshes.SetCapacityAndElementCount(metadata.numMeshes);
		if (metadata.numMaterial > 0)
			loadedModel->materials.SetCapacityAndElementCount(metadata.numMaterial);
		if (metadata.numBones > 0)
			loadedModel->bones.SetCapacityAndElementCount(metadata.numBones);

		//Load Vertices and Indices
		fread(&loadedModel->vertices[0], sizeof(ModelVertex), metadata.numVertices, file);
		fread(&loadedModel->indices[0], sizeof(u32), metadata.numIndices, file);

		char texturePath[512];
		char materialName[64];
		//Load Materials
		for (u32 i = 0; i < metadata.numMaterial; i++)
		{
			u32 materialNameLength;
			fread(&materialNameLength, sizeof(u32), 1, file);
			fread(materialName, 1, materialNameLength, file);
			materialName[materialNameLength] = 0;
			String mName = materialName;
			mName = "Res/Textures/" + mName;
			loadedModel->materials.Push(mName);
		}
		
		//Load Meshes
		fread(&loadedModel->meshes[0], sizeof(LoadedMesh), metadata.numMeshes, file);

		//Load Bones
		fread(&loadedModel->globalInverseTransform, sizeof(m4), 1, file);
		fread(&loadedModel->rootBone, sizeof(u32), 1, file);

		for (u32 i = 0; i < metadata.numBones; i++)
		{
			ModelBone* bone = &loadedModel->bones[i];

			u32 boneNameLength;
			fread(&boneNameLength, sizeof(u32), 1, file);
			bone->name = String(boneNameLength);
			fread(bone->name.GetChars(), 1, boneNameLength, file);
			fread(&bone->transform, sizeof(m4), 1, file);
			fread(&bone->boneSpaceTransform, sizeof(m4), 1, file);
			u32 numChildren;
			fread(&numChildren, sizeof(u32), 1, file);
			bone->children.SetCapacityAndElementCount(numChildren);
			fread(&bone->children[0], sizeof(u32), numChildren, file);
		}

		//Load Animations
		for (u32 i = 0; i < metadata.numAnimations; i++)
		{
			ModelAnimation animation;
			u32 animationNameLength;
			fread(&animationNameLength, sizeof(u32), 1, file);
			animation.name = String(animationNameLength);
			fread(animation.name.GetChars(), 1, animationNameLength, file);
			fread(&animation.durration, sizeof(r32), 1, file);
			fread(&animation.tps, sizeof(r32), 1, file);
			u32 numChannels;
			fread(&numChannels, sizeof(u32), 1, file);
			animation.channels.SetCapacityAndElementCount(numChannels);
			for (u32 j = 0; j < numChannels; j++)
			{
				ModelAnimationChannel* channel = &animation.channels[j];
				fread(&channel->boneIndex, sizeof(u32), 1, file);
				u32 numKeyFrames[3];
				fread(numKeyFrames, sizeof(u32), 3, file);
				if (numKeyFrames[0] > 0)
				{
					channel->positionKeyFrames.SetCapacityAndElementCount(numKeyFrames[0]);
					fread(&channel->positionKeyFrames[0], sizeof(ModelAnimationVecKeyFrame), numKeyFrames[0], file);
				}
				if (numKeyFrames[1] > 0)
				{
					channel->scaleKeyFrames.SetCapacityAndElementCount(numKeyFrames[1]);
					fread(&channel->scaleKeyFrames[0], sizeof(ModelAnimationVecKeyFrame), numKeyFrames[1], file);
				}
				if (numKeyFrames[2] > 0)
				{
					channel->rotationKeyFrames.SetCapacityAndElementCount(numKeyFrames[2]);
					fread(&channel->rotationKeyFrames[0], sizeof(ModelAnimationQuatKeyFrame), numKeyFrames[2], file);
				}
			}
			loadedModel->animations.Push(animation);
		}

		//Load Footer
		char footer[5];
		fread(footer, 1, 5, file);
		if (!(footer[0] == 'e' && footer[1] == 'u' && footer[2] == 'm' && footer[3] == 'd' && footer[4] == 'l'))
			return EUMDL_LOAD_ERROR_EXTRA_DATA_NOT_PARSED;

		return EUMDL_LOAD_SUCCESS;
	}
}
