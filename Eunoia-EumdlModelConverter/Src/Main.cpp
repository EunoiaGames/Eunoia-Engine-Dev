#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <iostream>
#include <vector>
#include <map>

typedef unsigned int u32;
typedef signed int s32;
typedef u32 b32;
typedef float r32;

static const aiScene* Scene;

struct vertex
{
	aiVector3D Pos;
	aiVector3D Color;
	aiVector3D Normal;
	aiVector3D Tangent;
	aiVector2D TexCoord;
	u32 BoneIDs[4];
	r32 BoneWeights[4];
};

struct mesh
{
	u32 IndexOffset;
	u32 VertexOffset;
	u32 IndexCount;
	u32 MaterialIndex;
	u32 MaterialModifierIndex;
};


enum material_texture_type
{
	Material_Texture_Albedo,
	Material_Texture_Normal,
	Material_Texture_Displacement,
	Material_Texture_AO,

	Material_Texture_Specular,
	Material_Texture_Gloss,

	Material_Texture_Metallic,
	Material_Texture_Roughness,

	Num_Material_Texture_Types
};

struct material
{
	aiString TextureNames[Num_Material_Texture_Types];
	aiString Name;
};

struct animation_vec_keyframe
{
	r32 time;
	aiVector3D vec;
};

struct quaternion
{
	r32 x;
	r32 y;
	r32 z;
	r32 w;
};

struct animation_quat_keyframe
{
	r32 time;
	quaternion quat;
};

struct animation_channel
{
	u32 BoneIndex;
	animation_vec_keyframe* PositionKeyFrames;
	u32 NumPositionKeyFrames;
	animation_vec_keyframe* ScalingKeyFrames;
	u32 NumScalingKeyFrames;
	animation_quat_keyframe* RotationKeyFrames;
	u32 NumRotationKeyFrames;
};

struct animation
{
	aiString Name;
	r32 Durration;
	r32 Tps;
	animation_channel* Channels;
	u32 NumChannels;
};

struct bone
{
	aiString Name;
	aiMatrix4x4 Transform;
	aiMatrix4x4 BoneSpace;
	std::vector<u32> Children;
};

struct model
{
	vertex* Vertices;
	u32 NumVertices;

	u32* Indices;
	u32 NumIndices;

	material* Materials;
	u32 NumMaterials;

	mesh* Meshes;
	u32 NumMeshes;

	animation* Animations;
	u32 NumAnimations;

	aiMatrix4x4 GlobalInverseTransform;
	std::vector<bone> Bones;
	u32 RootBone;
};

static u32 aiStringLength(const aiString& string)
{
	u32 length = 0;
	while (string.data[length] != 0)
		length++;
	return length;
}

static void CreateErrorFile(const char* Message)
{
	FILE* File = fopen("Error.txt", "w");
	fputs(Message, File);
	fclose(File);
	std::exit(1);
}

static void WriteHeader(FILE* File)
{
	char Header[8];
	strcpy(Header, "eumdl");
	Header[5] = 1; Header[6] = 1; Header[7] = 0;
	fwrite(Header, 1, 8, File);
}

static void WriteMetadata(FILE* File, model* Model)
{
	u32 Metadata[6];
	Metadata[0] = Model->NumVertices;
	Metadata[1] = Model->NumIndices;
	Metadata[2] = Model->NumMaterials;
	Metadata[3] = Model->NumMeshes;
	Metadata[4] = Model->Bones.size();
	Metadata[5] = Model->NumAnimations;
	fwrite(Metadata, sizeof(u32), 6, File);
}

static void WriteVerticesAndIndices(FILE* File, model* Model)
{
	fwrite(Model->Vertices, sizeof(vertex), Model->NumVertices, File);
	fwrite(Model->Indices, sizeof(u32), Model->NumIndices, File);
}

