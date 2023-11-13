#pragma once

#include "../../Math/Math.h"
#include "../../DataStructures/List.h"
#include "../../DataStructures/String.h"

namespace Eunoia {

	struct ModelVertex
	{
		ModelVertex(const v3& pos, const v2& texCoord = v2(0.0f, 0.0f), const v3& color = v3(0.0f, 0.0f, 0.0f), const v3& normal = v3(0.0f, 0.0f, 0.0f), const v3& tangent = v3(0.0f, 0.0f, 0.0f)) :
			pos(pos),
			texCoord(texCoord),
			color(color),
			normal(normal),
			tangent(tangent)
		{
			for (u32 i = 0; i < 4; i++)
			{
				boneIDs[i] = 0;
				boneWeights[i] = 0.0f;
			}
		}

		ModelVertex()
		{
			for (u32 i = 0; i < 4; i++)
			{
				boneIDs[i] = 0;
				boneWeights[i] = 0.0f;
			}
		}

		v3 pos;
		v3 color;
		v3 normal;
		v3 tangent;
		v2 texCoord;
		u32 boneIDs[4];
		r32 boneWeights[4];
	};

	struct LoadedMesh
	{
		u32 indexOffset;
		u32 vertexOffset;
		u32 indexCount;
		u32 materialIndex;
		u32 materialModifierIndex;
	};

	struct ModelBone
	{
		String name;
		m4 transform;
		m4 boneSpaceTransform;
		List<u32> children;
	};

	struct ModelAnimationVecKeyFrame
	{
		r32 timeStamp;
		v3 value;
	};

	struct ModelAnimationQuatKeyFrame
	{
		r32 timeStamp;
		quat value;
	};

	struct ModelAnimationChannel
	{
		u32 boneIndex;
		List<ModelAnimationVecKeyFrame> positionKeyFrames;
		List<ModelAnimationVecKeyFrame> scaleKeyFrames;
		List<ModelAnimationQuatKeyFrame> rotationKeyFrames;
	};

	struct ModelAnimation
	{
		String name;
		r32 durration;
		r32 tps;
		List<ModelAnimationChannel> channels;
	};

	struct LoadedModel
	{
		String path;
		List<ModelVertex> vertices;
		List<u32> indices;
		List<LoadedMesh> meshes;
		List<String> materials;
		List<String> materialModifiers;
		List<ModelBone> bones;
		List<ModelAnimation> animations;
		m4 globalInverseTransform;
		u32 rootBone;
	};

}