#include <DirectXCollision.h>
#include <algorithm>

#include"Collision.h"

bool Collision::IntersectSphereVsSphere(
	const DirectX::XMFLOAT3& positionA,
	float radiusA,
	const DirectX::XMFLOAT3& positionB,
	float radiusB,
	DirectX::XMFLOAT3& outPositionB)
{
	DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&positionA);
	DirectX::XMVECTOR PositionB = DirectX::XMLoadFloat3(&positionB);
	DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(PositionB, PositionA);
	DirectX::XMVECTOR LengthSq = DirectX::XMVector3LengthSq(Vec);
	float lengthSq;
	DirectX::XMStoreFloat(&lengthSq, LengthSq);

	float range = radiusA + radiusB;
	if (lengthSq > range * range)
	{
		return false;
	}

	// ベクトル正規化
	Vec = DirectX::XMVector3Normalize(Vec);
	Vec = DirectX::XMVectorScale(Vec, range);
	PositionB = DirectX::XMVectorAdd(PositionA, Vec);

	DirectX::XMStoreFloat3(&outPositionB, PositionB);

	return true;
}

bool Collision::IntersectCylinderVsCylinder(
	const DirectX::XMFLOAT3& positionA,
	float radiusA,
	float heightA,
	const DirectX::XMFLOAT3& positionB,
	float radiusB,
	float heightB,
	DirectX::XMFLOAT3& outPositionB)
{
	// 上下判定
	if (positionA.y > positionB.y + heightB)
	{
		return false;
	}
	if (positionA.y + heightA < positionB.y)
	{
		return false;
	}

	float vx = positionB.x - positionA.x;
	float vz = positionB.z - positionA.z;

	float range = radiusA + radiusB;

	float distXZ = sqrtf(vx * vx + vz * vz);

	if (distXZ > range)
	{
		return false;
	}

	// 方向計算
	vx /= distXZ;
	vz /= distXZ;

	outPositionB.x = positionA.x + (vx * range);
	outPositionB.y = positionB.y;
	outPositionB.z = positionA.z + (vz * range);

	return true;
}

bool Collision::IntersectSphereVsCylinder(
	const DirectX::XMFLOAT3& spherePosition,
	float sphereRadius,
	const DirectX::XMFLOAT3& cylinderPosition,
	float cylinderRadius, float cylinderHeight,
	DirectX::XMFLOAT3& outCylinderPosition)
{
	// 上下判定
	if (spherePosition.y + sphereRadius < cylinderPosition.y)
	{
		return false;
	}
	if (spherePosition.y - sphereRadius > cylinderPosition.y + cylinderHeight)
	{
		return false;
	}

	float vx = cylinderPosition.x - spherePosition.x;
	float vz = cylinderPosition.z - spherePosition.z;

	float range = sphereRadius + cylinderRadius;

	float distXZ = sqrtf(vx * vx + vz * vz);

	if (distXZ > range)
	{
		return false;
	}

	// 方向計算
	vx /= distXZ;
	vz /= distXZ;

	outCylinderPosition.x = spherePosition.x + (vx * range);
	outCylinderPosition.y = cylinderPosition.y;
	outCylinderPosition.z = spherePosition.z + (vz * range);

	return true;
}

