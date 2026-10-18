#pragma once
#include <DirectXMath.h>
#include "GameObjects/Player/PlayerConstants.h"
#include "GameObjects/Enemy/EnemyConstants.h"

// ステートフレンドマクロ
// プレイヤーステートクラスをフレンドとして登録
#define FRIEND_PLAYER_STATES \
    friend class PlayerAttackState; \
    friend class PlayerIdleState; \
    friend class PlayerRunState; \
    friend class PlayerJumpState; \
    friend class PlayerDashState; \
    friend class PlayerFallingState; \
    friend class PlayerProjectileAttackState; \
    friend class PlayerAirAttackState; \
    friend class PlayerCounterState; \
    friend class PlayerHitState; \
    friend class PlayerLaunchHitState; \
    friend class PlayerRegularHitState; \
    friend class PlayerDeathState; \
    friend class PlayerStunState

// 敵ステートクラスをフレンドとして登録
#define FRIEND_ENEMY_STATES \
    friend class EnemyPositioningState; \
    friend class EnemyIdleState; \
    friend class EnemyWalkState; \
    friend class EnemyDashState; \
    friend class EnemyAttackState; \
    friend class EnemyPowerChargeState; \
    friend class EnemyStarfeAttackState; \
    friend class EnemyComboAttackState; \
    friend class EnemyTeleportState; \
    friend class BerserkState; \
    friend class EnemyBlockState; \
    friend class RegularHitState; \
    friend class LaunchHitState; \
    friend class StunState; \
    friend class KnockDownState; \
    friend class EnemyDieState

// 共通キャラクター定数（プレイヤーと敵で共有）
namespace CharacterConstants
{
    // 攻撃カウント
    constexpr int FIRST_ATTACK = 0;
    constexpr int SECOND_ATTACK = 1;
    constexpr int THIRD_ATTACK = 2;
    constexpr int FOURTH_ATTACK = 3;

    // PBRシェーダーでt0~t7まで使用済みのため、トレイル用テクスチャはt8から開始。
    // トレイル用シェーダーリソーススロット
    constexpr int TRAIL_TEXTURE_SLOT_START = 8;
    constexpr int TRAIL_TEXTURE_SLOT_COUNT = 3;
    constexpr int TRAIL_MAX_SEGMENTS = 40;

    // 物理
    constexpr float DEFAULT_GRAVITY = -60.0f;

    // 移動／方向
    constexpr float INPUT_LENGTH_EPSILON = 0.01f;
    constexpr float SNAP_THRESHOLD = 0.001f;
    constexpr float VECTOR_LENGTH_EPSILON = 0.001f;

    // 数学ヘルパー
    constexpr float HALF_SIZE = 0.5f;
    constexpr float W_COMPONENT = 1.0f;
    constexpr float AXIS_CENTER = 0.0f;
    constexpr float Z_AXIS_CENTER = 0.0f;
    constexpr int RIGHT_VECTOR_INDEX = 0;
    constexpr int UP_VECTOR_INDEX = 1;

    // アニメーション
    constexpr float DEFAULT_BLEND_TIME = 0.0f;
    constexpr float FRAME_RATE = 60.0f;

    // 角度変換
    constexpr float RADIANS_PER_DEGREE = 0.0174533f;

    // スムーズステップ
    constexpr float SMOOTH_STEP_A = 3.0f;
    constexpr float SMOOTH_STEP_B = 2.0f;

    // 本体カプセル（共通）
    constexpr float BODY_CAPSULE_RADIUS = 0.4f;
    constexpr float BODY_CAPSULE_HEIGHT = 1.02f;
    constexpr DirectX::XMFLOAT3 BODY_CAPSULE_OFFSET = { 0.0f, 0.9f, 0.0f };
    constexpr DirectX::XMFLOAT3 BODY_CAPSULE_ROTATION = { 0.0f, 0.0f, 0.0f };
    constexpr DirectX::XMFLOAT4 BODY_CAPSULE_COLOR = { 1.0f, 1.0f, 1.0f, 1.0f };


    // 死亡（共通）
    constexpr float DEATH_TIMER = 5.0f;
    constexpr float DEATH_DISSOLVE_START = 3.5f;
    constexpr float DEATH_SOUND_VOLUME = 0.8f;
    constexpr float DEATH_DISSOLVE_EDGE_INTENSITY = 4.0f;
    constexpr float DEATH_DISSOLVE_EDGE_WIDTH = 0.25f;
    constexpr float DEATH_SWORD_EDGE_INTENSITY_RATIO = 0.6f;
    constexpr float DEATH_SWORD_EDGE_WIDTH = 0.15f;
    constexpr float DEATH_GLOW_INTENSITY = 3.0f;
    constexpr float DEATH_GLOW_FADE_RATIO = 1.0f;
    constexpr float DEATH_EDGE_WIDTH = 0.2f;

    // 確率
    constexpr int PERCENTAGE_BASE = 100;

    // デバッグ色（コリジョン表示用）
    constexpr DirectX::XMFLOAT4 BODY_DEBUG_COLOR_NORMAL = { 0.0f, 1.0f, 1.0f, 1.0f };
    constexpr DirectX::XMFLOAT4 BODY_DEBUG_COLOR_HIT = { 1.0f, 0.0f, 0.0f, 1.0f };
    constexpr DirectX::XMFLOAT4 WEAPON_DEBUG_COLOR_ACTIVE = { 1.0f, 0.0f, 0.0f, 1.0f };
    constexpr DirectX::XMFLOAT4 WEAPON_DEBUG_COLOR_INACTIVE = { 0.0f, 1.0f, 0.0f, 1.0f };

    // ヒットコンボステップ
    constexpr int COMBO_STEP_FIRST = 1;
    constexpr int COMBO_STEP_SECOND = 2;
    constexpr int COMBO_STEP_THIRD = 3;
    constexpr int COMBO_STEP_FOURTH = 4;
}