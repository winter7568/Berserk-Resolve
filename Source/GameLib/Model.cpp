#include <filesystem>
#include <fstream> 

#include <cereal/cereal.hpp> 
#include <cereal/archives/binary.hpp> 
#include <cereal/types/string.hpp> 
#include <cereal/types/vector.hpp> 

#include "GameLib/Misc.h"
#include "GameLib/AssimpImporter.h"
#include "GameLib/Model.h"
#include "GameLib/GpuResourceUtils.h"

namespace DirectX
{
	template<class Archive>
	void serialize(Archive& archive, XMUINT4& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y),
			cereal::make_nvp("z", v.z),
			cereal::make_nvp("w", v.w)
		);
	}

	template<class Archive>
	void serialize(Archive& archive, XMFLOAT2& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y)
		);
	}

	template<class Archive>
	void serialize(Archive& archive, XMFLOAT3& v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y),
			cereal::make_nvp("z", v.z)
		);
	}

	template<class Archive>
	void serialize(Archive & archive, XMFLOAT4 & v)
	{
		archive(
			cereal::make_nvp("x", v.x),
			cereal::make_nvp("y", v.y),
			cereal::make_nvp("z", v.z),
			cereal::make_nvp("w", v.w)
		);
	}

	template<class Archive>
	void serialize(Archive & archive, XMFLOAT4X4 & m)
	{
		archive(
			cereal::make_nvp("_11", m._11),
			cereal::make_nvp("_12", m._12),
			cereal::make_nvp("_13", m._13),
			cereal::make_nvp("_14", m._14),
			cereal::make_nvp("_21", m._21),
			cereal::make_nvp("_22", m._22),
			cereal::make_nvp("_23", m._23),
			cereal::make_nvp("_24", m._24),
			cereal::make_nvp("_31", m._31),
			cereal::make_nvp("_32", m._32),
			cereal::make_nvp("_33", m._33),
			cereal::make_nvp("_34", m._34),
			cereal::make_nvp("_41", m._41),
			cereal::make_nvp("_42", m._42),
			cereal::make_nvp("_43", m._43),
			cereal::make_nvp("_44", m._44)
		);
	}
}

template<class Archive>
void Model::Node::serialize(Archive& archive)
{
	archive(
		CEREAL_NVP(name),
		CEREAL_NVP(parentIndex),
		CEREAL_NVP(position),
		CEREAL_NVP(rotation),
		CEREAL_NVP(scale)
	);
}

template<class Archive>
void Model::Material::serialize(Archive& archive)
{
	archive(
		CEREAL_NVP(name),
		CEREAL_NVP(diffuseTextureFileName),
		CEREAL_NVP(normalTextureFileName),
		CEREAL_NVP(color),
		CEREAL_NVP(EmissiveTextureFileName),
		CEREAL_NVP(OcclusionFileName),
		CEREAL_NVP(MetalnessFileName),
		CEREAL_NVP(RoughnessFileName),
		CEREAL_NVP(emissiveColor),
		CEREAL_NVP(hasDissolve),
		CEREAL_NVP(dissolveThreshold),
		CEREAL_NVP(dissolveEdgeWidth),
		CEREAL_NVP(dissolveEdgeIntensity),
		CEREAL_NVP(emissiveStrength)
	);
}
template<class Archive>
void Model::Vertex::serialize(Archive & archive)
{
	archive(
		CEREAL_NVP(position),
		CEREAL_NVP(boneWeight),
		CEREAL_NVP(boneIndex),
		CEREAL_NVP(texcoord),
		CEREAL_NVP(normal),
		CEREAL_NVP(tangent)
	);
}

template<class Archive>
void Model::Bone::serialize(Archive & archive)
{
	archive(
		CEREAL_NVP(nodeIndex),
		CEREAL_NVP(offsetTransform)
	);
}

template<class Archive>
void Model::Mesh::serialize(Archive & archive)
{
	archive(
		CEREAL_NVP(vertices),
		CEREAL_NVP(indices),
		CEREAL_NVP(bones),
		CEREAL_NVP(nodeIndex),
		CEREAL_NVP(materialIndex)
	);
}