bool Collision::IntersectCapsuleVsSphere(
	const DirectX::XMFLOAT3& capsulePosition,
	float capsuleRadius,
	float capsuleHeight,
	const DirectX::XMFLOAT3& spherePosition,
	float sphereRadius,
	DirectX::XMFLOAT3& outSpherePosition)
{
	// カプセルの上下
	DirectX::XMVECTOR base = DirectX::XMLoadFloat3(&capsulePosition);
	DirectX::XMVECTOR top = DirectX::XMVectorAdd(base, DirectX::XMVectorSet(0.0f, capsuleHeight, 0.0f, 0.0f));

	DirectX::XMVECTOR sphere = DirectX::XMLoadFloat3(&spherePosition);

	DirectX::XMVECTOR seg = DirectX::XMVectorSubtract(top, base);

	DirectX::XMVECTOR baseToSphere = DirectX::XMVectorSubtract(sphere, base);
	DirectX::XMVECTOR segLenSqVec = DirectX::XMVector3Dot(seg, seg);

	float segLenSq;
	DirectX::XMStoreFloat(&segLenSq, segLenSqVec);
	if (segLenSq == 0.0f) return false;

	DirectX::XMVECTOR t = DirectX::XMVector3Dot(baseToSphere, seg);
	t = DirectX::XMVectorDivide(t, segLenSqVec);
	t = DirectX::XMVectorClamp(t, DirectX::XMVectorZero(), DirectX::XMVectorReplicate(1.0f));

	DirectX::XMVECTOR closest = DirectX::XMVectorMultiplyAdd(seg, t, base);

	DirectX::XMVECTOR d = DirectX::XMVectorSubtract(sphere, closest);
	DirectX::XMVECTOR distSqVec = DirectX::XMVector3LengthSq(d);

	float distSq;
	DirectX::XMStoreFloat(&distSq, distSqVec);

	float combinedRadius = capsuleRadius + sphereRadius;
	if (distSq > combinedRadius * combinedRadius)
		return false;

	// 押し戻し
	DirectX::XMVECTOR dir = DirectX::XMVector3Normalize(d);
	DirectX::XMVECTOR outPos = DirectX::XMVectorMultiplyAdd(dir, DirectX::XMVectorReplicate(combinedRadius), closest);

	DirectX::XMStoreFloat3(&outSpherePosition, outPos);
	return true;
}

bool Collision::IntersectCapsuleVsBox(
	const DirectX::XMFLOAT3& capsulePosition,
	float capsuleRadius,
	float capsuleHeight,
	const DirectX::XMFLOAT3& boxCenter,
	const DirectX::XMFLOAT3& boxSize,
	DirectX::XMFLOAT3& outBoxPosition)
{
	// カプセルの上下
	DirectX::XMVECTOR base = DirectX::XMLoadFloat3(&capsulePosition);
	DirectX::XMVECTOR top = DirectX::XMVectorAdd(base, DirectX::XMVectorSet(0.0f, capsuleHeight, 0.0f, 0.0f));

	// ボックスサイズ計算
	DirectX::XMFLOAT3 halfSize = {
		boxSize.x * 0.5f,
		boxSize.y * 0.5f,
		boxSize.z * 0.5f
	};

	DirectX::XMFLOAT3 boxMin = {
		boxCenter.x - halfSize.x,
		boxCenter.y - halfSize.y,
		boxCenter.z - halfSize.z
	};

	DirectX::XMFLOAT3 boxMax = {
		boxCenter.x + halfSize.x,
		boxCenter.y + halfSize.y,
		boxCenter.z + halfSize.z
	};

	// 最短距離探索
	float minDistSq = FLT_MAX;
	DirectX::XMVECTOR closestPoint = base;

	const int segments = 4;
	for (int i = 0; i <= segments; ++i)
	{
		float t = static_cast<float>(i) / static_cast<float>(segments);

		DirectX::XMVECTOR seg = DirectX::XMVectorSubtract(top, base);
		DirectX::XMVECTOR point = DirectX::XMVectorMultiplyAdd(seg, DirectX::XMVectorReplicate(t), base);

		// AABBにクランプ
		DirectX::XMVECTOR clampedVec = DirectX::XMVectorClamp(
			point,
			DirectX::XMLoadFloat3(&boxMin),
			DirectX::XMLoadFloat3(&boxMax)
		);

		// 距離計算
		DirectX::XMVECTOR d = DirectX::XMVectorSubtract(point, clampedVec);
		DirectX::XMVECTOR distSqVec = DirectX::XMVector3LengthSq(d);

		float distSq;
		DirectX::XMStoreFloat(&distSq, distSqVec);

		if (distSq < minDistSq)
		{
			minDistSq = distSq;
			closestPoint = point;
		}
	}

	if (minDistSq > capsuleRadius * capsuleRadius)
		return false;

	// 押し出し
	DirectX::XMVECTOR boxCenterVec = DirectX::XMLoadFloat3(&boxCenter);
	DirectX::XMVECTOR pushDir = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(boxCenterVec, closestPoint));
	DirectX::XMVECTOR newPos = DirectX::XMVectorMultiplyAdd(pushDir, DirectX::XMVectorReplicate(capsuleRadius + 0.01f), closestPoint);

	DirectX::XMStoreFloat3(&outBoxPosition, newPos);
	return true;
}

