#include "OkayImporter.h"
#include "Graphics/Assets/Mesh.h"

#include "assimp/cimport.h"
#include "assimp/importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include <DirectXMath.h>


namespace Okay
{
	bool Importer::Load(std::string_view filePath, Mesh::MeshInfo& outData, std::string* texPaths, std::string& matName)
	{
		Assimp::Importer importer;

		const aiScene* pScene = importer.ReadFile(filePath.data(),
			aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_JoinIdenticalVertices);

		OKAY_VERIFY(pScene);

		aiMesh* pMesh = pScene->mMeshes[0];	////
		aiMaterial* pMat = pScene->mMaterials[pMesh->mMaterialIndex];

		outData.name = pMesh->mName.C_Str();
		matName = pMat->GetName().C_Str();
		
		// Textures
		aiString aiStr;
		pMat->GetTexture(aiTextureType_DIFFUSE, 0, &aiStr);
		texPaths[0] = aiStr.C_Str();

		pMat->GetTexture(aiTextureType_SPECULAR, 0, &aiStr);
		texPaths[1] = aiStr.C_Str();


		// Vertex Positions
		outData.positions.resize(pMesh->mNumVertices);
		memcpy(outData.positions.data(), pMesh->mVertices, sizeof(DirectX::XMFLOAT3) * pMesh->mNumVertices);

		// Vertex UV (Assimp structures UV as float3)
		outData.uvs.resize(pMesh->mNumVertices);

		if (pMesh->mTextureCoords[0])
		{
			for (uint32_t i = 0; i < pMesh->mNumVertices; i++)
			{
				outData.uvs[i].x = pMesh->mTextureCoords[0][i].x;
				outData.uvs[i].y = pMesh->mTextureCoords[0][i].y;
			}
		}

		// Vertex Normal
		outData.normals.resize(pMesh->mNumVertices);
		memcpy(outData.normals.data(), pMesh->mNormals, sizeof(DirectX::XMFLOAT3) * pMesh->mNumVertices);

		// Indices
		uint32_t counter = 0;
		const uint32_t NumIndices = pMesh->mNumFaces * 3u;

		outData.indices.resize(NumIndices);
		for (uint32_t i = 0; i < pMesh->mNumFaces; i++)
		{
			outData.indices[counter++] = pMesh->mFaces[i].mIndices[0];
			outData.indices[counter++] = pMesh->mFaces[i].mIndices[1];
			outData.indices[counter++] = pMesh->mFaces[i].mIndices[2];
		}

		return true;
	}

#ifdef SKELETAL
	bool Importer::LoadSkeletal(std::string_view filePath, SkeletalVertexData& outData)
	{
		const aiScene* pScene = aiImportFile(filePath.data(),
			aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_JoinIdenticalVertices);

		VERIFY(pScene);

		aiMesh* pMesh = pScene->mMeshes[0];	////
		aiAnimation* pAni = pScene->mAnimations[0];

		// Mesh
		{
			// Vertex Positions
			outData.position.resize(pMesh->mNumVertices);
			memcpy(outData.position.data(), pMesh->mVertices, sizeof(glm::vec3) * pMesh->mNumVertices);


			// Vertex UV & Normals
			outData.uvNormal.resize(pMesh->mNumVertices);
			for (UINT i = 0; i < pMesh->mNumVertices; i++)
			{
				outData.uvNormal[i].normal.x = pMesh->mNormals[i].x;
				outData.uvNormal[i].normal.y = pMesh->mNormals[i].y;
				outData.uvNormal[i].normal.z = pMesh->mNormals[i].z;

				outData.uvNormal[i].uv.x = pMesh->mTextureCoords[0][i].x;
				outData.uvNormal[i].uv.y = pMesh->mTextureCoords[0][i].y;
			}


			// Indices
			UINT counter = 0;
			const UINT NumIndices = pMesh->mNumFaces * 3;

			outData.indices.resize(NumIndices);
			for (UINT i = 0; i < pMesh->mNumFaces; i++)
			{
				outData.indices[counter++] = pMesh->mFaces[i].mIndices[0];
				outData.indices[counter++] = pMesh->mFaces[i].mIndices[1];
				outData.indices[counter++] = pMesh->mFaces[i].mIndices[2];
			}
		}


		// Joints
		outData.weights.resize(outData.indices.size());
		outData.joints.resize(pMesh->mNumBones);

		outData.tickLengthS = 1.f / (float)pAni->mTicksPerSecond;
		outData.durationS = float(pAni->mDuration / pAni->mTicksPerSecond);

		std::vector<aiNodeAnim*> aniNodes(pAni->mNumChannels);
		memcpy(aniNodes.data(), pAni->mChannels, sizeof(aiNodeAnim*) * pAni->mNumChannels);

		for (UINT i = 0; i < pMesh->mNumBones; i++)
		{
			// Weights & Indices
			for (UINT k = 0; k < pMesh->mBones[i]->mNumWeights; k++)
			{
				aiVertexWeight& aiWeight = pMesh->mBones[i]->mWeights[k];
				SkinnedVertex& currVertex = outData.weights[aiWeight.mVertexId];

				if (currVertex.weight[0] == 0.f) { currVertex.weight[0] = aiWeight.mWeight; currVertex.jointIdx[0] = i; }
				else if (currVertex.weight[1] == 0.f) { currVertex.weight[1] = aiWeight.mWeight; currVertex.jointIdx[1] = i; }
				else if (currVertex.weight[2] == 0.f) { currVertex.weight[2] = aiWeight.mWeight; currVertex.jointIdx[2] = i; }
				else if (currVertex.weight[3] == 0.f) { currVertex.weight[3] = aiWeight.mWeight; currVertex.jointIdx[3] = i; }
			}


			// Joint Specific
			Joint& joint = outData.joints[i];

			joint.name = pMesh->mBones[i]->mName.C_Str();

			memcpy(&joint.invBindPose, &pMesh->mBones[i]->mOffsetMatrix, sizeof(DirectX::XMFLOAT4X4));
			joint.invBindPose = DirectX::XMMatrixTranspose(joint.invBindPose);

			aiNodeAnim* channel = FindAnimNode(pAni->mChannels, pAni->mNumChannels, joint.name);
			if (!channel)
			{
				printf("%s: channel was NULL\n", joint.name.c_str());
				if (!FixJoint(joint, pScene->mRootNode))
					printf("%s: failed fixing, using identity transform.\n", joint.name.c_str());

				continue;
			}

			// ASSUMING ALL STAMPS SAME LENGTH & TIME MATCHING
			if (channel->mNumPositionKeys != channel->mNumRotationKeys &&
				channel->mNumPositionKeys != channel->mNumScalingKeys &&
				channel->mNumRotationKeys != channel->mNumScalingKeys)
			{
				printf("%s: keys not matching\n", joint.name.c_str());
				if (!FixJoint(joint, pScene->mRootNode))
					printf("%s: failed fixing", joint.name.c_str());

				continue;
			}

			joint.stamps.resize(channel->mNumPositionKeys);
			for (UINT k = 0; k < channel->mNumPositionKeys; k++)
			{
				joint.stamps[k].time = (float)channel->mPositionKeys[k].mTime;

				memcpy(&joint.stamps[k].pos, &channel->mPositionKeys[k].mValue, sizeof(glm::vec3));
				memcpy(&joint.stamps[k].scale, &channel->mScalingKeys[k].mValue, sizeof(glm::vec3));

				// Assimp quaternion struct order (W, X, Y, Z) differs from Okay(X, Y, Z, W)
				joint.stamps[k].rot.x = channel->mRotationKeys[k].mValue.x;
				joint.stamps[k].rot.y = channel->mRotationKeys[k].mValue.y;
				joint.stamps[k].rot.z = channel->mRotationKeys[k].mValue.z;
				joint.stamps[k].rot.w = channel->mRotationKeys[k].mValue.w;
			}
		}

		SetParents(outData.joints, pScene->mRootNode);

		// Normalize Weights
		for (SkinnedVertex& weight : outData.weights)
		{
			float invSum = 1.f / (weight.weight[0] + weight.weight[1] + weight.weight[2] + weight.weight[3]);
			weight.weight[0] *= invSum;
			weight.weight[1] *= invSum;
			weight.weight[2] *= invSum;
			weight.weight[3] *= invSum;
		}

		aiReleaseImport(pScene);

		return true;
	}
#endif

#ifdef OKAY_ASSET
	bool Importer::WriteOkayAsset(const std::string& filePath, const VertexData& vertexData)
	{
		size_t pos = filePath.find_last_of('/');
		pos = pos == -1 ? filePath.find_last_of('\\') : pos;

		std::string fileName = filePath.substr(pos + 1);
		fileName = fileName.substr(0, fileName.find_last_of('.')) + ".okayAsset";

		std::ofstream writer("Content/Meshes/" + fileName, std::ios::binary | std::ios::trunc);
		VERIFY(writer);

		UINT info[2];
		info[0] = (UINT)vertexData.position.size();
		info[1] = (UINT)vertexData.indices.size();

		writer.write((const char*)info, sizeof(info));
		writer.write((const char*)vertexData.position.data(), sizeof(glm::vec3) * info[0]);
		writer.write((const char*)vertexData.uvNormal.data(), sizeof(UVNormal) * info[0]);
		writer.write((const char*)vertexData.indices.data(), sizeof(UINT) * info[1]);

		writer.close();

		return true;
	}

	bool Importer::LoadOkayAsset(const std::string& fileName, VertexData& vertexData)
	{
		std::ifstream reader("Content/Meshes/" + fileName + ".OkayAsset", std::ios::binary);
		VERIFY(reader);

		// info[0] = NumVertex, info[1] = NumIndex
		UINT info[2];
		reader.read((char*)info, sizeof(info));

		vertexData.position.resize(info[0]);
		vertexData.uvNormal.resize(info[0]);
		vertexData.indices.resize(info[1]);

		reader.read((char*)vertexData.position.data(), sizeof(glm::vec3) * info[0]);
		reader.read((char*)vertexData.uvNormal.data(), sizeof(UVNormal) * info[0]);
		reader.read((char*)vertexData.indices.data(), sizeof(UINT) * info[1]);

		reader.close();

		return true;
	}
#endif
}