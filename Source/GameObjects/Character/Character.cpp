#include <algorithm>
#include <cmath> 

#include "Character.h"

void Character::UpdateTransform()
{
	// 位置・回転・スケールからワールド行列を作成
	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);

	// 最終行列（S * R * T）
	DirectX::XMMATRIX WorldTransform = S * R * T;

	// 行列を保存してモデルに適用
	DirectX::XMStoreFloat4x4(&transform, WorldTransform);
	model->UpdateTransform(transform);
}

void Character::Turn(float elapsedTime, float vx, float vz, float speed)
{
	// フレーム時間で回転速度を調整
	speed *= elapsedTime;

	// 入力ベクトルの長さを計算
	float length = std::sqrt(vx * vx + vz * vz);
	if (length < CharacterConstants::INPUT_LENGTH_EPSILON) return; // 入力が小さい場合は何もしない
	// 正規化
	vx /= length;
	vz /= length;

	// 現在の前方向
	float frontX = std::sin(angle.y);
	float frontZ = std::cos(angle.y);

	// 入力方向から目標角度を計算
	float targetAngle = std::atan2(vx, vz);

	// 角度差を計算（-π?πに正規化）
	float angleDiff = targetAngle - angle.y;
	while (angleDiff > DirectX::XM_PI) angleDiff -= DirectX::XM_2PI;
	while (angleDiff < -DirectX::XM_PI) angleDiff += DirectX::XM_2PI;

	// 差が小さい場合はスナップ
	if (std::fabs(angleDiff) < CharacterConstants::SNAP_THRESHOLD)
	{
		angle.y = targetAngle;
		return;
	}

	// 回転量を制限して加算
	float absSpeed = std::abs(speed);  
	float rotationStep = std::clamp(angleDiff, -absSpeed, absSpeed);
	angle.y += rotationStep;

	// 角度を範囲内に保つ
	while (angle.y > DirectX::XM_PI) angle.y -= DirectX::XM_2PI;
	while (angle.y < -DirectX::XM_PI) angle.y += DirectX::XM_2PI;
}


DirectX::XMFLOAT3 Character::GetLeftHandPosition() const
{
	const char* boneName = GetLeftHandBoneName();
	if (!boneName) return { 0.0f, 0.0f, 0.0f };

	for (const auto& node : model->GetNodes())
	{
		if (node.name == boneName)
		{
			DirectX::XMMATRIX nodeGlobal = DirectX::XMLoadFloat4x4(&node.globalTransform);
			DirectX::XMMATRIX characterWorld = DirectX::XMLoadFloat4x4(&transform);
			DirectX::XMMATRIX flip = DirectX::XMMatrixScaling(-1.0f, 1.0f, 1.0f);
			DirectX::XMVECTOR offset = DirectX::XMVectorSet(
				leftHandOffset[0],
				leftHandOffset[1],
				leftHandOffset[2],
				0.0f
			);
			DirectX::XMVECTOR worldPos = DirectX::XMVector3Transform(offset, nodeGlobal * flip * characterWorld);
			DirectX::XMFLOAT3 pos;
			DirectX::XMStoreFloat3(&pos, worldPos);
			return pos;
		}
	}
	return { 0.0f, 0.0f, 0.0f };
}

// 現在のアニメーション名を取得
std::string Character::GetCurrentAnimationName() const
{
	if (!model || animationIndex < 0) return "None";

	const auto& animations = model->GetAnimations();

	// 有効なインデックスなら名前を返す
	if (animationIndex < animations.size()) {
		return animations[animationIndex].name;
	}
	return "Invalid Index";
}

void Character::UpdateInvincibleTimer(float elapsedTime)
{
	// 無敵時間を減らす
	if (invincibleTimer > 0.0f)
	{
		invincibleTimer -= elapsedTime;
	}
}

// 攻撃アニメーション名を取得
std::string Character::GetCurrentAttackAnimationName() const
{
	if (model && model->GetAnimations().size() > 0) {
		int currentAnimIndex = animationIndex;
		const auto& animations = model->GetAnimations();

		// 有効範囲なら取得
		if (currentAnimIndex >= 0 && currentAnimIndex < animations.size()) {
			return animations[currentAnimIndex].name;
		}
	}
	return "Unknown Animation";
}