bool Collision::IntersectCapsuleVsCylinder(
	const DirectX::XMFLOAT3& capsuleBase,
	const DirectX::XMFLOAT3& capsuleTip,
	float capsuleRadius,
	const DirectX::XMFLOAT3& cylinderPosition,
	float cylinderRadius,
	float cylinderHeight,
	DirectX::XMFLOAT3& outCylinderPosition)
{
	// XZ平面で判定
	using namespace DirectX;

	XMVECTOR A = XMLoadFloat3(&capsuleBase);
	XMVECTOR B = XMLoadFloat3(&capsuleTip);
	XMVECTOR C = XMLoadFloat3(&cylinderPosition);

	XMFLOAT3 A2 = { capsuleBase.x, 0.0f, capsuleBase.z };
	XMFLOAT3 B2 = { capsuleTip.x, 0.0f, capsuleTip.z };
	XMFLOAT3 C2 = { cylinderPosition.x, 0.0f, cylinderPosition.z };

	XMVECTOR a = XMLoadFloat3(&A2);
	XMVECTOR b = XMLoadFloat3(&B2);
	XMVECTOR c = XMLoadFloat3(&C2);

	XMVECTOR ab = XMVectorSubtract(b, a);
	XMVECTOR ac = XMVectorSubtract(c, a);

	float abLenSq = XMVectorGetX(XMVector3LengthSq(ab));
	if (abLenSq == 0.0f)
		return false;

	float t = XMVectorGetX(XMVector3Dot(ac, ab)) / abLenSq;
	t = max(0.0f, min(1.0f, t));

	XMVECTOR closest = XMVectorMultiplyAdd(ab, XMVectorReplicate(t), a);

	XMVECTOR delta = XMVectorSubtract(c, closest);
	float distSq = XMVectorGetX(XMVector3LengthSq(delta));

	float range = capsuleRadius + cylinderRadius;

	if (distSq > range * range)
		return false;

	// 高さ判定
	float capMinY = min(capsuleBase.y, capsuleTip.y) - capsuleRadius;
	float capMaxY = max(capsuleBase.y, capsuleTip.y) + capsuleRadius;
	float cylMinY = cylinderPosition.y;
	float cylMaxY = cylinderPosition.y + cylinderHeight;

	if (capMaxY < cylMinY || capMinY > cylMaxY)
		return false;

	// 押し出し
	float dist = sqrtf(distSq);

	if (dist > 0.0001f)
	{
		float dx = XMVectorGetX(delta) / dist;
		float dz = XMVectorGetZ(delta) / dist;

		outCylinderPosition.x = XMVectorGetX(c) + dx * range;
		outCylinderPosition.y = XMVectorGetY(c);
		outCylinderPosition.z = XMVectorGetZ(c) + dz * range;
	}
	else
	{
		outCylinderPosition = cylinderPosition;
	}

	return true;
}

