#include <algorithm>
#include "HitManager.h"
#include "StateMachine/EnemyStates/HitStates.h"
#include "GameSystems/Collision/CollisionManager.h"
#include "GameSystems/Debug/DebugRenderer.h"
#include "GameSystems/Effect/EffectManager.h"
#include "CameraFiles/CameraShake.h"

// ============================================================
// プレイヤーの武器ヒット処理
// ============================================================

bool HitManager::ProcessWeaponHit(Player& player, Enemy& enemy) {
    // 敵が死んでいる場合は処理しない
    if (enemy.GetHealth() <= 0) return false;

    // 敵が表示されていない場合は処理しない
    if (!enemy.IsVisible()) return false;

    // 打ち上げ状態中はヒットさせない
    if (enemy.GetStateManager().IsInState(EnemyStateID::LAUNCH_HIT)) return false;

    // 武器の当たり判定が有効でない場合は処理しない
    if (!player.IsWeaponHitActive()) return false;

    // 武器と敵の当たり判定チェック
    auto& collisionManager = CollisionManager::Instance();
    if (!collisionManager.CheckWeaponEnemyCollision(player, enemy)) return false;

    // 武器の球情報を取得
    DirectX::XMFLOAT3 sphereCenter;
    float sphereRadius;
    if (!player.GetWeaponSphere(sphereCenter, sphereRadius)) return false;

    // ヒット位置と敵位置
    DirectX::XMFLOAT3 hitPoint = sphereCenter;
    DirectX::XMFLOAT3 enemyPos = enemy.GetPosition();
    DirectX::XMFLOAT3 hitDirection = CalculateDirection(hitPoint, enemyPos);

    // 攻撃強さの初期値
    float attackStrength = HitConstants::Player::DEFAULT_ATTACK_STRENGTH;

    // プレイヤーの現在ステート取得
    PlayerStateID playerStateID = player.GetStateManager().GetCurrentStateID();

    // 攻撃系でない場合はヒットしない
    if (playerStateID != PlayerStateID::ATTACK && playerStateID != PlayerStateID::COUNTER) {
        return false;
    }

    // コンボ段階取得
    int currentAttack = player.GetCurrentAttackIndex();

    // 強攻撃か軽攻撃か判定
    if (currentAttack >= HitConstants::Player::SECOND_ATTACK_INDEX) {
        attackStrength = HitConstants::Player::HEAVY_ATTACK_STRENGTH;
    }
    else {
        attackStrength = HitConstants::Player::LIGHT_ATTACK_STRENGTH;
    }

    // ガード判定
    if (CheckEnemyBlock(enemy, hitDirection, attackStrength)) {
        // ガードエフェクト
        EffectManager::Instance().PlayEffect("BlockEffect", enemy.GetLeftHandPosition(), HitConstants::EFFECT_SCALE);

        // ガード時のヒット結果
        HitResult blockResult;
        blockResult.hitData.hitType = Character::HitType::Regular;
        blockResult.hitData.hitForce = attackStrength * HitConstants::Player::BLOCK_HIT_FORCE_MULTIPLIER;
        blockResult.hitData.damage = 0;

        // ヒットストップとカメラシェイク
        ApplyHitStop(blockResult, false, HitConstants::BLOCK_HIT_STOP_MULTIPLIER);
        CameraShake::Instance().AddTrauma(HitConstants::Player::BLOCK_TRAUMA);
        return true;
    }

    // ヒットデータ作成
    Character::HitData hitData = CreatePlayerHitData(playerStateID, currentAttack, hitPoint, hitDirection);

    // ヒット結果作成
    HitResult result;
    result.hitConnected = true;
    result.hitPoint = hitPoint;
    result.attackType = PlayerStateToString(playerStateID);
    result.hitData = hitData;

    // ヒット登録
    RegisterHit(result);

    // 特定状態中は強制ヒット処理
    if (ShouldForceHit(enemy, player)) {
        // ヒットストップとダメージ適用
        ApplyHitStop(result, false, HitConstants::HIT_STOP_MULTIPLIER_NORMAL);
        enemy.StopEnemySound(EnemySound::HIT);
        enemy.PlayEnemySound(EnemySound::HIT, HitConstants::HIT_SOUND_VOLUME);
        CameraShake::Instance().AddTrauma(HitConstants::Player::POWER_CHARGE_TRAUMA);
        ApplyForceHitDamageAndKnockdown(enemy, result);
    }
    else {
        // 通常ヒットリアクション
        ApplyHitReaction(enemy, result);
        ApplyHitStop(result, false, HitConstants::HIT_STOP_MULTIPLIER_NORMAL);
    }

    // ヒット音再生
    enemy.StopEnemySound(EnemySound::HIT);
    enemy.PlayEnemySound(EnemySound::HIT, HitConstants::HIT_SOUND_VOLUME);

    // エフェクト生成
    SpawnHitEffects(result);
    return true;
}

