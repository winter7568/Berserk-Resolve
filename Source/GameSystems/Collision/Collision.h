#pragma once
#include <DirectXMath.h>

#include "GameLib/Model.h"

class Collision
{
public:
	static bool IntersectSphereVsSphere(
		const DirectX::XMFLOAT3& positionA,
		float radiusA,
		const DirectX::XMFLOAT3& positionB,
		float radiusB,
		DirectX::XMFLOAT3& outPositionB
	);

	static bool IntersectCylinderVsCylinder(
		const DirectX::XMFLOAT3& positionA,
		float radiusA,
		float heightA,
		const DirectX::XMFLOAT3& positionB,
		float radiusB,
		float heightB,
		DirectX::XMFLOAT3& outPositionB
	);

	// カプセルと球
	static bool IntersectCapsuleVsSphere(
		const DirectX::XMFLOAT3& capsuleBase,
		const DirectX::XMFLOAT3& capsuleTip,
		float capsuleRadius,
		const DirectX::XMFLOAT3& spherePosition,
		float sphereRadius,
		DirectX::XMFLOAT3& outSpherePushPosition
	);

	static bool IntersectSphereVsCylinder(
		const DirectX::XMFLOAT3& spherePosition,
		float sphereRadius,
		const DirectX::XMFLOAT3& cylinderPosition,
		float cylinderRadius,
		float cylinderHeight,
		DirectX::XMFLOAT3& outCylinderPosition
	);

	// 回転ボックスとシリンダー
	static bool IntersectOrientedBoxVsCylinder(
		const DirectX::XMFLOAT3& boxCenter,
		const DirectX::XMFLOAT3& boxSize,
		const DirectX::XMFLOAT3& boxAngle,
		const DirectX::XMFLOAT3& cylinderBase,
		float cylinderRadius,
		float cylinderHeight,
		DirectX::XMFLOAT3& outBoxPushPosition
	);

	// カプセルと球（高さあり）
	static bool IntersectCapsuleVsSphere(
		const DirectX::XMFLOAT3& capsulePosition,
		float capsuleRadius,
		float capsuleHeight,
		const DirectX::XMFLOAT3& spherePosition,
		float sphereRadius,
		DirectX::XMFLOAT3& outSpherePosition
	);

	static bool IntersectCapsuleVsBox(
		const DirectX::XMFLOAT3& capsulePosition,
		float capsuleRadius,
		float capsuleHeight,
		const DirectX::XMFLOAT3& boxCenter,
		const DirectX::XMFLOAT3& boxSize,
		DirectX::XMFLOAT3& outBoxPosition
	);

	static bool IntersectCapsuleVsCylinder(
		const DirectX::XMFLOAT3& capsuleBase,
		const DirectX::XMFLOAT3& capsuleTip,
		float capsuleRadius,
		const DirectX::XMFLOAT3& cylinderPosition,
		float cylinderRadius,
		float cylinderHeight,
		DirectX::XMFLOAT3& outCylinderPosition
	);

	static bool IntersectCapsuleVsCapsule(
		const DirectX::XMFLOAT3& aBase,
		const DirectX::XMFLOAT3& aTip,
		float aRadius,
		const DirectX::XMFLOAT3& bBase,
		const DirectX::XMFLOAT3& bTip,
		float bRadius,
		DirectX::XMFLOAT3& outPushPositionB
	);

	// レイキャスト
	static bool RayCast(
		const DirectX::XMFLOAT3& start,
		const DirectX::XMFLOAT3& end,
		const DirectX::XMFLOAT4X4& worldTransform,
		const Model* model,
		DirectX::XMFLOAT3& hitPosition,
		DirectX::XMFLOAT3& hitNormal
	);

	static bool IntersectSphereVsCapsule(
		const DirectX::XMFLOAT3& spherePosition,
		float sphereRadius,
		const DirectX::XMFLOAT3& capsuleBase,
		const DirectX::XMFLOAT3& capsuleTip,
		float capsuleRadius,
		DirectX::XMFLOAT3& outSpherePosition
	);
};