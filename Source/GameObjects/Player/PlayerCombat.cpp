#include <imgui.h>

#include "Player.h"
#include "GameObjects/Enemy/Enemy.h"
#include "GameSystems/Particle/ParticleManager.h"
#include "StateMachine/PlayerStates/AttackStates/PlayerCounterState.h"
#include "GameLib/Graphics.h"
#include "GameLib/TransformUtils.h"

// ============================================================================
// PUBLIC METHODS - Damage & Combat
// ============================================================================

void Player::ApplyDamage(int damage, float invincibleTime, const HitData& hitData)
{
    // 最後に受けたダメージ情報を保存
    lastHitData = hitData;
    lastHitData.damage = damage;
    // コンボ攻撃かチェック
    bool isComboHit = (hitData.comboStep > 1);
    // 無敵中かチェック
    bool isInvincible = (invincibleTimer > 0.0f);
    // ダメージを受けるかどうか
    bool canTakeDamage = !isInvincible || isComboHit;
    // 無敵ならダメージを受けない
    if (!canTakeDamage) {
        return;
    }
    // ダメージを適用
    int healthBefore = health;
    health -= damage;
    // 体力は0以下にしない
    if (health <= 0) {
        health = 0;
    }
    // 無敵時間をセット
    invincibleTimer = invincibleTime;
}

bool Player::CanEnterCounterState()
{
    // すでにカウンター状態の場合
    if (stateManager.IsInState(PlayerStateID::COUNTER))
        return false;
    // カウンターウィンドウ内かチェック
    bool inCounterWindow = PlayerCounterState::Instance().CanCounterEnemyAttack(*this);
    if (!inCounterWindow)
        return false;
    // 向いている方向をチェック（水平面のみ）
    Enemy& enemy = Enemy::Instance();
    DirectX::XMFLOAT3 playerPos = GetPosition();
    DirectX::XMFLOAT3 enemyPos = enemy.GetPosition();
    DirectX::XMVECTOR forward = DirectX::XMLoadFloat3(&GetForwardVector());
    DirectX::XMVECTOR toEnemy = DirectX::XMVectorSubtract(
        DirectX::XMLoadFloat3(&enemyPos),
        DirectX::XMLoadFloat3(&playerPos)
    );
    toEnemy = DirectX::XMVector3Normalize(toEnemy);
    // 内積で敵が正面の円錐内にいるかチェック
    float facingDot = DirectX::XMVectorGetX(DirectX::XMVector3Dot(forward, toEnemy));
    // 敵が正面15度以内にいればtrueを返す
    return (facingDot > PlayerConstants::COUNTER_FACING_CONE);
}

bool Player::IsAttacking() const
{
    auto& stateManager = GetStateManager();
    return stateManager.IsInState(PlayerStateID::ATTACK) ||
        stateManager.IsInState(PlayerStateID::AIR_ATTACK) ||
        stateManager.IsInState(PlayerStateID::PROJECTILE_ATTACK);
}

// ============================================================================
// PUBLIC METHODS - Weapon
// ============================================================================

void Player::StartWeaponSpin(float speed)
{
    isWeaponSpinning = true;
    weaponSpinSpeed = speed;
    weaponSpinAngle = 0.0f;
}

void Player::StopWeaponSpin()
{
    isWeaponSpinning = false;
    weaponSpinSpeed = 0.0f;
    weaponSpinAngle = 0.0f;
}

void Player::UpdateWeaponSpin(float elapsedTime)
{
    if (!isWeaponSpinning)
        return;
    // 回転角度を更新（ラジアン）
    weaponSpinAngle += weaponSpinSpeed * elapsedTime;
    // 2πでラップアラウンド（一周）
    if (weaponSpinAngle > DirectX::XM_2PI)
        weaponSpinAngle -= DirectX::XM_2PI;
}

DirectX::XMFLOAT3 Player::GetWeaponTipPosition() const
{
    // 武器のワールド行列を取得
    DirectX::XMMATRIX weaponWorld = DirectX::XMLoadFloat4x4(&staff.transform);
    // 武器の先端のオフセット
    DirectX::XMVECTOR offset = DirectX::XMVectorSet(0.0f, 0.0f, weaponTipOffset, 0.0f);
    // ワールド座標に変換
    DirectX::XMVECTOR tipWorld = DirectX::XMVector3Transform(offset, weaponWorld);
    // float3に変換
    DirectX::XMFLOAT3 pos;
    DirectX::XMStoreFloat3(&pos, tipWorld);

    return pos;
}

DirectX::XMFLOAT3 Player::GetWeaponBasePosition() const
{
    // 武器のワールド行列を取得
    DirectX::XMMATRIX weaponWorld = DirectX::XMLoadFloat4x4(&staff.transform);
    // 武器の根元のオフセット
    DirectX::XMVECTOR offset = DirectX::XMVectorSet(0.0f, 0.0f, PlayerConstants::WEAPON_BASE_OFFSET, 0.0f);
    // ワールド座標に変換
    DirectX::XMVECTOR baseWorld = DirectX::XMVector3Transform(offset, weaponWorld);
    // float3に変換
    DirectX::XMFLOAT3 pos;
    DirectX::XMStoreFloat3(&pos, baseWorld);
    return pos;
}