static void WriteMaterials(FILE* MdlFile, FILE* MtlFile, model* Model)
{
	if (Model->NumMaterials == 0)
		return;

	char EumtlHeader[8];
	strcpy(EumtlHeader, "eumtl");
	EumtlHeader[5] = 1; EumtlHeader[6] = 0; EumtlHeader[7] = 0;
	fwrite(EumtlHeader, 1, 8, MtlFile);

	u32 EumtlMetadata[2] = { Model->NumMaterials, 0 };
	fwrite(EumtlMetadata, sizeof(u32), 2, MtlFile);

	for (u32 i = 0; i < Model->NumMaterials; i++)
	{
		u32 textureCount = 0;
		for (u32 j = 0; j < Num_Material_Texture_Types; j++)
		{
			u32 length = aiStringLength(Model->Materials[i].TextureNames[j]);
			if (length > 0)
				textureCount++;
		}

		u32 materialNameLength = aiStringLength(Model->Materials[i].Name);

		fwrite(&materialNameLength, sizeof(u32), 1, MdlFile);
		fwrite(Model->Materials[i].Name.C_Str(), 1, materialNameLength, MdlFile);

		fwrite(&materialNameLength, sizeof(u32), 1, MtlFile);
		fwrite(Model->Materials[i].Name.C_Str(), 1, materialNameLength, MtlFile);
		fwrite(&textureCount, sizeof(u32), 1, MtlFile);

		//Write all texture names in the material
		for (u32 j = 0; j < Num_Material_Texture_Types; j++)
		{
			u32 length = aiStringLength(Model->Materials[i].TextureNames[j]);
			if (length > 0)
			{
				fwrite(&j, sizeof(material_texture_type), 1, MtlFile);
				fwrite(&length, sizeof(u32), 1, MtlFile);
				fwrite(Model->Materials[i].TextureNames[j].C_Str(), 1, length, MtlFile);
			}
		}
	}
}

static void WriteMeshes(FILE* File, model* Model)
{
	fwrite(Model->Meshes, sizeof(mesh), Model->NumMeshes, File);
}

static void WriteBones(FILE* File, model* Model)
{
	fwrite(&Model->GlobalInverseTransform, sizeof(aiMatrix4x4), 1, File);
	fwrite(&Model->RootBone, sizeof(u32), 1, File);

	for (u32 i = 0; i < Model->Bones.size(); i++)
	{
		bone* Bone = &Model->Bones[i];
		u32 BoneNameLength = aiStringLength(Bone->Name);
		fwrite(&BoneNameLength, sizeof(u32), 1, File);
		fwrite(Bone->Name.C_Str(), 1, BoneNameLength, File);
		fwrite(&Bone->Transform, sizeof(aiMatrix4x4), 1, File);
		fwrite(&Bone->BoneSpace, sizeof(aiMatrix4x4), 1, File);
		u32 NumChildren = Bone->Children.size();
		fwrite(&NumChildren, sizeof(u32), 1, File);
		if(NumChildren > 0)
			fwrite(&Bone->Children[0], sizeof(u32), NumChildren, File);
	}
}

static void WriteAnimations(FILE* File, model* Model)
{
	for (u32 i = 0; i < Model->NumAnimations; i++)
	{
		const animation* Animation = &Model->Animations[i];
		u32 AnimationNameLength = aiStringLength(Animation->Name);
		fwrite(&AnimationNameLength, sizeof(u32), 1, File);
		fwrite(Animation->Name.C_Str(), 1, AnimationNameLength, File);
		fwrite(&Animation->Durration, sizeof(r32), 1, File);
		fwrite(&Animation->Tps, sizeof(r32), 1, File);
		fwrite(&Animation->NumChannels, sizeof(u32), 1, File);

		for (u32 j = 0; j < Animation->NumChannels; j++)
		{
			const animation_channel* Channel = &Animation->Channels[j];
			fwrite(&Channel->BoneIndex, sizeof(u32), 1, File);
			fwrite(&Channel->NumPositionKeyFrames, sizeof(u32), 1, File);
			fwrite(&Channel->NumScalingKeyFrames, sizeof(u32), 1, File);
			fwrite(&Channel->NumRotationKeyFrames, sizeof(u32), 1, File);
			fwrite(Channel->PositionKeyFrames, sizeof(animation_vec_keyframe), Channel->NumPositionKeyFrames, File);
			fwrite(Channel->ScalingKeyFrames, sizeof(animation_vec_keyframe), Channel->NumScalingKeyFrames, File);
			fwrite(Channel->RotationKeyFrames, sizeof(animation_quat_keyframe), Channel->NumRotationKeyFrames, File);
		}
	}
}

static void WriteFooter(FILE* File)
{
	char Footer[6] = "eumdl";
	fwrite(Footer, 1, 5, File);
}

