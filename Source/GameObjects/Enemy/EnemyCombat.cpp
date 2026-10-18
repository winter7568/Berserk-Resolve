#include <imgui.h>
#include "Enemy.h"
#include "GameSystems/Effect/EffectManager.h"
#include "GameLib/Graphics.h"
#include "GameLib/TransformUtils.h"

// ============================================================================
// PUBLIC METHODS - Damage & Combat
// ============================================================================

void Enemy::ApplyDamage(int damage, float invincibleTime, const HitData& hitData)
{
    // ダメージを受けて体力を減らす
    health -= damage;
    if (health <= 0) {
        health = 0;
    }

    // 一定時間無敵状態にする
    invincibleTimer = invincibleTime;

    // ヒット情報を保存
    lastHitData = hitData;
    lastHitData.damage = damage;
}

void Enemy::SetEmissiveColor(const DirectX::XMFLOAT3& color)
{
    currentEmissiveColor = color;

    // モデルの全マテリアルに適用
    for (auto& material : model->GetMaterial())
    {
        const_cast<Model::Material&>(material).emissiveColor = color;
    }

    // 武器モデルにも適用
    if (sword.model)
    {
        for (auto& material : sword.model->GetMaterial())
        {
            const_cast<Model::Material&>(material).emissiveColor = color;
        }
    }
}

void Enemy::ResetEmissiveColor()
{
    SetEmissiveColor(defaultEmissiveColor);
}

void Enemy::FlashEmissive(const DirectX::XMFLOAT3& color, float duration)
{
    flashColor = color;
    emissiveFlashTimer = duration;
    SetEmissiveColor(color);
}

void Enemy::ApplyBerserkSwordPose()
{
    sword.position = EnemyConstants::BERSERK_SWORD_POS;
    sword.angle = EnemyConstants::BERSERK_SWORD_ROT;
    sword.scale = EnemyConstants::BERSERK_SWORD_SCALE;
}

void Enemy::ApplyDefaultSwordPose()
{
    sword.position = EnemyConstants::DEFAULT_SWORD_POS;
    sword.angle = EnemyConstants::DEFAULT_SWORD_ROT;
    sword.scale = EnemyConstants::DEFAULT_SWORD_SCALE;
}

// ============================================================================
// PUBLIC METHODS - Weapon
// ============================================================================
bool Enemy::GetWeaponCapsule(DirectX::XMFLOAT3& outBase, DirectX::XMFLOAT3& outTip, float& outRadius)
{
    auto& weaponParams = GetWeaponCapsuleParams();

    // 武器のワールド行列を取得
    DirectX::XMMATRIX swordMatrix = DirectX::XMLoadFloat4x4(&sword.transform);

    // カプセルの回転を適用
    DirectX::XMMATRIX capsuleRotation = DirectX::XMMatrixRotationRollPitchYaw(
        DirectX::XMConvertToRadians(weaponParams.rotation.x),
        DirectX::XMConvertToRadians(weaponParams.rotation.y),
        DirectX::XMConvertToRadians(weaponParams.rotation.z)
    );

    // カプセルのオフセットを適用
    DirectX::XMMATRIX offsetMatrix = DirectX::XMMatrixTranslation(
        weaponParams.offset.x,
        weaponParams.offset.y,
        weaponParams.offset.z
    );

    // 全ての変換を合成
    DirectX::XMMATRIX final = capsuleRotation * offsetMatrix * swordMatrix;

    // カプセルのローカル空間での端点を設定（Z軸方向）
    DirectX::XMVECTOR baseLocal = DirectX::XMVectorSet(CharacterConstants::Z_AXIS_CENTER,
        CharacterConstants::Z_AXIS_CENTER, -weaponParams.height * CharacterConstants::HALF_SIZE,
        CharacterConstants::W_COMPONENT);
    DirectX::XMVECTOR tipLocal = DirectX::XMVectorSet(CharacterConstants::Z_AXIS_CENTER,
        CharacterConstants::Z_AXIS_CENTER, weaponParams.height * CharacterConstants::HALF_SIZE,
        CharacterConstants::W_COMPONENT);

    // ワールド空間に変換
    DirectX::XMVECTOR baseWorld = DirectX::XMVector3TransformCoord(baseLocal, final);
    DirectX::XMVECTOR tipWorld = DirectX::XMVector3TransformCoord(tipLocal, final);

    // 結果を出力
    DirectX::XMStoreFloat3(&outBase, baseWorld);
    DirectX::XMStoreFloat3(&outTip, tipWorld);

    // スケールから半径を計算
    DirectX::XMVECTOR rightVec = final.r[CharacterConstants::RIGHT_VECTOR_INDEX];
    DirectX::XMVECTOR upVec = final.r[CharacterConstants::UP_VECTOR_INDEX];
    float scaleX = DirectX::XMVectorGetX(DirectX::XMVector3Length(rightVec));
    float scaleY = DirectX::XMVectorGetX(DirectX::XMVector3Length(upVec));
    float radiusScale = max(scaleX, scaleY);

    outRadius = weaponParams.radius * radiusScale;

    return true;
}

// ============================================================================
// PUBLIC METHODS - Effects
// ============================================================================

void Enemy::StartPowerChargeEffect(DirectX::XMFLOAT3& startPosition)
{
    float effectScale = IsBerserk() ?
        EnemyConstants::BERSERK_EFFECT_SCALE : EnemyConstants::NORMAL_EFFECT_SCALE;

    EffectManager::Instance().PlayEffect(
        "EnemyDomain2",
        position,
        effectScale
    );

    PlayEnemySound(EnemySound::AREA_WARNING, EnemyConstants::WARNING_VOLUME);
}

void Enemy::StopPowerChargeEffect()
{
    EffectManager::Instance().StopEffect("EnemyDomain2");
}

// ============================================================================
// PUBLIC METHODS - Sound
// ============================================================================

void Enemy::PlayEnemySound(EnemySound sound, float volume, bool loop)
{
    auto it = enemySounds.find(sound);
    if (it != enemySounds.end() && it->second) {
        it->second->SetVolume(volume);
        it->second->Play(loop);
    }
}

void Enemy::StopEnemySound(EnemySound sound)
{
    auto it = enemySounds.find(sound);
    if (it != enemySounds.end() && it->second) {
        it->second->Stop();
    }
}

void Enemy::CleanupAudio()
{
    for (auto it = enemySounds.begin(); it != enemySounds.end(); ++it) {
        if (it->second) {
            it->second->Stop();
            delete it->second;
        }
    }
    enemySounds.clear();
}