bool Collision::IntersectCapsuleVsSphere(
	const DirectX::XMFLOAT3& capsuleBase,
	const DirectX::XMFLOAT3& capsuleTip,
	float capsuleRadius,
	const DirectX::XMFLOAT3& spherePosition,
	float sphereRadius,
	DirectX::XMFLOAT3& outSpherePushPosition)
{
	using namespace DirectX;

	XMVECTOR A = XMLoadFloat3(&capsuleBase);
	XMVECTOR B = XMLoadFloat3(&capsuleTip);
	XMVECTOR S = XMLoadFloat3(&spherePosition);

	XMVECTOR ab = XMVectorSubtract(B, A);
	XMVECTOR ap = XMVectorSubtract(S, A);

	float abLenSq = XMVectorGetX(XMVector3LengthSq(ab));
	if (abLenSq == 0.0f)
		return false;

	float t = XMVectorGetX(XMVector3Dot(ap, ab)) / abLenSq;
	t = std::clamp(t, 0.0f, 1.0f);

	XMVECTOR closest = XMVectorMultiplyAdd(ab, XMVectorReplicate(t), A);

	XMVECTOR delta = XMVectorSubtract(S, closest);
	float distSq = XMVectorGetX(XMVector3LengthSq(delta));

	float range = capsuleRadius + sphereRadius;

	if (distSq > range * range)
		return false;

	float dist = sqrtf(distSq);

	if (dist > 0.0001f)
	{
		XMVECTOR normal = XMVectorScale(delta, 1.0f / dist);
		XMVECTOR push = XMVectorMultiplyAdd(normal, XMVectorReplicate(range), closest);
		XMStoreFloat3(&outSpherePushPosition, push);
	}
	else
	{
		// 完全重なり
		outSpherePushPosition = spherePosition;
	}

	return true;
}


bool Collision::IntersectOrientedBoxVsCylinder(
	const DirectX::XMFLOAT3& boxCenter,
	const DirectX::XMFLOAT3& boxSize,
	const DirectX::XMFLOAT3& boxAngle,
	const DirectX::XMFLOAT3& cylinderBase,
	float cylinderRadius,
	float cylinderHeight,
	DirectX::XMFLOAT3& outBoxPushPosition)
{
	using namespace DirectX;

	// ボックス変換行列
	XMMATRIX scale = XMMatrixScaling(boxSize.x, boxSize.y, boxSize.z);
	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(boxAngle.x, boxAngle.y, boxAngle.z);
	XMMATRIX translation = XMMatrixTranslation(boxCenter.x, boxCenter.y, boxCenter.z);
	XMMATRIX boxWorld = scale * rotation * translation;

	// ワールドからローカルへ
	XMMATRIX invBoxWorld = XMMatrixInverse(nullptr, boxWorld);

	// シリンダー上下
	XMVECTOR cylBase = XMLoadFloat3(&cylinderBase);
	XMVECTOR cylTop = XMVectorAdd(cylBase, XMVectorSet(0.0f, cylinderHeight, 0.0f, 0.0f));

	// ローカル空間へ変換
	XMVECTOR localBase = XMVector3TransformCoord(cylBase, invBoxWorld);
	XMVECTOR localTop = XMVector3TransformCoord(cylTop, invBoxWorld);

	// AABB
	XMVECTOR boxMin = XMVectorSet(-0.5f, -0.5f, -0.5f, 0);
	XMVECTOR boxMax = XMVectorSet(0.5f, 0.5f, 0.5f, 0);

	// 最短距離探索
	float minDistSq = FLT_MAX;
	XMVECTOR closestPoint;

	const int steps = 4;
	for (int i = 0; i <= steps; ++i)
	{
		float t = static_cast<float>(i) / steps;

		XMVECTOR point = XMVectorLerp(localBase, localTop, t);
		XMVECTOR clamped = XMVectorClamp(point, boxMin, boxMax);

		XMVECTOR delta = XMVectorSubtract(point, clamped);
		float distSq = XMVectorGetX(XMVector3LengthSq(delta));

		if (distSq < minDistSq)
		{
			minDistSq = distSq;
			closestPoint = point;
		}
	}

	if (minDistSq > cylinderRadius * cylinderRadius)
		return false;

	// 押し出し方向
	XMVECTOR pushDir = XMVector3Normalize(XMVectorSubtract(XMVectorZero(), closestPoint));
	XMVECTOR newLocal = XMVectorMultiplyAdd(pushDir, XMVectorReplicate(cylinderRadius + 0.01f), closestPoint);

	// ワールドへ戻す
	XMVECTOR result = XMVector3TransformCoord(newLocal, boxWorld);
	XMStoreFloat3(&outBoxPushPosition, result);

	return true;
}


