#include <imgui.h>
#include <functional>
#include <algorithm>
#include <map>

// エンジン基本システム
#include "Player.h"
#include "GameLib/Graphics.h"
#include "GameSystems/Collision/Collision.h"


void Player::Initialize()
{
    ID3D11Device* device = Graphics::Instance().GetDevice();

    // モデル読み込み
    LoadModels(device);

    // ディゾルブエフェクト設定
    SetupDissolveEffect(device);

    // 初期位置・回転・スケール
    SetupInitialTransform();

    // アニメーション開始
    StartAnimation(GetAnimIndex(PlayerAnim::IDLE_COMBAT), true);
    model->GetNodePoses(nodePoses);

    // 武器設定
    SetupWeapon();

    // コライダー設定
    SetupColliders();

    // プレイヤーステータス設定
    SetupPlayerStats();

    // ステート管理初期化
    SetupStateManager();

    // トレイルシステム初期化
    SetupTrailSystem(device);

    // 音データ読み込み
    LoadAudio();
}

void Player::Update(float elapsedTime)
{
    // スポーンの更新（終わったらステート更新）
    if (UpdateSpawn(elapsedTime))
        stateManager.Update(elapsedTime);

    // モデルのアニメーション更新
    model->UpdateAnimation(elapsedTime);

    // アニメーションの移動を位置に反映
    RootMotion(elapsedTime);

    // マナの回復
    UpdateMana(elapsedTime);

    // アニメーション時間を進める
    AdvanceAnimationTime(elapsedTime);

    // 位置や回転を更新
    UpdateTransform();

    // 無敵時間の更新
    UpdateInvincibleTimer(elapsedTime);

    // 攻撃中ならパーティクルを出す
    UpdateStaffParticlesIfAttacking(elapsedTime);

    // 武器の位置と動きの更新
    UpdateWeaponAttachment();
    UpdateWeaponSpin(elapsedTime);
    UpdateLockOnByDistance();

    // 軌跡の更新
    staffTrail.Update(elapsedTime, staff.transform);
}

void Player::Reset()
{
    // 位置と速度のリセット
    ResetTransform();

    // 体力などのリセット
    ResetStats();

    // ステートをリセット
    ResetState();

    // 攻撃のリセット
    ResetAttackState();

    // 移動フラグのリセット
    ResetMovementFlags();

    // 武器の状態リセット
    ResetWeaponState();

    // マナのリセット
    ResetMana();

    // スポーン状態のリセット
    ResetSpawnState();

    // プレイヤーを表示
    SetVisibility(true);
}

void Player::Render(const RenderContext& rc)
{
    if (!isVisible) return;

    // シェーダー取得
    Shader* shader = Graphics::Instance().GetShader(ShaderId::PBR);

    // 描画開始
    shader->Begin(rc);
    shader->Draw(rc, model.get());
    shader->Draw(rc, staff.model.get());
    shader->End(rc);
}

// ============================================================================
// PRIVATE METHODS - Reset Helpers
// ============================================================================
void Player::ResetTransform()
{
    position = PlayerConstants::RESET_POSITION;
    velocity = PlayerConstants::RESET_VELOCITY;
    angle = PlayerConstants::RESET_ANGLE;
}

void Player::ResetStats()
{
    health = PlayerConstants::MAX_HEALTH;
    invincibleTimer = 0.0f;
    isGround = true;
    isDead = false;
}

void Player::ResetState()
{
    stateManager.ClearStates();
}

void Player::ResetAttackState()
{
    currentAttackIndex = 0;
    currentAttackStateID = PlayerStateID::NONE;
}

void Player::ResetMovementFlags()
{
    hasAirDashed = false;
    jumpCount = 0;
}

void Player::ResetWeaponState()
{
    isWeaponHitActive = false;
    isLockOnActive = false;
}

void Player::ResetSpawnState()
{
    spawnTimer = 0.0f;
    spawnCompleted = false;
    spawnEffectPlayed = false;
}