// ============================================================
// プレイヤーの飛び道具ヒット処理
// ============================================================

bool HitManager::ProcessProjectileHit(Projectile& projectile, Enemy& enemy) {
    // 敵が死んでいる場合は処理しない
    if (enemy.GetHealth() <= 0) return false;

    // 敵が表示されていない場合は処理しない
    if (!enemy.IsVisible()) return false;

    // 特定状態中はヒットさせない
    if (enemy.GetStateManager().IsInState(EnemyStateID::LAUNCH_HIT) ||
        enemy.GetStateManager().IsInState(EnemyStateID::POWER_CHARGE)) {
        return false;
    }

    // 位置取得
    DirectX::XMFLOAT3 projectilePos = projectile.GetPosition();
    DirectX::XMFLOAT3 enemyPos = enemy.GetPosition();

    // ヒット方向を計算
    DirectX::XMFLOAT3 hitDirection = CalculateDirection(projectilePos, enemyPos);

    // ヒットデータ作成
    Character::HitData hitData;
    hitData.hitOrigin = projectilePos;
    hitData.hitDirection = hitDirection;
    hitData.hitType = Character::HitType::Regular;
    hitData.hitForce = projectile.GetImpactForce();
    hitData.damage = projectile.GetDamage();
    hitData.comboStep = HitConstants::PROJECTILE_COMBO_STEP;

    // ヒット結果作成
    HitResult result;
    result.hitConnected = true;
    result.hitPoint = projectilePos;
    result.attackType = "Projectile_" + projectile.GetProjectileType();
    result.hitData = hitData;

    // ヒット登録
    RegisterHit(result);

    // 特定状態中の処理
    if (ShouldEnemyForceHit(enemy, hitData.hitForce)) {
        // ヒットストップとダメージ
        ApplyHitStop(result, false, HitConstants::PROJECTILE_HIT_STOP_MULTIPLIER);
        enemy.StopEnemySound(EnemySound::HIT_PROJECTILE);
        enemy.PlayEnemySound(EnemySound::HIT_PROJECTILE, HitConstants::HIT_SOUND_VOLUME);
        CameraShake::Instance().AddTrauma(HitConstants::PROJECTILE_TRAUMA);
        ApplyForceHitDamageAndKnockdown(enemy, result);
    }
    else {
        // 通常のヒットリアクション
        ApplyProjectileHitReaction(enemy, result, projectile);
    }

    // ヒット音
    enemy.StopEnemySound(EnemySound::HIT_PROJECTILE);
    enemy.PlayEnemySound(EnemySound::HIT_PROJECTILE, HitConstants::HIT_SOUND_VOLUME);

    // エフェクト生成
    SpawnProjectileHitEffects(result, projectile);
    return true;
}

// ============================================================
// ヒット登録
// ============================================================

void HitManager::RegisterHit(const HitResult& result) {
    // ヒットしている場合のみ処理
    if (result.hitConnected) {
        // ヒット履歴に追加
        recentHits.push_back(result);

        // 履歴が上限を超えたら古いものを削除
        if (recentHits.size() > HitConstants::MAX_HIT_HISTORY) {
            recentHits.erase(recentHits.begin());
        }

        // コンボ数を増やす
        currentCombo++;

        // コンボリセットタイマーを更新
        comboResetTimer = comboResetTime;
    }
}

// ============================================================
// エネミーの武器ヒット処理
// ============================================================

