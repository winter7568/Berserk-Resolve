#pragma once
#include <cstdint>  // 固定長整数型

// プレイヤーステートID
// uint8_tでサイズを小さくする
enum class PlayerStateID : uint8_t
{
    NONE,               // 初期状態

    IDLE,               // 待機
    RUN,                // 移動
    DASH,               // ダッシュ

    JUMP,               // ジャンプ
    FALLING,            // 落下

    ATTACK,             // 近接攻撃
    PROJECTILE_ATTACK,  // 飛び道具攻撃
    AIR_ATTACK,         // 空中攻撃

    COUNTER,            // カウンター

    REGULAR_HIT,        // 通常ダメージ
    LAUNCH_HIT,         // 吹き飛び

    STUN,               // 行動不能
    DEATH,              // 死亡

    EnumCount           // ステート数
};

// プレイヤーステートを文字列に変換
inline const char* PlayerStateToString(PlayerStateID state) {
    switch (state) {
    case PlayerStateID::NONE: return "None";
    case PlayerStateID::IDLE: return "Idle";
    case PlayerStateID::RUN: return "Run";
    case PlayerStateID::DASH: return "Dash";
    case PlayerStateID::JUMP: return "Jump";
    case PlayerStateID::FALLING: return "Falling";
    case PlayerStateID::ATTACK: return "Attack";
    case PlayerStateID::PROJECTILE_ATTACK: return "ProjectileAttack";
    case PlayerStateID::AIR_ATTACK: return "AirAttack";
    case PlayerStateID::COUNTER: return "Counter";
    case PlayerStateID::REGULAR_HIT: return "RegularHit";
    case PlayerStateID::LAUNCH_HIT: return "LaunchHit";
    case PlayerStateID::STUN: return "Stun";
    case PlayerStateID::DEATH: return "Death";
    default: return "Unknown";
    }
}

// 敵ステートID
enum class EnemyStateID : uint8_t
{
    NONE,               // 初期状態

    IDLE,               // 待機
    WALK,               // 移動
    DASH,               // ダッシュ

    POSITIONING,        // 位置調整

    ATTACK,             // 攻撃
    POWER_CHARGE,       // 溜め攻撃

    STARFE_ATTACK,      // 横移動攻撃
    COMBO_ATTACK,       // コンボ攻撃

    TELEPORT,           // テレポート
    BLOCK,              // ガード

    REGULAR_HIT,        // 通常ダメージ
    LAUNCH_HIT,         // 吹き飛び
    KNOCK_DOWN,         // ダウン

    STUN,               // 行動不能
    DIE,                // 死亡

    BERSERK,            // バーサーク

    EnumCount           // ステート数
};

// 敵ステートを文字列に変換
inline const char* EnemyStateToString(EnemyStateID state) {
    switch (state) {
    case EnemyStateID::NONE: return "None";
    case EnemyStateID::IDLE: return "Idle";
    case EnemyStateID::WALK: return "Walk";
    case EnemyStateID::DASH: return "Dash";
    case EnemyStateID::POSITIONING: return "Positioning";
    case EnemyStateID::ATTACK: return "Attack";
    case EnemyStateID::POWER_CHARGE: return "PowerCharge";
    case EnemyStateID::STARFE_ATTACK: return "StarfeAttack";
    case EnemyStateID::COMBO_ATTACK: return "ComboAttack";
    case EnemyStateID::TELEPORT: return "Teleport";
    case EnemyStateID::BLOCK: return "Block";
    case EnemyStateID::REGULAR_HIT: return "RegularHit";
    case EnemyStateID::LAUNCH_HIT: return "LaunchHit";
    case EnemyStateID::KNOCK_DOWN: return "KnockDown";
    case EnemyStateID::STUN: return "Stun";
    case EnemyStateID::DIE: return "Die";
    case EnemyStateID::BERSERK: return "Berserk";
    default: return "Unknown";
    }
}