void Character::GetBodyCapsuleEndpoints(DirectX::XMFLOAT3& outBase, DirectX::XMFLOAT3& outTip) const
{
	const auto& bodyParams = GetCapsuleParams();

	// カプセル端点計算用定数
	static constexpr float HALF_SIZE = CharacterConstants::HALF_SIZE;           // 半分を表す定数
	static constexpr float Y_AXIS_BASE = CharacterConstants::AXIS_CENTER;        // Y軸の底位置
	static constexpr float Y_AXIS_CENTER = CharacterConstants::AXIS_CENTER;       // Y軸の中心位置
	static constexpr float W_COMPONENT = CharacterConstants::W_COMPONENT;         // 座標変換用W成分

	// ワールド行列作成
	DirectX::XMMATRIX worldMatrix =
		DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) *
		DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z) *
		DirectX::XMMatrixTranslation(position.x, position.y, position.z);

	// オフセット
	DirectX::XMMATRIX offsetMatrix = DirectX::XMMatrixTranslation(
		bodyParams.offset.x,
		bodyParams.offset.y,
		bodyParams.offset.z
	);

	// カプセル回転（度→ラジアン）
	DirectX::XMMATRIX capsuleRotation = DirectX::XMMatrixRotationRollPitchYaw(
		DirectX::XMConvertToRadians(bodyParams.rotation.x),
		DirectX::XMConvertToRadians(bodyParams.rotation.y),
		DirectX::XMConvertToRadians(bodyParams.rotation.z)
	);

	// 最終行列
	DirectX::XMMATRIX final = capsuleRotation * offsetMatrix * worldMatrix;

	// ローカル空間の端点（Y軸方向）
	DirectX::XMVECTOR baseLocal = DirectX::XMVectorSet(Y_AXIS_CENTER, -bodyParams.height * HALF_SIZE, Y_AXIS_CENTER, W_COMPONENT);
	DirectX::XMVECTOR tipLocal = DirectX::XMVectorSet(Y_AXIS_CENTER, bodyParams.height * HALF_SIZE, Y_AXIS_CENTER, W_COMPONENT);

	// ワールド座標に変換
	DirectX::XMVECTOR baseWorld = DirectX::XMVector3TransformCoord(baseLocal, final);
	DirectX::XMVECTOR tipWorld = DirectX::XMVector3TransformCoord(tipLocal, final);

	// 結果を保存
	DirectX::XMStoreFloat3(&outBase, baseWorld);
	DirectX::XMStoreFloat3(&outTip, tipWorld);
}

void Character::UpdateWeaponAttachment()
{
    const char* rightHandName = "middle_01_r";

    auto& weapon = GetWeapon();
    if (!weapon.model) return;

    // 武器のローカル変換（スケール・回転・位置）
    DirectX::XMMATRIX weaponS = DirectX::XMMatrixScaling(weapon.scale.x, weapon.scale.y, weapon.scale.z);
    DirectX::XMMATRIX weaponR = DirectX::XMMatrixRotationRollPitchYaw(weapon.angle.x, weapon.angle.y, weapon.angle.z);
    DirectX::XMMATRIX weaponT = DirectX::XMMatrixTranslation(weapon.position.x, weapon.position.y, weapon.position.z);
    DirectX::XMMATRIX localOffset = weaponS * weaponR * weaponT;

    // GLTFのZ-up座標系をDirectXの座標系に変換
    DirectX::XMMATRIX zUpCorrection = DirectX::XMMatrixRotationX(-DirectX::XM_PIDIV2);

    // 座標系の反転（左右反転）
    DirectX::XMMATRIX coordinateFlip = DirectX::XMMatrixScaling(-1, 1, 1);

    // 武器ローカル変換を合成
    DirectX::XMMATRIX weaponLocal = zUpCorrection * localOffset;

    // モデル内のノードをループして右手ボーンを探す
    for (const Model::Node& node : model->GetNodes())
    {
        if (node.name == rightHandName)
        {
            // ボーンのグローバル変換とキャラクターのワールド変換を取得
            DirectX::XMMATRIX handGlobal = DirectX::XMLoadFloat4x4(&node.globalTransform);
            DirectX::XMMATRIX characterWorld = DirectX::XMLoadFloat4x4(&transform);

            // 最終的な武器のワールド行列を計算
            DirectX::XMMATRIX weaponWorld = weaponLocal * handGlobal * coordinateFlip * characterWorld;

            // 計算結果を保存
            DirectX::XMStoreFloat4x4(&weapon.transform, weaponWorld);
            break;
        }
    }

    // 武器モデルに変換を適用
    weapon.model->UpdateTransform(weapon.transform);
}

void Character::AdvanceAnimationTime(float elapsedTime)
{
	const Model::Animation& animation = model->GetAnimations().at(animationIndex);

	// 時間更新
	oldAnimationSeconds = animationSeconds;
	animationSeconds += elapsedTime;

	// ループ処理
	if (animationSeconds > animation.secondsLength)
	{
		if (animationLoop)
		{
			animationSeconds -= animation.secondsLength;
		}
		else
		{
			animationSeconds = animation.secondsLength; // 最後で止める
		}
	}
}

void Character::StartAnimation(int animIndex, bool loop)
{
	// アニメーション初期化
	animationIndex = animIndex;
	animationLoop = loop;
	animationSeconds = 0.0f;
	oldAnimationSeconds = 0.0f;

	// モデルに再生指示
	model->PlayAnimation(animationIndex, animationLoop, animationBlendSeconds);
}