static void WriteModelToFile(const char* OriginalPath, model* Model)
{
	//		Res/Models/Place/cube.fbx	->	Res/Models/Place/cube.happmdl

	char NewPath[512];
	u32 Index = 0;
	while (OriginalPath[Index] != '.')
	{
		NewPath[Index] = OriginalPath[Index];
		Index++;
	}

	strcpy(&NewPath[Index], ".eumdl");
	NewPath[Index + 6] = 0;
	FILE* MdlFile = fopen(NewPath, "wb");

	FILE* MtlFile = 0;

	if (Model->NumMaterials > 0)
	{
		strcpy(&NewPath[Index], "-Materials.eumtl");
		NewPath[Index + 16] = 0;
		MtlFile = fopen(NewPath, "wb");
	}

	WriteHeader(MdlFile);
	WriteMetadata(MdlFile, Model);
	WriteVerticesAndIndices(MdlFile, Model);
	WriteMaterials(MdlFile, MtlFile, Model);
	if(Model->NumMaterials > 0)
		fclose(MtlFile);
	WriteMeshes(MdlFile, Model);
	WriteBones(MdlFile, Model);
	WriteAnimations(MdlFile, Model);
	WriteFooter(MdlFile);

	fclose(MdlFile);
}

static void LoadMaterials(model* Model)
{
	Model->NumMaterials = Scene->mNumMaterials;
	if (Model->NumMaterials == 0)
		return;

	u32 MaterialCount = 0;
	for (u32 i = 0; i < Scene->mNumMaterials; i++)
	{
		if (Scene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE) != 0)
			MaterialCount++;
	}

	Model->NumMaterials = MaterialCount;

	if (MaterialCount == 0)
		return;

	Model->Materials = (material*)malloc(sizeof(material) * Model->NumMaterials);

	for (u32 i = 0; i < Model->NumMaterials; i++)
	{
		const aiMaterial* Material = Scene->mMaterials[i];

		Material->Get(AI_MATKEY_NAME, Model->Materials[i].Name);

		if (Material->GetTextureCount(aiTextureType_DIFFUSE) == 0)
			continue;
		Material->GetTexture(aiTextureType_DIFFUSE, 0, &Model->Materials[i].TextureNames[Material_Texture_Albedo]);

		if (Material->GetTextureCount(aiTextureType_DISPLACEMENT) > 0)
			Material->GetTexture(aiTextureType_DISPLACEMENT, 0, &Model->Materials[i].TextureNames[Material_Texture_Displacement]);
		else
			Model->Materials[i].TextureNames[Material_Texture_Displacement] = "";

		if (Material->GetTextureCount(aiTextureType_HEIGHT) > 0)
			Material->GetTexture(aiTextureType_HEIGHT, 0, &Model->Materials[i].TextureNames[Material_Texture_Normal]);
		else
			Model->Materials[i].TextureNames[Material_Texture_Normal] = "";

		if (Material->GetTextureCount(aiTextureType_METALNESS) > 0)
			Material->GetTexture(aiTextureType_METALNESS, 0, &Model->Materials[i].TextureNames[Material_Texture_Metallic]);
		else
			Model->Materials[i].TextureNames[Material_Texture_Metallic] = "";

		if (Material->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0)
			Material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &Model->Materials[i].TextureNames[Material_Texture_Roughness]);
		else
			Model->Materials[i].TextureNames[Material_Texture_Roughness] = "";

		if (Material->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION) > 0)
			Material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &Model->Materials[i].TextureNames[Material_Texture_AO]);
		else
			Model->Materials[i].TextureNames[Material_Texture_AO] = "";

		if (Material->GetTextureCount(aiTextureType_SPECULAR) > 0)
			Material->GetTexture(aiTextureType_SPECULAR, 0, &Model->Materials[i].TextureNames[Material_Texture_Specular]);
		else
			Model->Materials[i].TextureNames[Material_Texture_Specular] = "";

		if (Material->GetTextureCount(aiTextureType_SHININESS) > 0)
			Material->GetTexture(aiTextureType_SPECULAR, 0, &Model->Materials[i].TextureNames[Material_Texture_Gloss]);
		else
			Model->Materials[i].TextureNames[Material_Texture_Gloss] = "";
	}
}

static void AllocateVerticesAndIndices(model* Model)
{
	Model->NumVertices = 0;
	Model->NumIndices = 0;

	for (u32 i = 0; i < Scene->mNumMeshes; i++)
	{
		const aiMesh* Mesh = Scene->mMeshes[i];
		Model->NumVertices += Mesh->mNumVertices;
		Model->NumIndices += Mesh->mNumFaces * 3;
	}

	Model->Vertices = (vertex*)malloc(sizeof(vertex) * Model->NumVertices);
	Model->Indices = (u32*)malloc(sizeof(u32) * Model->NumIndices);

	for (u32 i = 0; i < Model->NumVertices; i++)
	{
		for (u32 j = 0; j < 4; j++)
		{
			Model->Vertices[i].BoneIDs[j] = 0;
			Model->Vertices[i].BoneWeights[j] = 0.0f;
		}
	}
}