// レイキャスト
bool Collision::RayCast(
	const DirectX::XMFLOAT3& start,
	const DirectX::XMFLOAT3& end,
	const DirectX::XMFLOAT4X4& worldTransform,
	const Model* model,
	DirectX::XMFLOAT3& hitPosition,
	DirectX::XMFLOAT3& hitNormal)
{
	bool hit = false;

	// 始点と終点からレイのベクトルと長さを求める
	DirectX::XMVECTOR WorldRayStart = DirectX::XMLoadFloat3(&start);
	DirectX::XMVECTOR WorldRayEnd = DirectX::XMLoadFloat3(&end);
	DirectX::XMVECTOR WorldRayVec = DirectX::XMVectorSubtract(WorldRayEnd, WorldRayStart);
	DirectX::XMVECTOR WorldRayLength = DirectX::XMVector3Length(WorldRayVec);

	float nearestDist = DirectX::XMVectorGetX(WorldRayLength);
	if (nearestDist <= 0.0f) return false;

	DirectX::XMMATRIX ParentWorldTransform = DirectX::XMLoadFloat4x4(&worldTransform);

	// モデル内の全てのメッシュと交差判定を行う
	const Model* resource = model;
	for (const Model::Mesh& mesh : resource->GetMeshes())
	{
		// メッシュのワールド行列を求める
		const Model::Node& node = model->GetNodes().at(mesh.nodeIndex);
		DirectX::XMMATRIX GlobalTransform = DirectX::XMLoadFloat4x4(&node.globalTransform);
		DirectX::XMMATRIX WorldTransform = DirectX::XMMatrixMultiply(GlobalTransform, ParentWorldTransform);

		// レイをメッシュのローカル空間に変換する
		DirectX::XMMATRIX InverseWorldTransform = DirectX::XMMatrixInverse(nullptr, WorldTransform);
		DirectX::XMVECTOR LocalRayStart = DirectX::XMVector3Transform(WorldRayStart, InverseWorldTransform);
		DirectX::XMVECTOR LocalRayEnd = DirectX::XMVector3Transform(WorldRayEnd, InverseWorldTransform);
		DirectX::XMVECTOR LocalRayVec = DirectX::XMVectorSubtract(LocalRayEnd, LocalRayStart);
		DirectX::XMVECTOR LocalRayDirection = DirectX::XMVector3Normalize(LocalRayVec);
		float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(LocalRayVec));

		// メッシュ内の全ての三角形と交差判定を行う
		for (size_t i = 0; i < mesh.indices.size(); i += 3)
		{
			// 三角形の頂点座標を取得
			const Model::Vertex& a = mesh.vertices.at(mesh.indices.at(i + 0));
			const Model::Vertex& b = mesh.vertices.at(mesh.indices.at(i + 1));
			const Model::Vertex& c = mesh.vertices.at(mesh.indices.at(i + 2));

			DirectX::XMVECTOR A = DirectX::XMLoadFloat3(&a.position);
			DirectX::XMVECTOR B = DirectX::XMLoadFloat3(&b.position);
			DirectX::XMVECTOR C = DirectX::XMLoadFloat3(&c.position);

			// 三角形の交差判定
			if (DirectX::TriangleTests::Intersects(LocalRayStart, LocalRayDirection, A, B, C, distance))
			{
				// メッシュのローカル空間での交点を求める
				DirectX::XMVECTOR LocalHitVec = DirectX::XMVectorScale(LocalRayDirection, distance);
				DirectX::XMVECTOR LocalHitPosition = DirectX::XMVectorAdd(LocalRayStart, LocalHitVec);

				// メッシュのローカル空間での交点をワールド空間に変換する
				DirectX::XMVECTOR WorldHitPosition = DirectX::XMVector3Transform(LocalHitPosition, WorldTransform);

				// ワールド空間でのレイの始点から交点までの距離を求める
				DirectX::XMVECTOR WorldHitVec = DirectX::XMVectorSubtract(WorldHitPosition, WorldRayStart);
				DirectX::XMVECTOR WorldHitDist = DirectX::XMVector3Length(WorldHitVec);
				float worldHitDist = DirectX::XMVectorGetX(WorldHitDist);

				// 交点までの距離が今までに計算した交点より近い場合は交差したと判定する
				if (worldHitDist <= nearestDist)
				{
					// メッシュのローカル空間での三角形の法線ベクトルを算出
					DirectX::XMVECTOR AB = DirectX::XMVectorSubtract(B, A);
					DirectX::XMVECTOR BC = DirectX::XMVectorSubtract(C, B);
					DirectX::XMVECTOR LocalHitNormal = DirectX::XMVector3Cross(AB, BC);

					// メッシュのローカル空間での法線ベクトルをワールド空間に変換する
					DirectX::XMVECTOR WorldHitNormal = DirectX::XMVector3TransformNormal(LocalHitNormal, WorldTransform);

					// 三角形の裏表判定（内積の結果がマイナスならば表向き）
					DirectX::XMVECTOR Dot = DirectX::XMVector3Dot(LocalRayDirection, LocalHitNormal);
					float dot = DirectX::XMVectorGetX(Dot);
					if (dot < 0)
					{
						// 交差したのでヒット情報を格納
						DirectX::XMStoreFloat3(&hitNormal, DirectX::XMVector3Normalize(WorldHitNormal));
						DirectX::XMStoreFloat3(&hitPosition, WorldHitPosition);

						// 交点までの距離を更新する
						nearestDist = worldHitDist;
						hit = true;
					}
				}
			}
		}
	}
	return hit;
}

