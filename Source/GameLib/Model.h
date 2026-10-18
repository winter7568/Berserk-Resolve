#pragma once

#include <vector>
#include <DirectXMath.h>
#include <wrl.h>
#include <d3d11.h>
#include <string>
#include <functional>

class Model
{
public:
	Model(ID3D11Device* device, const char* filename, float scaling = 1.0f);

	struct VectorKeyframe
	{
		float    seconds;
		DirectX::XMFLOAT3  value;

		template<class Archive>
		void serialize(Archive& archive);
	};

	struct QuaternionKeyframe
	{
		float    seconds;
		DirectX::XMFLOAT4  value;

		template<class Archive>
		void serialize(Archive& archive);
	};

	struct NodeAnim
	{
		std::vector<VectorKeyframe>  positionKeyframes;
		std::vector<QuaternionKeyframe> rotationKeyframes;
		std::vector<VectorKeyframe>  scaleKeyframes;

		template<class Archive>
		void serialize(Archive& archive);
	};

	struct AnimationMarker
	{
		std::string name;   // e.g. "HitActive"
		float time;         // time in seconds when it triggers
		bool fired = false; // internal use: avoid firing twice in same play
	};

	struct Animation
	{
		std::string            name;
		float                  secondsLength;
		std::vector<NodeAnim>  nodeAnims;

		std::vector<AnimationMarker> markers;

		template<class Archive>
		void serialize(Archive& archive);
	};


	struct Vertex
	{
		DirectX::XMFLOAT3    position   = { 0,0,0 };
		DirectX::XMFLOAT4    boneWeight = { 1,0,0,0 };
		DirectX::XMUINT4     boneIndex  = { 0,0,0,0 };
		DirectX::XMFLOAT2    texcoord   = { 0,0 };
		DirectX::XMFLOAT3    normal     = { 0,0,0 };
		DirectX::XMFLOAT3    tangent    = { 0, 0, 0 };

		template<class Archive>
		void serialize(Archive& archive);
	};

	struct Material
	{
		DirectX::XMFLOAT4       color = { 1,1,1,1 };
		DirectX::XMFLOAT3       emissiveColor = { 1.0f, 1.0f, 1.0f };

		bool hasDissolve = false;
		float dissolveThreshold = 0.0f;
		float dissolveEdgeWidth = 0.1f;
		float dissolveEdgeIntensity = 1.0f;
		float emissiveStrength = 1.0f;

		std::string             name;
		std::string             diffuseTextureFileName;
		std::string             normalTextureFileName;
		std::string             EmissiveTextureFileName;
		std::string             OcclusionFileName;
		std::string             MetalnessFileName;
		std::string             RoughnessFileName;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> diffuseMap;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalMap;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> EmissiveMap;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> OcclusionMap;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> MetalnessMap;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> RoughnessMap;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> dissolveMask;

		template<class Archive>
		void serialize(Archive& archive);
	};

	float emissiveStrengthMultiplier = 1.0f;
	float environmentStrength = 0.02f;

	struct Node
	{
		std::string          name;
		int                  parentIndex;
		DirectX::XMFLOAT3    position;
		DirectX::XMFLOAT4    rotation;
		DirectX::XMFLOAT3    scale;

		DirectX::XMFLOAT4X4  localTransform;
		DirectX::XMFLOAT4X4  worldTransform;

		Node*                parent = nullptr;
		std::vector<Node*>   children;

		DirectX::XMFLOAT4X4  globalTransform;
		//DirectX::XMFLOAT4X4  woorldTransform;

		template<class Archive>
		void serialize(Archive& archive);
	};
	Node* GetRootNode() { return nodes.data(); }

	struct NodeCache
	{
		DirectX::XMFLOAT3 position = { 0, 0, 0 };
		DirectX::XMFLOAT4 rotation = { 0, 0, 0, 1 };
		DirectX::XMFLOAT3 scale = { 1, 1, 1 };
	};
	std::vector<NodeCache>  nodeCaches;

	struct Bone
	{
		int                   nodeIndex;
		DirectX::XMFLOAT4X4   offsetTransform;

		Node*                 node = nullptr;

		template<class Archive>
		void serialize(Archive& archive);
	};
	struct Mesh
	{
		std::vector<Vertex>                   vertices;
		std::vector<uint32_t>                 indices;
		int                                   materialIndex = 0;

