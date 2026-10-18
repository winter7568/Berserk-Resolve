#pragma once

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <filesystem>
#include <map>

#include "Model.h"

class AssimpImporter
{
private:
	using MeshList = std::vector<Model::Mesh>;
	using MaterialList = std::vector<Model::Material>;
	using NodeList = std::vector<Model::Node>;
	using AnimationList = std::vector<Model::Animation>;
public:
	AssimpImporter(const char* filename);

	//メッシュデータを読み込み
	//void LoadMeshes(MeshList& meshes);
	void LoadMeshes(MeshList& meshes, const NodeList& nodes);

	//マテリアルを読み込み
	void LoadMaterials(MaterialList& materials);
	
	//ノードデータを読み込み
	void LoadNodes(NodeList& nodes);

	// アニメーションデータを読み込み 
	void LoadAnimations(AnimationList& animations, const NodeList& nodes);

private:
	//メッシュデータを読み込み
	void LoadMeshes(MeshList& meshes, const NodeList& nodes,  const aiNode* aNode);

	//ノードデータ再帰読み込み
	void LoadNodes(NodeList& nodes, const aiNode* aNode, int parentIndex);

	// ノードインデックス取得 
	static int GetNodeIndex(const NodeList& nodes, const char* name);

	//aiQuaternion->XMFLOAT4
	static DirectX::XMFLOAT4 aiQuaternionToXMFLOAT4(const aiQuaternion& aValue);

	//aiVector3D->XMFLOAT3
	static DirectX::XMFLOAT3 AssimpImporter::aiVector3DToXMFLOAT3(const aiVector3D& aValue);

	//aiVector3D->XMFLOAT2
	static DirectX::XMFLOAT2 AssimpImporter::aiVector3DToXMFLOAT2(const aiVector3D& aValue);

	//aiColor3D->XMFLOAT4
	static DirectX::XMFLOAT4 AssimpImporter::aiColor3DToXMFLOAT4(const aiColor3D& aValue);

	//aiMatrix4x4->XMFLOAT4x4
	static DirectX::XMFLOAT4X4 aiMatrix4x4toXMFLOAT4X4(const aiMatrix4x4& aValue);

private:
	Assimp::Importer                 aImporter;
	const aiScene*                   aScene = nullptr;
	std::filesystem::path            filepath;
	std::map<const aiNode*, int>     nodeIndexMap;
};