bool Collision::IntersectCapsuleVsCapsule(
	const DirectX::XMFLOAT3& aBase,
	const DirectX::XMFLOAT3& aTip,
	float aRadius,
	const DirectX::XMFLOAT3& bBase,
	const DirectX::XMFLOAT3& bTip,
	float bRadius,
	DirectX::XMFLOAT3& outPushPositionB)
{
	using namespace DirectX;

	XMVECTOR A0 = XMLoadFloat3(&aBase);
	XMVECTOR A1 = XMLoadFloat3(&aTip);
	XMVECTOR B0 = XMLoadFloat3(&bBase);
	XMVECTOR B1 = XMLoadFloat3(&bTip);

	XMVECTOR u = XMVectorSubtract(A1, A0);
	XMVECTOR v = XMVectorSubtract(B1, B0);
	XMVECTOR w0 = XMVectorSubtract(A0, B0);

	float a = XMVectorGetX(XMVector3Dot(u, u));
	float b = XMVectorGetX(XMVector3Dot(u, v));
	float c = XMVectorGetX(XMVector3Dot(v, v));
	float d = XMVectorGetX(XMVector3Dot(u, w0));
	float e = XMVectorGetX(XMVector3Dot(v, w0));

	float SMALL_NUM = 1e-8f;
	float denom = a * c - b * b;

	float s = 0.0f;
	float t = 0.0f;

	// 2本の線分の最近接点を計算する
	// s = A線分上の位置 (0〜1)
	// t = B線分上の位置 (0〜1)

	if (denom < SMALL_NUM)
	{
		// ほぼ平行な場合（計算が不安定になるため特別処理）
		s = 0.0f;

		// B側をAに投影して最近点を決める
		if (c > SMALL_NUM) {
			t = std::clamp(e / c, 0.0f, 1.0f);
		}
		else {
			// 完全に同じ方向のとき
			t = 0.0f;
		}
	}
	else
	{
		// 通常の交差計算（2本の線分の最短距離）
		float sN = (b * e - c * d);
		float tN = (a * e - b * d);

		s = sN / denom;
		t = tN / denom;

		// sが範囲外ならクランプして再計算
		if (s < 0.0f) {
			s = 0.0f;
			if (c > SMALL_NUM) t = std::clamp(e / c, 0.0f, 1.0f);
		}
		else if (s > 1.0f) {
			s = 1.0f;
			if (c > SMALL_NUM) t = std::clamp((e + b) / c, 0.0f, 1.0f);
		}
		else {
			// t側も範囲チェック
			if (t < 0.0f) {
				t = 0.0f;
				if (a > SMALL_NUM) s = std::clamp(-d / a, 0.0f, 1.0f);
			}
			else if (t > 1.0f) {
				t = 1.0f;
				if (a > SMALL_NUM) s = std::clamp((b - d) / a, 0.0f, 1.0f);
			}
		}
	}

	// 最近接点をそれぞれ計算
	XMVECTOR closestPointA = XMVectorAdd(A0, XMVectorScale(u, s));
	XMVECTOR closestPointB = XMVectorAdd(B0, XMVectorScale(v, t));

	// 2点間の距離
	XMVECTOR delta = XMVectorSubtract(closestPointB, closestPointA);
	float distSq = XMVectorGetX(XMVector3LengthSq(delta));
	float dist = sqrtf(distSq);

	float range = aRadius + bRadius;

	// 当たっていない場合は終了
	if (distSq > range * range)
		return false;

	// 押し出し処理（めり込み解消）
	if (dist > 1e-6f)
	{
		// 正規化方向
		XMVECTOR normal = XMVectorScale(delta, 1.0f / dist);

		// めり込み量
		float overlap = range - dist;

		// Bを押し戻す
		XMVECTOR push = XMVectorMultiplyAdd(normal, XMVectorReplicate(overlap + 0.001f), closestPointB);
		XMStoreFloat3(&outPushPositionB, push);
	}
	else
	{
		// 完全に重なっている場合（方向が取れない）
		XMVECTOR center = XMVectorLerp(B0, B1, 0.5f);
		XMVECTOR push = XMVectorAdd(center, XMVectorSet(0.0f, range, 0.0f, 0.0f));
		XMStoreFloat3(&outPushPositionB, push);
	}

	return true;
}