template<class Archive>
void Model::VectorKeyframe::serialize(Archive & archive)
{
	archive(
		CEREAL_NVP(seconds),
		CEREAL_NVP(value)
	);
}

template<class Archive>
void Model::QuaternionKeyframe::serialize(Archive & archive)
{
	archive(
		CEREAL_NVP(seconds),
		CEREAL_NVP(value)
	);
}

template<class Archive>
void Model::NodeAnim::serialize(Archive& archive)
{
	archive(
		CEREAL_NVP(positionKeyframes),
		CEREAL_NVP(rotationKeyframes),
		CEREAL_NVP(scaleKeyframes)
	);
}

template<class Archive>
void Model::Animation::serialize(Archive& archive)
{
	archive(
		CEREAL_NVP(name),
		CEREAL_NVP(secondsLength),
		CEREAL_NVP(nodeAnims)
	);
}

// シリアライズ 
void Model::Serialize(const char* filename)
{
	std::ofstream ostream(filename, std::ios::binary);
	if (ostream.is_open())
	{
		cereal::BinaryOutputArchive archive(ostream);

		try
		{
			archive(
				CEREAL_NVP(nodes),
				CEREAL_NVP(materials),
				CEREAL_NVP(meshes),
				CEREAL_NVP(animations)
			);
		}
		catch (...)
		{
			_ASSERT_EXPR_A(false, "Model serialize failed.");
		}
	}
}

// デシリアライズ 
void Model::Deserialize(const char* filename)
{
	std::ifstream istream(filename, std::ios::binary);
	if (istream.is_open())
	{
		cereal::BinaryInputArchive archive(istream);

		try
		{
			archive(
				CEREAL_NVP(nodes),
				CEREAL_NVP(materials),
				CEREAL_NVP(meshes),
				CEREAL_NVP(animations)
			);
		}
		catch (...)
		{
			_ASSERT_EXPR_A(false, "Model deserialize failed.");
		}
	}
	else
	{
		_ASSERT_EXPR_A(false, "Model File not found.");
	}
}


