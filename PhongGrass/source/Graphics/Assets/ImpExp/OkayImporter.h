#pragma once
#include "../Mesh.h"

namespace Okay
{
	class Importer
	{
	private:
		friend class ContentBrowser;

		static bool Load(std::string_view filePath, Mesh::MeshInfo& outData, std::string* texPath, std::string& matName);
	};
}

#ifdef SKELETAL
inline aiNodeAnim* Importer::FindAnimNode(aiNodeAnim** nodes, UINT numNodes, std::string_view name)
{
	for (UINT i = 0; i < numNodes; i++)
	{
		if (nodes[i]->mNodeName.C_Str() == name)
			return nodes[i];
	}

	return nullptr;
}

inline bool Importer::FixJoint(Okay::Joint& joint, aiNode* pRootNode)
{
	std::string name = std::string(joint.name) + "_$AssimpFbx$_Translation";
	aiNode* pNode = FindTraNode(pRootNode, name);
	joint.stamps.resize(1);

	if (!pNode)
		return false;

	joint.stamps[0].time = 0.f;

	joint.stamps[0].pos.x = pNode->mTransformation.a4;
	joint.stamps[0].pos.y = pNode->mTransformation.b4;
	joint.stamps[0].pos.z = pNode->mTransformation.c4;

	return true;
}

inline aiNode* Importer::FindTraNode(aiNode* parent, std::string_view jointName)
{
	aiNode* ptr = nullptr;

	for (size_t i = 0; i < parent->mNumChildren; i++)
	{
		if (parent->mChildren[i]->mName.C_Str() == jointName)
			ptr = parent->mChildren[i];

		else if (!ptr)
			ptr = FindTraNode(parent->mChildren[i], jointName);
	}

	return ptr;
}

inline int Importer::FindJointIndex(std::vector<Okay::Joint>& joints, std::string_view name)
{
	for (int i = 0; i < (int)joints.size(); i++)
	{
		if (joints[i].name == name)
			return i;
	}

	return -1;
}

inline aiNode* Importer::GetJointParentNode(std::vector<Okay::Joint>& joints, aiNode* child)
{
	// Is root?
	if (!child->mParent)
		return nullptr;

	// Is the parent node a joint?
	if (FindJointIndex(joints, child->mParent->mName.C_Str()) != -1)
		return child->mParent;

	// Continue searching...
	return GetJointParentNode(joints, child->mParent);
}

inline void Importer::SetParents(std::vector<Okay::Joint>& joints, aiNode* node)
{
	int currentJointIdx = FindJointIndex(joints, node->mName.C_Str());
	if (currentJointIdx != -1)
	{
		aiNode* pParent = GetJointParentNode(joints, node);
		if (pParent)
			joints[currentJointIdx].parentIdx = FindJointIndex(joints, pParent->mName.C_Str());
	}

	for (UINT i = 0; i < node->mNumChildren; i++)
		SetParents(joints, node->mChildren[i]);
}
#endif