bool Player::GetWeaponCapsule(DirectX::XMFLOAT3& outBase, DirectX::XMFLOAT3& outTip, float& outRadius)
{
    auto& weaponParams = GetWeaponCapsuleParams();

    // 武器カプセル計算用定数
    constexpr float HALF_SIZE = 0.5f;
    constexpr float W_COMPONENT = 1.0f;
    constexpr int RIGHT_VECTOR_INDEX = 0;
    constexpr int UP_VECTOR_INDEX = 1;
    constexpr int FORWARD_VECTOR_INDEX = 2;

    // 武器の変換行列を読み込み
    DirectX::XMMATRIX swordMatrix = DirectX::XMLoadFloat4x4(&staff.transform);
    // パラメータからカプセルの回転を適用
    DirectX::XMMATRIX capsuleRotation = DirectX::XMMatrixRotationRollPitchYaw(
        DirectX::XMConvertToRadians(weaponParams.rotation.x),
        DirectX::XMConvertToRadians(weaponParams.rotation.y),
        DirectX::XMConvertToRadians(weaponParams.rotation.z)
    );
    // パラメータからカプセルのオフセットを適用
    DirectX::XMMATRIX offsetMatrix = DirectX::XMMatrixTranslation(
        weaponParams.offset.x,
        weaponParams.offset.y,
        weaponParams.offset.z
    );
    // 全ての変換を合成
    DirectX::XMMATRIX final = capsuleRotation * offsetMatrix * swordMatrix;
    // 武器の前方向（Z軸）に沿ったカプセルを作成
    DirectX::XMVECTOR baseLocal = DirectX::XMVectorSet(0, 0, -weaponParams.height * HALF_SIZE, W_COMPONENT);
    DirectX::XMVECTOR tipLocal = DirectX::XMVectorSet(0, 0, weaponParams.height * HALF_SIZE, W_COMPONENT);
    // ワールド空間に変換
    DirectX::XMVECTOR baseWorld = DirectX::XMVector3TransformCoord(baseLocal, final);
    DirectX::XMVECTOR tipWorld = DirectX::XMVector3TransformCoord(tipLocal, final);
    DirectX::XMStoreFloat3(&outBase, baseWorld);
    DirectX::XMStoreFloat3(&outTip, tipWorld);

    // 各軸のスケールを取得
    DirectX::XMVECTOR rightVec = final.r[RIGHT_VECTOR_INDEX];
    DirectX::XMVECTOR upVec = final.r[UP_VECTOR_INDEX];
    DirectX::XMVECTOR forwardVec = final.r[FORWARD_VECTOR_INDEX];
    float scaleX = DirectX::XMVectorGetX(DirectX::XMVector3Length(rightVec));
    float scaleY = DirectX::XMVectorGetX(DirectX::XMVector3Length(upVec));
    float scaleZ = DirectX::XMVectorGetX(DirectX::XMVector3Length(forwardVec));
    // カプセルはZ軸に沿って伸びるので、半径はX/Y軸のスケールに基づく
    float radiusScale = max(scaleX, scaleY);
    outRadius = weaponParams.radius * radiusScale;

    return true;
}

bool Player::GetWeaponSphere(DirectX::XMFLOAT3& outCenter, float& outRadius)
{
    auto& sphereParams = GetWeaponSphereParams();
    // 武器の描画と同じ変換行列を使用
    DirectX::XMMATRIX swordMatrix = DirectX::XMLoadFloat4x4(&staff.transform);
    // スフィアのオフセットを適用（武器の先端）
    DirectX::XMVECTOR offset = DirectX::XMVectorSet(
        sphereParams.offset.x,
        sphereParams.offset.y,
        sphereParams.offset.z,
        0.0f
    );
    // ワールド空間に変換
    DirectX::XMVECTOR sphereWorld = DirectX::XMVector3Transform(offset, swordMatrix);
    DirectX::XMStoreFloat3(&outCenter, sphereWorld);
    outRadius = sphereParams.radius;

    return true;
}

