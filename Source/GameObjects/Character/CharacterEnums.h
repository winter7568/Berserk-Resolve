#pragma once

// ============================================================================
// プレイヤーアニメーション
// ============================================================================

enum class PlayerAnim {
    // 移動
    IDLE_COMBAT,     // 待機

    RUN_START,       // 走り開始
    RUN_LOOP,        // 走り中
    RUN_STOP,        // 走り終了

    JUMP_START,      // ジャンプ開始
    JUMP_LOOP,       // ジャンプ中
    JUMP_STOP,       // 着地
    DOUBLE_JUMP,     // 二段ジャンプ

    DODGE_F,         // 前回避
    DODGE_B,         // 後ろ回避
    AIR_DODGE_F,     // 空中回避

    // 近接攻撃
    ATTACK_01_01,
    ATTACK_01_02,
    ATTACK_01_03,
    ATTACK_01_04,

    // 飛び道具攻撃
    PROJECTILE_ATTACK_01,
    PROJECTILE_ATTACK_02,
    PROJECTILE_ATTACK_03,
    PROJECTILE_ATTACK_04,

    // 空中攻撃
    AIR_ATTACK_01,
    AIR_ATTACK_02,
    AIR_ATTACK_03,
    AIR_ATTACK_04,

    // カウンター
    JUMP_BACK_START, // 後ろジャンプ開始
    JUMP_BACK_END,   // 後ろジャンプ終了
    COUNTER_ATTACK,  // カウンター攻撃

    // ダメージ
    HIT_AIR,         // 空中ヒット
    HIT_LAUNCH,      // 吹き飛び
    GET_UP,          // 起き上がり

    // 死亡
    DEATH,
};

// ============================================================================
// 敵アニメーション
// ============================================================================

enum class EnemyAnim {
    // 待機
    IDLE_COMBAT,

    // 前移動
    WALK_START_F,    // 歩き開始
    WALK_LOOP_F,     // 歩き中
    WALK_STOP_F,     // 歩き終了

    // 左移動
    WALK_START_F_L_90,
    WALK_LOOP_F_L_90,
    WALK_STOP_F_L_90,

    // 走り攻撃
    RUN_START_F,
    RUN_LOOP_F,
    RUN_ATTACK_01,

    // 回避
    DODGE_B,

    // ガード
    BLOCK_START,       // ガード開始
    BLOCK_LOOP,        // ガード中
    BLOCK_HIT,         // ガードヒット
    BLOCK_END,         // ガード終了
    BLOCK_HIT_BREAK,   // ガード崩れ

    // テレポート
    JUMP_START,
    JUMP_LOOP,
    JUMP_STOP,

    // 溜め攻撃
    POWER_ATTACK_START,
    POWER_ATTACK_LOOP,
    POWER_ATTACK_END,

    // コンボ攻撃
    COMBO_ATTACK_01,
    COMBO_ATTACK_02,
    COMBO_ATTACK_03,
    COMBO_ATTACK_04,

    // ダメージ
    HIT_LAUNCH,
    KNOCK_DOWN_START,
    KNOCK_DOWN_LOOP,
    KNOCK_DOWN_END,

    // バーサーク
    B_IDLE,
    B_JUMP_END,
    AIM_START,
    AIM_LOOP,
    AIM_END,
    METEOR_SLASH01,
    METEOR_SLASH02,
    METEOR_BLAST01,
    METEOR_BLAST02,

    // 死亡
    DEATH,
};

// ============================================================================
// プレイヤーサウンド
// ============================================================================

enum class PlayerSound
{
    SPAWN,          // 出現

    SWING,          // 攻撃
    COUNTER,        // カウンター

    PISTOL,         // ピストル
    STAFF,          // スタッフ
    BOW_QUICK,      // 弓速射
    BOW_LAUNCH,     // 弓溜め

    AIR_SLASH,      // 空中攻撃
    STAFF_SPIN,     // 回転攻撃
    AIR_BEAM,       // ビーム

    HIT_SLICE,      // 被ダメージ
    HIT_BLAST,      // 強ダメージ

    DEATH           // 死亡
};

// ============================================================================
// 敵サウンド
// ============================================================================

enum class EnemySound
{
    SPAWN,              // 出現

    SLASH,              // 攻撃
    MULTI_SLASH,        // 連続攻撃

    PROJECTILE,         // 飛び道具

    METEOR_SPAWN,       // メテオ生成
    METEOR_STRIKE,      // メテオ落下

    AREA_WARNING,       // 範囲警告

    TELEPORT,           // テレポート

    BERSERK_ON,         // バーサーク開始

    HIT,                // 被ダメージ
    HIT_PROJECTILE,     // 飛び道具ダメージ

    DEATH               // 死亡
};