static void LoadMeshes(model* Model)
{
	Model->NumMeshes = Scene->mNumMeshes;

	if (Model->NumMeshes == 0)
		return;

	Model->Meshes = (mesh*)malloc(sizeof(mesh) * Model->NumMeshes);
	u32 VertexOffset = 0;
	u32 IndexOffset = 0;
	for (u32 i = 0; i < Scene->mNumMeshes; i++)
	{
		const aiMesh* Mesh = Scene->mMeshes[i];

		for (u32 j = 0; j < Mesh->mNumVertices; j++)
		{
			vertex* Vertex = &Model->Vertices[VertexOffset + j];

			Vertex->Pos = Mesh->mVertices[j];
			if (Mesh->HasVertexColors(0))
				Vertex->Color = aiVector3D(Mesh->mColors[0][j].r, Mesh->mColors[0][j].g, Mesh->mColors[0][j].b);
			else
				Vertex->Color = aiVector3D(1.0f, 1.0f, 1.0f);

			if (Mesh->HasTextureCoords(0))
				Vertex->TexCoord = aiVector2D(Mesh->mTextureCoords[0][j].x, Mesh->mTextureCoords[0][j].y);
			else
				Vertex->TexCoord = aiVector2D(0.0f, 0.0f);

			Vertex->Normal = Mesh->mNormals[j];
			Vertex->Tangent = Mesh->mTangents[j];
		}

		for (u32 j = 0; j < Mesh->mNumBones; j++)
		{
			const aiBone* AiBone = Mesh->mBones[j];
			bone* Bone = 0;
			u32 BoneIndex = 0;
			for(u32 k = 0; k < Model->Bones.size(); k++)
			{
				if (Model->Bones[k].Name == AiBone->mName)
				{
					BoneIndex = k;
					Bone = &Model->Bones[k];
					break;
				}
			}

			if (Bone)
				Bone->BoneSpace = AiBone->mOffsetMatrix;
			
			for (u32 k = 0; k < AiBone->mNumWeights; k++)
			{
				const aiVertexWeight* Weight = &AiBone->mWeights[k];
				vertex* Vertex = &Model->Vertices[VertexOffset + Weight->mVertexId];
				u32 VertexBoneIndex = 100;
				for (u32 l = 0; l < 4; l++)
				{
					if (Vertex->BoneWeights[l] == 0.0f)
					{
						VertexBoneIndex = l;
						break;
					}
				}

				if (VertexBoneIndex != 100)
				{
					Vertex->BoneIDs[VertexBoneIndex] = BoneIndex;
					Vertex->BoneWeights[VertexBoneIndex] = Weight->mWeight;
				}
			}
		}

		for (u32 j = 0; j < Mesh->mNumFaces; j++)
		{
			const aiFace* Face = &Mesh->mFaces[j];
			Model->Indices[IndexOffset + j * 3 + 0] = Face->mIndices[0];
			Model->Indices[IndexOffset + j * 3 + 1] = Face->mIndices[1];
			Model->Indices[IndexOffset + j * 3 + 2] = Face->mIndices[2];
		}

		Model->Meshes[i].IndexOffset = IndexOffset;
		Model->Meshes[i].VertexOffset = VertexOffset;
		Model->Meshes[i].IndexCount = Mesh->mNumFaces * 3;
		Model->Meshes[i].MaterialIndex = Mesh->mMaterialIndex;
		Model->Meshes[i].MaterialModifierIndex = 0;

		VertexOffset += Mesh->mNumVertices;
		IndexOffset += Mesh->mNumFaces * 3;
	}
}