bool Collision::IntersectSphereVsCapsule(
	const DirectX::XMFLOAT3& spherePosition,
	float sphereRadius,
	const DirectX::XMFLOAT3& capsuleBase,
	const DirectX::XMFLOAT3& capsuleTip,
	float capsuleRadius,
	DirectX::XMFLOAT3& outSpherePosition)
{
	using namespace DirectX;

	XMVECTOR P0 = XMLoadFloat3(&capsuleBase);
	XMVECTOR P1 = XMLoadFloat3(&capsuleTip);
	XMVECTOR S = XMLoadFloat3(&spherePosition);

	XMVECTOR AB = XMVectorSubtract(P1, P0);
	XMVECTOR AP = XMVectorSubtract(S, P0);

	float abLenSq = XMVectorGetX(XMVector3LengthSq(AB));

	// カプセルが点の場合
	if (abLenSq == 0.0f)
	{
		XMVECTOR center = P0;
		XMVECTOR delta = XMVectorSubtract(S, center);

		float distSq = XMVectorGetX(XMVector3LengthSq(delta));
		float range = sphereRadius + capsuleRadius;

		if (distSq > range * range)
			return false;

		if (distSq > 0.0001f)
		{
			XMVECTOR normal = XMVector3Normalize(delta);
			XMVECTOR push = XMVectorMultiplyAdd(normal, XMVectorReplicate(range), center);
			XMStoreFloat3(&outSpherePosition, push);
		}
		else
		{
			outSpherePosition = spherePosition;
		}

		return true;
	}

	float t = XMVectorGetX(XMVector3Dot(AP, AB)) / abLenSq;
	t = std::clamp(t, 0.0f, 1.0f);

	XMVECTOR closest = XMVectorMultiplyAdd(AB, XMVectorReplicate(t), P0);
	XMVECTOR delta = XMVectorSubtract(S, closest);

	float distSq = XMVectorGetX(XMVector3LengthSq(delta));
	float range = sphereRadius + capsuleRadius;

	if (distSq > range * range)
		return false;

	float dist = sqrtf(distSq);

	if (dist > 0.0001f)
	{
		XMVECTOR normal = XMVectorScale(delta, 1.0f / dist);
		XMVECTOR push = XMVectorMultiplyAdd(normal, XMVectorReplicate(range), closest);
		XMStoreFloat3(&outSpherePosition, push);
	}
	else
	{
		outSpherePosition = spherePosition;
		outSpherePosition.y += range;
	}

	return true;
}