Model::Model(ID3D11Device* device, const char* filename, float scaling)
	: scaling(scaling)
{
	std::filesystem::path filepath(filename);
	std::filesystem::path dirpath(filepath.parent_path());

	// 独自形式のモデルファイルの存在確認 
	filepath.replace_extension(".cereal");
	if (std::filesystem::exists(filepath))
	{
		// 独自形式のモデルファイルの読み込み 
		Deserialize(filepath.string().c_str());
	}
	else
	{
		//ファイル読み込み
		AssimpImporter importer(filename);
		//マテリアルデータ読み取り
		importer.LoadMaterials(materials);

		//ノードデータ読み取り
		importer.LoadNodes(nodes);

		//メッシュデータ読み取り
		//importer.LoadMeshes(meshes);
		importer.LoadMeshes(meshes, nodes);

		// アニメーションデータ読み取り 
		importer.LoadAnimations(animations, nodes);

		// 独自形式のモデルファイルを保存 
		Serialize(filepath.string().c_str());
	}

	// ノードキャッシュ 
	nodeCaches.resize(nodes.size());

	//ノード
	for (size_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex)
	{
		Node& node = nodes.at(nodeIndex);

		//親子関係
		node.parent = node.parentIndex >= 0 ? &nodes.at(node.parentIndex) : nullptr;
		if (node.parent != nullptr)
		{
			node.parent->children.emplace_back(&node);
		}
	}

	for (Material& material : materials)
	{
		if (material.diffuseTextureFileName.empty())
		{
			//ダミーテクスチャ作成
			HRESULT hr = GpuResourceUtils::CreateDummyTexture(device,0xFFFFFFFF,
				material.diffuseMap.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		else
		{
			//デフューズテクスチャ読み込み
			std::filesystem::path diffuseTexturePath(dirpath / material.diffuseTextureFileName);
			HRESULT hr = GpuResourceUtils::LoadTexture(device, diffuseTexturePath.string().c_str(),
				material.diffuseMap.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}

		if (material.normalTextureFileName.empty())
		{
			// 法線ダミーテクスチャ作成 
			HRESULT hr = GpuResourceUtils::CreateDummyTexture(device, 0xFFFF7F7F,
				material.normalMap.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		else
		{
			// 法線テクスチャ読み込み 
			std::filesystem::path texturePath(dirpath / material.normalTextureFileName);
			HRESULT hr = GpuResourceUtils::LoadTexture(device, texturePath.string().c_str(),
				material.normalMap.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		if (material.EmissiveTextureFileName.empty())
		{
			//ダミーテクスチャ作成
			HRESULT hr = GpuResourceUtils::CreateDummyTexture(device, 0xFF000000,
				material.EmissiveMap.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		else
		{
			//エミッシブテクスチャ読み込み
			std::filesystem::path texturePath(dirpath / material.EmissiveTextureFileName);
			HRESULT hr = GpuResourceUtils::LoadTexture(device, texturePath.string().c_str(),
				material.EmissiveMap.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}

		if (material.OcclusionFileName.empty())
		{
			//ダミーテクスチャ作成
			HRESULT hr = GpuResourceUtils::CreateDummyTexture(device, 0xFFFFFFFF,
				material.OcclusionMap.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		else
		{
			//オックルゾンテクスチャ読み込み
			std::filesystem::path texturePath(dirpath / material.OcclusionFileName);
			HRESULT hr = GpuResourceUtils::LoadTexture(device, texturePath.string().c_str(),
				material.OcclusionMap.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}

		if (material.MetalnessFileName.empty())
		{
			//ダミーテクスチャ作成
			HRESULT hr = GpuResourceUtils::CreateDummyTexture(device, 0xFFFFFFFF,
				material.MetalnessMap.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		else
		{
			//メタールネッステクスチャ読み込み
			std::filesystem::path texturePath(dirpath / material.MetalnessFileName);
			HRESULT hr = GpuResourceUtils::LoadTexture(device, texturePath.string().c_str(),
				material.MetalnessMap.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		if (material.RoughnessFileName.empty())
		{
			//ダミーテクスチャ作成
			HRESULT hr = GpuResourceUtils::CreateDummyTexture(device, 0xFFFFFFFF,
				material.RoughnessMap.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
		else
		{
			//ラーフネッステクスチャ読み込み
			std::filesystem::path texturePath(dirpath / material.RoughnessFileName);
			HRESULT hr = GpuResourceUtils::LoadTexture(device, texturePath.string().c_str(),
				material.RoughnessMap.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}
	}

	//メッシュ
	for (Mesh& mesh : meshes)
	{
		//参考ノード設定
		mesh.node = &nodes.at(mesh.nodeIndex);

		//参照マテリアル設定
		mesh.material = &materials.at(mesh.materialIndex);

		//頂点バッファ
		{
			D3D11_BUFFER_DESC bufferDesc = {};
			D3D11_SUBRESOURCE_DATA subresourceData = {};

			bufferDesc.ByteWidth = static_cast<UINT>(sizeof(Vertex) * mesh.vertices.size());
			bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags = 0;
			bufferDesc.MiscFlags = 0;
			bufferDesc.StructureByteStride = 0;
			subresourceData.pSysMem = mesh.vertices.data();
			subresourceData.SysMemPitch = 0;
			subresourceData.SysMemSlicePitch = 0;
			//vertex buffer object creation
			HRESULT hr = device->CreateBuffer(&bufferDesc, &subresourceData, mesh.vertexBuffer.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}

		//インデックスバッファ
		{
			D3D11_BUFFER_DESC bufferDesc = {};
			D3D11_SUBRESOURCE_DATA subresourceData = {};

			bufferDesc.ByteWidth = static_cast<UINT>(sizeof(uint32_t) * mesh.indices.size());
			bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bufferDesc.CPUAccessFlags = 0;
			bufferDesc.MiscFlags = 0;
			bufferDesc.StructureByteStride = 0;
			subresourceData.pSysMem = mesh.indices.data();
			subresourceData.SysMemPitch = 0;
			subresourceData.SysMemSlicePitch = 0;
			//vertex buffer object creation
			HRESULT hr = device->CreateBuffer(&bufferDesc, &subresourceData, mesh.indexBuffer.GetAddressOf());
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		}

		//ボーン構築
		for (Bone& bone : mesh.bones)
		{
			//参照ノード設定
			bone.node = &nodes.at(bone.nodeIndex);
		}

	}
}

float Model::GetCurrentBlendWeight() const
{
	if (animationBlendSecondsLength <= 0) return 1.0f;
	return currentAnimationBlendSeconds / animationBlendSecondsLength;
}

// トランスフォーム更新処理 
void Model::UpdateTransform(const DirectX::XMFLOAT4X4& worldTransform)
{
	DirectX::XMMATRIX ParentWorldTransform = DirectX::XMLoadFloat4x4(&worldTransform);

	// 右手座標系から左手座標系へ変換する行列 
	//DirectX::XMMATRIX CoordinateSystemTransform = DirectX::XMMatrixScaling(-1, 1, 1);
	DirectX::XMMATRIX CoordinateSystemTransform = DirectX::XMMatrixScaling(-scaling, scaling, scaling);

	for (Node& node : nodes)
	{
		//ローカル行列算出
		DirectX::XMMATRIX S = DirectX::XMMatrixScaling(node.scale.x, node.scale.y, node.scale.z);
		DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&node.rotation));
		DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(node.position.x, node.position.y, node.position.z);
		DirectX::XMMATRIX LocalTransform = S * R * T;

		// グローバル行列算出 
		DirectX::XMMATRIX ParentGlobalTransform;
		if (node.parent != nullptr)
		{
			ParentGlobalTransform = DirectX::XMLoadFloat4x4(&node.parent->globalTransform);
		}
		else
		{
			ParentGlobalTransform = DirectX::XMMatrixIdentity();
		}
		DirectX::XMMATRIX GlobalTransform = LocalTransform * ParentGlobalTransform;

		// ワールド行列算出 
		DirectX::XMMATRIX WorldTransform = GlobalTransform * CoordinateSystemTransform * ParentWorldTransform;

		// 計算結果を格納 
		DirectX::XMStoreFloat4x4(&node.localTransform, LocalTransform);
		DirectX::XMStoreFloat4x4(&node.globalTransform, GlobalTransform);
		DirectX::XMStoreFloat4x4(&node.worldTransform, WorldTransform);
	}
}

// アニメーション再生 
void Model::PlayAnimation(int index, bool loop, float blendSeconds)
{
	currentAnimationIndex = index;
	currentAnimationSeconds = 0;
	animationLoop = loop;
	animationPlaying = true;

	// ブレンドパラメータ 
	animationBlending = blendSeconds > 0.0f;
	currentAnimationBlendSeconds = 0.0f;
	animationBlendSecondsLength = blendSeconds;
}

// アニメーション再生中か 
bool Model::IsPlayAnimation() const
{
	if (currentAnimationIndex < 0) return false;
	if (currentAnimationIndex >= animations.size()) return false;
	return animationPlaying;
}


// ノードポーズ取得
void Model::GetNodePoses(std::vector<NodeCache>& nodePoses) const
{
	if (nodePoses.size() != nodes.size())
	{
		nodePoses.resize(nodes.size());
	}
	for (size_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex)
	{
		const Node& node = nodes.at(nodeIndex);
		NodeCache& pose = nodePoses.at(nodeIndex);

		pose.position = node.position;
		pose.rotation = node.rotation;
		pose.scale = node.scale;
	}
}



// アニメーション更新処理 
void Model::UpdateAnimation(float elapsedTime)
{
	// 再生中のアニメーションがなければ終了
	if (!IsPlayAnimation()) return;

	Animation& anim = animations[currentAnimationIndex];

	// アニメーション時間を進める
	currentAnimationSeconds += elapsedTime;

	// アニメーション終了またはループ処理
	if (currentAnimationSeconds > anim.secondsLength)
	{
		if (animationLoop)
		{
			// ループ時は時間を巻き戻す
			currentAnimationSeconds = fmod(currentAnimationSeconds, anim.secondsLength);

			// マーカーをリセット
			for (auto& m : anim.markers)
				m.fired = false;
		}
		else
		{
			// 終了時間に固定して停止
			currentAnimationSeconds = anim.secondsLength;
			animationPlaying = false;
		}
	}

	// マーカーイベントの発火
	for (auto& marker : anim.markers)
	{
		if (!marker.fired && currentAnimationSeconds >= marker.time)
		{
			TriggerAnimationEvent(marker.name);  // PlayerAttackState::OnAnimationEvent を呼ぶ
			marker.fired = true;
		}
	}

	// 現在のアニメーションフレームを計算
	ComputeAnimation(elapsedTime);

	// ブレンドがあれば適用
	ComputeBlending(elapsedTime);
}

// アニメーション計算処理 
void Model::ComputeAnimation(float elapsedTime)
{
	if (!IsPlayAnimation()) return;

	// 指定のアニメーションデータを取得 
	const Animation& animation = animations.at(currentAnimationIndex);

	// ノード毎のアニメーション処理 
	for (size_t nodeIndex = 0; nodeIndex < animation.nodeAnims.size(); ++nodeIndex)
	{
		Node& node = nodes.at(nodeIndex);
		const NodeAnim& nodeAnim = animation.nodeAnims.at(nodeIndex);

		// 位置 
		for (size_t index = 0; index < nodeAnim.positionKeyframes.size() - 1; ++index)
		{
			// 現在の時間がどのキーフレームの間にいるか判定する 
			const VectorKeyframe& keyframe0 = nodeAnim.positionKeyframes.at(index);
			const VectorKeyframe& keyframe1 = nodeAnim.positionKeyframes.at(index + 1);
			if (currentAnimationSeconds >= keyframe0.seconds && currentAnimationSeconds < keyframe1.seconds)
			{
				// 再生時間とキーフレームの時間から補完率を算出する 
				float rate = (currentAnimationSeconds - keyframe0.seconds) / (keyframe1.seconds -
					keyframe0.seconds);

				// 前のキーフレームと次のキーフレームの姿勢を補完 
				DirectX::XMVECTOR V0 = DirectX::XMLoadFloat3(&keyframe0.value);
				DirectX::XMVECTOR V1 = DirectX::XMLoadFloat3(&keyframe1.value);
				DirectX::XMVECTOR V = DirectX::XMVectorLerp(V0, V1, rate);
				// 計算結果をノードに格納 
				DirectX::XMStoreFloat3(&node.position, V);
			}
		}
		// 回転 
		for (size_t index = 0; index < nodeAnim.rotationKeyframes.size() - 1; ++index)
		{
			// 現在の時間がどのキーフレームの間にいるか判定する 
			const QuaternionKeyframe& keyframe0 = nodeAnim.rotationKeyframes.at(index);
			const QuaternionKeyframe& keyframe1 = nodeAnim.rotationKeyframes.at(index + 1);
			if (currentAnimationSeconds >= keyframe0.seconds && currentAnimationSeconds < keyframe1.seconds)
			{
				// 再生時間とキーフレームの時間から補完率を算出する 
				float rate = (currentAnimationSeconds - keyframe0.seconds) / (keyframe1.seconds -
					keyframe0.seconds);

				// 前のキーフレームと次のキーフレームの姿勢を補完 
				DirectX::XMVECTOR Q0 = DirectX::XMLoadFloat4(&keyframe0.value);
				DirectX::XMVECTOR Q1 = DirectX::XMLoadFloat4(&keyframe1.value);
				DirectX::XMVECTOR Q = DirectX::XMQuaternionSlerp(Q0, Q1, rate);
				// 計算結果をノードに格納 
				DirectX::XMStoreFloat4(&node.rotation, Q);
			}
		}
		// スケール 
		for (size_t index = 0; index < nodeAnim.scaleKeyframes.size() - 1; ++index)
		{
			// 現在の時間がどのキーフレームの間にいるか判定する 
			const VectorKeyframe& keyframe0 = nodeAnim.scaleKeyframes.at(index);
			const VectorKeyframe& keyframe1 = nodeAnim.scaleKeyframes.at(index + 1);
			if (currentAnimationSeconds >= keyframe0.seconds && currentAnimationSeconds < keyframe1.seconds)
			{
				// 再生時間とキーフレームの時間から補完率を算出する 
				float rate = (currentAnimationSeconds - keyframe0.seconds) / (keyframe1.seconds -
					keyframe0.seconds);

				// 前のキーフレームと次のキーフレームの姿勢を補完 
				DirectX::XMVECTOR V0 = DirectX::XMLoadFloat3(&keyframe0.value);
				DirectX::XMVECTOR V1 = DirectX::XMLoadFloat3(&keyframe1.value);
				DirectX::XMVECTOR V = DirectX::XMVectorLerp(V0, V1, rate);
				// 計算結果をノードに格納 
				DirectX::XMStoreFloat3(&node.scale, V);
			}
		}
	}
}

void Model::ComputeBlending(float elapsedTime)
{
	if (!animationBlending)
	{
		return;
	}

	// ブレンド率の計算 
	float rate = currentAnimationBlendSeconds / animationBlendSecondsLength;

	// ブレンド計算 
	int count = static_cast<int>(nodes.size());
	for (int i = 0; i < count; ++i)
	{
		const NodeCache& cache = nodeCaches.at(i);
		Node& node = nodes.at(i);

		DirectX::XMVECTOR S0 = DirectX::XMLoadFloat3(&cache.scale);
		DirectX::XMVECTOR S1 = DirectX::XMLoadFloat3(&node.scale);
		DirectX::XMVECTOR R0 = DirectX::XMLoadFloat4(&cache.rotation);
		DirectX::XMVECTOR R1 = DirectX::XMLoadFloat4(&node.rotation);
		DirectX::XMVECTOR T0 = DirectX::XMLoadFloat3(&cache.position);
		DirectX::XMVECTOR T1 = DirectX::XMLoadFloat3(&node.position);

		DirectX::XMVECTOR S = DirectX::XMVectorLerp(S0, S1, rate);
		DirectX::XMVECTOR R = DirectX::XMQuaternionSlerp(R0, R1, rate);
		DirectX::XMVECTOR T = DirectX::XMVectorLerp(T0, T1, rate);

		DirectX::XMStoreFloat3(&node.scale, S);
		DirectX::XMStoreFloat4(&node.rotation, R);
		DirectX::XMStoreFloat3(&node.position, T);
	}

	// 時間経過 
	currentAnimationBlendSeconds += elapsedTime;
	if (currentAnimationBlendSeconds >= animationBlendSecondsLength)
	{
		currentAnimationBlendSeconds = animationBlendSecondsLength;
		animationBlending = false;
	}
}

// アニメーションインデックス取得
int Model::GetAnimationIndex(const char* name) const
{
	for (size_t animationIndex = 0; animationIndex < animations.size(); ++animationIndex)
	{
		if (animations.at(animationIndex).name == name)
		{
			return static_cast<int>(animationIndex);
		}
	}
	return -1;
}

// ノードインデックス取得
int Model::GetNodeIndex(const char* name) const
{
	for (size_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex)
	{
		if (nodes.at(nodeIndex).name == name)
		{
			return static_cast<int>(nodeIndex);
		}
	}
	return -1;
}

void Model::ComputeAnimation(int animationIndex, int nodeIndex, float time, NodeCache& nodePose) const
{
	const Animation& animation = animations.at(animationIndex);
	const NodeAnim& nodeAnim = animation.nodeAnims.at(nodeIndex);

	// 位置
	for (size_t index = 0; index < nodeAnim.positionKeyframes.size() - 1; ++index)
	{
		// 現在の時間がどのキーフレームの間にいるか判定する
		const VectorKeyframe& keyframe0 = nodeAnim.positionKeyframes.at(index);
		const VectorKeyframe& keyframe1 = nodeAnim.positionKeyframes.at(index + 1);
		if (time >= keyframe0.seconds && time <= keyframe1.seconds)
		{
			// 再生時間とキーフレームの時間から補完率を算出する
			float delta = keyframe1.seconds - keyframe0.seconds;
			if (delta > 0.0f)
			{
				float rate = (time - keyframe0.seconds) / delta;

				// 前のキーフレームと次のキーフレームの姿勢を補完
				DirectX::XMVECTOR V0 = DirectX::XMLoadFloat3(&keyframe0.value);
				DirectX::XMVECTOR V1 = DirectX::XMLoadFloat3(&keyframe1.value);
				DirectX::XMVECTOR V = DirectX::XMVectorLerp(V0, V1, rate);
				// 計算結果をノードに格納
				DirectX::XMStoreFloat3(&nodePose.position, V);
			}
		}
	}

	// 回転
	for (size_t index = 0; index < nodeAnim.rotationKeyframes.size() - 1; ++index)
	{
		// 現在の時間がどのキーフレームの間にいるか判定する
		const QuaternionKeyframe& keyframe0 = nodeAnim.rotationKeyframes.at(index);
		const QuaternionKeyframe& keyframe1 = nodeAnim.rotationKeyframes.at(index + 1);
		if (time >= keyframe0.seconds && time <= keyframe1.seconds)
		{
			// 再生時間とキーフレームの時間から補完率を算出する
			float delta = keyframe1.seconds - keyframe0.seconds;
			if (delta > 0.0f)
			{
				float rate = (time - keyframe0.seconds) / delta;

				// 前のキーフレームと次のキーフレームの姿勢を補完
				DirectX::XMVECTOR Q0 = DirectX::XMLoadFloat4(&keyframe0.value);
				DirectX::XMVECTOR Q1 = DirectX::XMLoadFloat4(&keyframe1.value);
				DirectX::XMVECTOR Q = DirectX::XMQuaternionSlerp(Q0, Q1, rate);
				// 計算結果をノードに格納
				DirectX::XMStoreFloat4(&nodePose.rotation, Q);
			}
		}
	}

	// スケール
	for (size_t index = 0; index < nodeAnim.scaleKeyframes.size() - 1; ++index)
	{
		// 現在の時間がどのキーフレームの間にいるか判定する
		const VectorKeyframe& keyframe0 = nodeAnim.scaleKeyframes.at(index);
		const VectorKeyframe& keyframe1 = nodeAnim.scaleKeyframes.at(index + 1);
		if (time >= keyframe0.seconds && time <= keyframe1.seconds)
		{
			// 再生時間とキーフレームの時間から補完率を算出する
			float delta = keyframe1.seconds - keyframe0.seconds;
			if (delta > 0.0f)
			{
				float rate = (time - keyframe0.seconds) / delta;

				// 前のキーフレームと次のキーフレームの姿勢を補完
				DirectX::XMVECTOR V0 = DirectX::XMLoadFloat3(&keyframe0.value);
				DirectX::XMVECTOR V1 = DirectX::XMLoadFloat3(&keyframe1.value);
				DirectX::XMVECTOR V = DirectX::XMVectorLerp(V0, V1, rate);
				// 計算結果をノードに格納
				DirectX::XMStoreFloat3(&nodePose.scale, V);
			}
		}
	}
}
// アニメーション計算
void Model::ComputeAnimation(int animationIndex, float time, std::vector<NodeCache>& nodePoses) const
{
	if (nodePoses.size() != nodes.size())
	{
		nodePoses.resize(nodes.size());
	}
	for (size_t nodeIndex = 0; nodeIndex < nodePoses.size(); ++nodeIndex)
	{
		ComputeAnimation(animationIndex, static_cast<int>(nodeIndex), time, nodePoses.at(nodeIndex));
	}
}

// ノードポーズ設定
void Model::SetNodePoses(const std::vector<NodeCache>& nodePoses)
{
	for (size_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex)
	{
		const NodeCache& pose = nodePoses.at(nodeIndex);
		Node& node = nodes.at(nodeIndex);

		node.position = pose.position;
		node.rotation = pose.rotation;
		node.scale = pose.scale;
	}
}