static void LoadAnimations(model* Model)
{
	if (!Scene->HasAnimations())
	{
		Model->NumAnimations = 0;
		Model->Animations = 0;
		return;
	}

	Model->NumAnimations = Scene->mNumAnimations;
	Model->Animations = (animation*)malloc(Model->NumAnimations * sizeof(animation));

	for (u32 i = 0; i < Scene->mNumAnimations; i++)
	{
		const aiAnimation* AiAnimation = Scene->mAnimations[i];
		animation* Animation = &Model->Animations[i];
		Animation->Name = AiAnimation->mName;
		Animation->Tps = AiAnimation->mTicksPerSecond;
		Animation->Durration = AiAnimation->mDuration;
		Animation->NumChannels = AiAnimation->mNumChannels;

		if (Animation->NumChannels > 0)
			Animation->Channels = (animation_channel*)malloc(sizeof(animation_channel) * Animation->NumChannels);

		for (u32 j = 0; j < AiAnimation->mNumChannels; j++)
		{
			const aiNodeAnim* NodeAnim = AiAnimation->mChannels[j];
			animation_channel* Channel = &Animation->Channels[j];
			
			Channel->NumPositionKeyFrames = NodeAnim->mNumPositionKeys;
			Channel->NumScalingKeyFrames = NodeAnim->mNumScalingKeys;
			Channel->NumRotationKeyFrames = NodeAnim->mNumRotationKeys;

			if (Channel->NumPositionKeyFrames > 0)
				Channel->PositionKeyFrames = (animation_vec_keyframe*)malloc(sizeof(animation_vec_keyframe) * Channel->NumPositionKeyFrames);
			if (Channel->NumScalingKeyFrames > 0)
				Channel->ScalingKeyFrames = (animation_vec_keyframe*)malloc(sizeof(animation_vec_keyframe) * Channel->NumScalingKeyFrames);
			if (Channel->NumRotationKeyFrames > 0)
				Channel->RotationKeyFrames = (animation_quat_keyframe*)malloc(sizeof(animation_quat_keyframe) * Channel->NumRotationKeyFrames);

			s32 BoneIndex = -1;
			for (u32 k = 0; k < Model->Bones.size(); k++)
			{
				if (NodeAnim->mNodeName == Model->Bones[k].Name)
				{
					Channel->BoneIndex = k;
					break;
				}
			}
			
			for (u32 k = 0; k < NodeAnim->mNumPositionKeys; k++)
			{
				Channel->PositionKeyFrames[k].time = NodeAnim->mPositionKeys[k].mTime;
				Channel->PositionKeyFrames[k].vec = NodeAnim->mPositionKeys[k].mValue;
			}
			
			for (u32 k = 0; k < NodeAnim->mNumRotationKeys; k++)
			{
				Channel->RotationKeyFrames[k].time = NodeAnim->mRotationKeys[k].mTime;
				Channel->RotationKeyFrames[k].quat.x = NodeAnim->mRotationKeys[k].mValue.x;
				Channel->RotationKeyFrames[k].quat.y = NodeAnim->mRotationKeys[k].mValue.y;
				Channel->RotationKeyFrames[k].quat.z = NodeAnim->mRotationKeys[k].mValue.z;
				Channel->RotationKeyFrames[k].quat.w = NodeAnim->mRotationKeys[k].mValue.w;
			}

			for (u32 k = 0; k < NodeAnim->mNumScalingKeys; k++)
			{
				Channel->ScalingKeyFrames[k].time = NodeAnim->mScalingKeys[k].mTime;
				Channel->ScalingKeyFrames[k].vec = NodeAnim->mScalingKeys[k].mValue;
			}
		}
	}
}

static void LoadBoneHierarchy(model* Model, const aiNode* Node)
{
	bone Bone;
	Bone.Name = Node->mName;
	Bone.Transform = Node->mTransformation;

	for (u32 i = 0; i < Node->mNumChildren; i++)
	{
		LoadBoneHierarchy(Model, Node->mChildren[i]);
		Bone.Children.push_back(Model->Bones.size() - 1);
	}

	Model->Bones.push_back(Bone);
	if (Node == Scene->mRootNode)
		Model->RootBone = Model->Bones.size() - 1;
}

static void LoadModel(model* Model)
{
	Model->GlobalInverseTransform = Scene->mRootNode->mTransformation;
	Model->GlobalInverseTransform.Inverse();
	LoadMaterials(Model);
	LoadBoneHierarchy(Model, Scene->mRootNode);
	AllocateVerticesAndIndices(Model);
	LoadMeshes(Model);
	LoadAnimations(Model);
}

int main(int argc, char** argv)
{
	Assimp::Importer Importer;

	if (argc == 1)
		return 1;

	char* filePath = argv[1];

	Scene = Importer.ReadFile(filePath, aiProcess_FlipUVs |
		aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_RemoveRedundantMaterials |
		aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

	if (!Scene)
		CreateErrorFile(Importer.GetErrorString());

	model Model;
	LoadModel(&Model);
	WriteModelToFile(filePath, &Model);

	return 0;
}