bool HitManager::ProcessEnemyWeaponHit(Enemy& enemy, Player& player) {
    // プレイヤーが死亡している場合は無効
    if (player.GetHealth() <= 0) return false;

    // カウンター中はヒットしない
    if (player.GetStateManager().IsInState(PlayerStateID::COUNTER)) return false;

    // 敵の攻撃判定が無効なら終了
    if (!enemy.IsWeaponHitActive()) return false;

    // 武器のカプセル情報を取得
    DirectX::XMFLOAT3 weaponBase, weaponTip;
    float weaponRadius;
    if (!enemy.GetWeaponCapsule(weaponBase, weaponTip, weaponRadius)) return false;

    // プレイヤーの当たり判定を取得
    DirectX::XMFLOAT3 playerBase, playerTip;
    player.GetBodyCapsuleEndpoints(playerBase, playerTip);
    float playerRadius = player.GetScaledBodyRadius();

    // カプセル同士の当たり判定
    DirectX::XMFLOAT3 outPosition;
    bool collision = Collision::IntersectCapsuleVsCapsule(
        weaponBase, weaponTip, weaponRadius,
        playerBase, playerTip, playerRadius,
        outPosition
    );

    // 当たっていなければ終了
    if (!collision) return false;

    // プレイヤー位置を取得
    DirectX::XMFLOAT3 playerPos = player.GetPosition();

    // ヒット方向を計算
    DirectX::XMFLOAT3 hitDirection = CalculateDirection(outPosition, playerPos);

    // 敵の状態と攻撃情報を取得
    EnemyStateID enemyStateID = enemy.GetStateManager().GetCurrentStateID();
    int currentAttack = enemy.GetCurrentAttackIndex();
    int currentSlash = enemy.GetCurrentSlashIndex();

    // ヒットデータ作成
    Character::HitData hitData = CreateEnemyHitData(enemyStateID, currentAttack, currentSlash, outPosition, hitDirection);

    // 結果データ作成
    HitResult result;
    result.hitConnected = true;
    result.hitPoint = outPosition;
    result.attackType = "EnemyWeapon";
    result.hitData = hitData;

    // サウンド再生
    player.StopPlayerSound(PlayerSound::HIT_SLICE);
    player.PlayPlayerSound(PlayerSound::HIT_SLICE, HitConstants::HIT_SOUND_VOLUME);

    // ヒットストップ適用
    ApplyHitStop(result, true, HitConstants::ENEMY_WEAPON_HIT_STOP_MULTIPLIER);

    // プレイヤーへのリアクション適用
    ApplyPlayerHitReaction(player, result);

    // ヒットエフェクト生成
    EffectManager::Instance().PlayEffect("enemySlashEffect", result.hitPoint, HitConstants::EFFECT_SCALE);
    return true;
}

// ============================================================
// エネミーの飛び道具ヒット処理
// ============================================================

bool HitManager::ProcessEnemyProjectileHit(Projectile& projectile, Player& player) {
    // プレイヤーが死亡している場合は無効
    if (player.GetHealth() <= 0) return false;

    // 位置を取得
    DirectX::XMFLOAT3 projectilePos = projectile.GetPosition();
    DirectX::XMFLOAT3 playerPos = player.GetPosition();

    // ヒット方向を計算
    DirectX::XMFLOAT3 hitDirection = CalculateDirection(playerPos, projectilePos);

    // ヒットデータ作成
    Character::HitData hitData;
    hitData.hitOrigin = projectilePos;
    hitData.hitDirection = hitDirection;
    hitData.hitType = Character::HitType::Regular;
    hitData.hitForce = projectile.GetImpactForce();
    hitData.damage = projectile.GetDamage();
    hitData.comboStep = HitConstants::PROJECTILE_COMBO_STEP;

    // 衝撃力によってヒットタイプを変更
    if (projectile.GetImpactForce() > HitConstants::Enemy::PROJECTILE_LAUNCH_THRESHOLD) {
        hitData.hitType = Character::HitType::Launch;
    }
    else if (projectile.GetImpactForce() > HitConstants::Enemy::PROJECTILE_STUN_THRESHOLD) {
        hitData.hitType = Character::HitType::Stun;
    }

    // 結果データ作成
    HitResult result;
    result.hitConnected = true;
    result.hitPoint = projectilePos;
    result.attackType = "EnemyProjectile_" + projectile.GetProjectileType();
    result.hitData = hitData;

    // ヒットストップ適用
    ApplyHitStop(result, true, HitConstants::ENEMY_PROJECTILE_HIT_STOP_MULTIPLIER);

    // サウンド再生
    player.PlayPlayerSound(PlayerSound::HIT_BLAST, HitConstants::HIT_SOUND_VOLUME);

    // ヒット登録
    RegisterHit(result);

    // プレイヤーリアクション適用
    ApplyEnemyProjectileHitReaction(player, result, projectile);

    // エフェクト生成
    SpawnProjectileHitEffects(result, projectile);
    return true;
}