		Microsoft::WRL::ComPtr<ID3D11Buffer>  vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>  indexBuffer;
		Material* material = nullptr;
		int                                   nodeIndex;
		Node*                                 node = nullptr;
		std::vector<Bone>                     bones;

		template<class Archive>
		void serialize(Archive& archive);
	};
	const std::vector<Mesh>& GetMeshes() const { return meshes; }
	const std::vector<Material>& GetMaterial() const { return materials; }
	const std::vector<Node>& GetNodes() const { return nodes; }
	bool IsBlending() const { return animationBlending; }
	float GetCurrentBlendWeight() const; 

	void UpdateTransform(const DirectX::XMFLOAT4X4& worldTransform);

	// アニメーションデータ取得 
	const std::vector<Animation>& GetAnimations() const { return animations; }

	// アニメーション再生 
	//void PlayAnimation(int index, bool loop);
	void PlayAnimation(int index, bool loop, float blendSeconds = 0.0f);

	// アニメーション再生中か 
	bool IsPlayAnimation() const;

	// アニメーション更新処理 
	void UpdateAnimation(float elapsedTime);

	// ノードポーズ取得
	void GetNodePoses(std::vector<NodeCache>& nodePoses) const;

	// アニメーションインデックス取得
	int GetAnimationIndex(const char* name) const;

	// アニメーション計算
	void ComputeAnimation(int animationIndex, int nodeIndex, float time, NodeCache& nodePose) const;
	void ComputeAnimation(int animationIndex, float time, std::vector<NodeCache>& nodePoses) const;

	// ノードポーズ設定
	void SetNodePoses(const std::vector<NodeCache>& nodePoses);

	// ノードインデックス取得
	int GetNodeIndex(const char* name) const;

	// Register / clear animation event callbacks
	void SetAnimationEventCallback(std::function<void(const std::string&)> callback) {
		animationEventCallback = std::move(callback);
	}

	void ClearAnimationEventCallback() {
		animationEventCallback = nullptr;
	}

	// Trigger manually (used internally by UpdateAnimation)
	void TriggerAnimationEvent(const std::string& eventName) {
		if (animationEventCallback) {
			animationEventCallback(eventName);
		}
	}

	bool IsAnimationFinished() const {
		if (currentAnimationIndex < 0 || currentAnimationIndex >= (int)animations.size())
			return true;
		return currentAnimationSeconds >= animations[currentAnimationIndex].secondsLength;
	}

	void EnableDissolve(bool enable)
	{
		for (auto& material : materials)
		{
			material.hasDissolve = enable;
		}
	}

	void SetDissolveThreshold(float threshold)
	{
		for (auto& material : materials)
		{
			material.dissolveThreshold = threshold;
		}
	}

	void SetDissolveMask(ID3D11ShaderResourceView* maskSRV)
	{
		for (auto& material : materials)
		{
			material.dissolveMask = maskSRV;
		}
	}

	float GetDissolveThreshold() const
	{
		if (materials.empty()) return 0.0f;
		return materials[0].dissolveThreshold;
	}

	void SetDissolveEdgeWidth(float width)
	{
		for (auto& material : materials)
		{
			material.dissolveEdgeWidth = width;
		}
	}

	void SetDissolveEdgeIntensity(float intensity)
	{
		for (auto& material : materials)
		{
			material.dissolveEdgeIntensity = intensity;
		}
	}

private:
	// アニメーション計算処理 
	void ComputeAnimation(float elapsedTime);

	// ブレンディング計算処理 
	void ComputeBlending(float elapsedTime);

	// シリアライズ 
	void Serialize(const char* filename);

	// デシリアライズ 
	void Deserialize(const char* filename);

private:
	std::function<void(const std::string&)> animationEventCallback;

	std::vector<Mesh>     meshes;
	std::vector<Material> materials;
	std::vector<Node>     nodes;

	std::vector<Animation>  animations;

	int  currentAnimationIndex = -1;
	float currentAnimationSeconds = 0;
	bool  animationPlaying = false;
	bool  animationLoop = false;


	float currentAnimationBlendSeconds = 0.0f;
	float animationBlendSecondsLength = -1.0f;
	bool  animationBlending = false;

	float scaling = 1.0f;
};