void Player::UpdateWeaponAttachment()
{
    const char* rightHandName = "middle_01_r";
    
    auto& weapon = GetWeapon();
    if (!weapon.model) return;

    // 武器のローカル変換（スケール・回転・位置）
    DirectX::XMMATRIX weaponS = DirectX::XMMatrixScaling(weapon.scale.x, weapon.scale.y, weapon.scale.z);
    DirectX::XMMATRIX weaponR = DirectX::XMMatrixRotationRollPitchYaw(weapon.angle.x, weapon.angle.y, weapon.angle.z);
    DirectX::XMMATRIX weaponT = DirectX::XMMatrixTranslation(weapon.position.x, weapon.position.y, weapon.position.z);
    DirectX::XMMATRIX localOffset = weaponS * weaponR * weaponT;
    
    DirectX::XMMATRIX zUpCorrection = DirectX::XMMatrixRotationX(-DirectX::XM_PIDIV2);
    DirectX::XMMATRIX coordinateFlip = DirectX::XMMatrixScaling(-1, 1, 1);
    
    DirectX::XMMATRIX weaponLocal = zUpCorrection * localOffset;
    
    // 回転攻撃中の処理
    if (isWeaponSpinning)
    {
        // 前方向に移動
        DirectX::XMMATRIX forwardT = DirectX::XMMatrixTranslation(
            weapon.position.x,
            weapon.position.y,
            weapon.position.z + staffForwardOffset
        );
        
        // デバッグ用の回転
        DirectX::XMMATRIX straightR = DirectX::XMMatrixRotationRollPitchYaw(
            debugSpinAngleX,
            debugSpinAngleY,
            debugSpinAngleZ
        );
        
        DirectX::XMMATRIX straightLocalOffset = weaponS * straightR * forwardT;
        DirectX::XMMATRIX straightWeaponLocal = zUpCorrection * straightLocalOffset;
        
        // 回転を追加
        DirectX::XMMATRIX spinRotation = DirectX::XMMatrixRotationZ(weaponSpinAngle);
        weaponLocal = straightWeaponLocal * spinRotation;
    }
    
    // 右手のボーンを探す
    for (const Model::Node& node : model->GetNodes())
    {
        if (node.name == rightHandName)
        {
            DirectX::XMMATRIX handGlobal = DirectX::XMLoadFloat4x4(&node.globalTransform);
            DirectX::XMMATRIX playerWorld = DirectX::XMLoadFloat4x4(&transform);
            DirectX::XMMATRIX weaponWorld = weaponLocal * handGlobal * coordinateFlip * playerWorld;
            DirectX::XMStoreFloat4x4(&weapon.transform, weaponWorld);
            break;
        }
    }
    
    weapon.model->UpdateTransform(weapon.transform);
}

// ============================================================================
// PUBLIC METHODS - Mana
// ============================================================================

bool Player::TryConsumeMana(float cost)
{
    // マナが足りているかチェック
    if (currentMana >= cost && cost > 0) {
        currentMana -= cost;
        // マナが0未満にならないようにする
        if (currentMana < 0) currentMana = 0;
        return true;
    }
    return false;
}

void Player::UpdateMana(float elapsedTime)
{
    // 攻撃中かチェック
    bool isInAttackState = (
        stateManager.IsInState(PlayerStateID::PROJECTILE_ATTACK) ||
        stateManager.IsInState(PlayerStateID::AIR_ATTACK)
        );
    // 攻撃していない時だけマナ回復
    if (!isInAttackState && currentMana < maxMana) {
        currentMana += manaRegenRate * elapsedTime;
        // 最大値を超えないようにする
        if (currentMana > maxMana) currentMana = maxMana;
    }
}

// ============================================================================
// PUBLIC METHODS - Sound
// ============================================================================

void Player::PlayPlayerSound(PlayerSound sound, float volume, bool loop)
{
    auto it = sounds.find(sound);
    if (it != sounds.end() && it->second) {
        it->second->SetVolume(volume);
        it->second->Play(loop);
    }
}

void Player::StopPlayerSound(PlayerSound sound)
{
    auto it = sounds.find(sound);
    if (it != sounds.end() && it->second) {
        it->second->Stop();
    }
}

void Player::CleanupAudio()
{
    for (auto it = sounds.begin(); it != sounds.end(); ++it) {
        if (it->second) {
            it->second->Stop();
            delete it->second;
            it->second = nullptr;
        }
    }
    sounds.clear();
}

// ============================================================================
// PRIVATE METHODS - Update Helpers
// ============================================================================

void Player::UpdateStaffParticlesIfAttacking(float elapsedTime)
{
    bool renderParticle = (
        stateManager.IsInState(PlayerStateID::AIR_ATTACK) ||
        stateManager.IsInState(PlayerStateID::PROJECTILE_ATTACK) ||
        stateManager.IsInState(PlayerStateID::COUNTER) ||
        stateManager.IsInState(PlayerStateID::ATTACK)
        );
    if (renderParticle)
        UpdateStaffParticles(elapsedTime);
}

// ============================================================================
// PRIVATE METHODS - Particle Helpers
// ============================================================================

void Player::UpdateStaffParticles(float elapsedTime) const
{
    // 武器の先端と根本の位置を取得
    DirectX::XMFLOAT3 tip = GetWeaponTipPosition();
    DirectX::XMFLOAT3 base = GetWeaponBasePosition();

    // 武器の前方向を計算
    DirectX::XMFLOAT3 dir = {
        tip.x - base.x,
        tip.y - base.y,
        tip.z - base.z
    };
    DirectX::XMVECTOR d = DirectX::XMLoadFloat3(&dir);
    d = DirectX::XMVector3Normalize(d);
    DirectX::XMStoreFloat3(&dir, d);

    // ParticleManager経由で呼び出し
    ParticleManager::Instance().SpawnStaffAuraParticles(tip, base, dir, elapsedTime);
}