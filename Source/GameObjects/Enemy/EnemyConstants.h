#pragma once
#include <DirectXMath.h>

// 敵定数
namespace EnemyConstants
{
    // 基本ステータス
    constexpr int MAX_HEALTH = 1000;
    constexpr float DEFAULT_WEIGHT = 0.7f;
    constexpr float INVINCIBLE_TIME_DEFAULT = 0.8f;

    // 体力しきい値
    constexpr float HEALTH_CRITICAL = 0.35f;
    constexpr float HEALTH_LOW = 0.50f;
    constexpr float HEALTH_MEDIUM = 0.75f;

    // トランスフォーム
    constexpr DirectX::XMFLOAT3 INITIAL_POSITION = { 0.0f, 0.4f, 0.2f };
    constexpr DirectX::XMFLOAT3 INITIAL_SCALE = { 7.8f, 7.8f, 7.8f };
    constexpr DirectX::XMFLOAT3 INITIAL_ANGLE = { 0.0f, 80.0f, 0.0f };
    constexpr DirectX::XMFLOAT3 INITIAL_VELOCITY = { 0.0f, 0.0f, 0.0f };
    constexpr DirectX::XMFLOAT3 RESET_ANGLE = { 0.0f, 0.0f, 0.0f };

    // 回転速度
    constexpr float TURN_SPEED_NORMAL = 8.0f;
    constexpr float TURN_SPEED_FAST = 12.0f;

    // ダッシュ
    constexpr float DASH_DURATION = 0.4f;
    constexpr float DASH_MAX_DISTANCE = 40.0f;
    constexpr float DASH_MIN_MULTIPLIER = 0.5f;
    constexpr float DASH_MAX_MULTIPLIER = 1.0f;
    constexpr float DASH_MULTIPLIER_RANGE = DASH_MAX_MULTIPLIER - DASH_MIN_MULTIPLIER;

    // ルートモーション倍率
    constexpr float DASH_DISTANCE_SCALE = 2.0f;
    constexpr float COMBO_DISTANCE_SCALE = 0.5f;
    constexpr float ATTACK_RUN_SCALE = 1.4f;

    // 距離範囲
    constexpr float ATTACK_RANGE = 40.0f;
    constexpr float WALK_START_DISTANCE = 45.5f;
    constexpr float WALK_STOP_DISTANCE = 50.0f;
    constexpr float BACK_DASH_DISTANCE = 15.0f;

    // ブロック
    constexpr float BLOCK_MAX_DISTANCE = 15.0f;
    constexpr float BLOCK_MIN_MULTIPLIER = 0.7f;
    constexpr float BLOCK_MAX_MULTIPLIER = 1.0f;
    constexpr float BLOCK_MULTIPLIER_RANGE = BLOCK_MAX_MULTIPLIER - BLOCK_MIN_MULTIPLIER;
    constexpr float BLOCK_MAX_STAMINA = 100.0f;
    constexpr float BLOCK_STAMINA_DRAIN_LIGHT = 15.0f;
    constexpr float BLOCK_STAMINA_DRAIN_HEAVY = 35.0f;
    constexpr float BLOCK_STAMINA_REGEN_RATE = 5.0f;
    constexpr float BLOCK_BREAK_THRESHOLD = 60.0f;
    constexpr float BLOCK_START_DURATION = 0.4f;
    constexpr float BLOCK_HIT_DURATION = 0.3f;
    constexpr float BLOCK_RECOVERY_DURATION = 0.76f;
    constexpr float BLOCK_BREAK_DURATION = 1.0f;
    constexpr float BLOCK_HOLD_MAX_TIME = 2.7f;
    constexpr float BLOCK_NO_ATTACK_TIMEOUT = 1.5f;
    constexpr float BLOCK_COOLDOWN_MIN = 2.0f;
    constexpr float BLOCK_COOLDOWN_MAX = 4.0f;
    constexpr float MAX_BLOCK_DISTANCE = 15.0f;
    constexpr float MIN_BLOCK_DISTANCE = 15.0f;

    // 待機
    constexpr float MAX_IDLE_TIME = 0.3f;

    // 向き判定
    constexpr float FACING_DOT_THRESHOLD = 0.5f;
    constexpr float DIRECTION_DOT_FRONT = 0.5f;
    constexpr float DIRECTION_DOT_BACK = -0.5f;

    // 剣の姿勢
    constexpr DirectX::XMFLOAT3 DEFAULT_SWORD_POS = { -0.043f, -0.011f, 0.009f };
    constexpr DirectX::XMFLOAT3 DEFAULT_SWORD_ROT = { 0.0279f, -0.8203f, 0.0175f };
    constexpr DirectX::XMFLOAT3 DEFAULT_SWORD_SCALE = { 0.8f, 0.8f, 0.8f };

    constexpr DirectX::XMFLOAT3 BERSERK_SWORD_POS = { -0.030f, 0.0f, 0.030f };
    constexpr DirectX::XMFLOAT3 BERSERK_SWORD_ROT = { 0.031f, -0.805f, 0.484f };
    constexpr DirectX::XMFLOAT3 BERSERK_SWORD_SCALE = { 0.8f, 0.8f, 0.8f };

    // 武器カプセル
    constexpr float WEAPON_CAPSULE_RADIUS = 0.1f;
    constexpr float WEAPON_CAPSULE_HEIGHT = 1.5f;
    constexpr DirectX::XMFLOAT3 WEAPON_CAPSULE_OFFSET = { 0.008f, -0.013f, 0.760f };
    constexpr DirectX::XMFLOAT3 WEAPON_CAPSULE_ROTATION = { 0.1f, 0.1f, 0.0f };

    // リセット値
    constexpr DirectX::XMFLOAT3 RESET_SWORD_POS = { -0.120f, 0.300f, -0.060f };
    constexpr DirectX::XMFLOAT3 RESET_SWORD_ROT = { 0.0f, 3.8f, 0.0f };
    constexpr DirectX::XMFLOAT3 RESET_SWORD_SCALE = { 1.0f, 1.0f, 1.0f };

    constexpr DirectX::XMFLOAT3 RESET_CAPSULE_OFFSET = { 0.0f, 0.130f, 0.257f };
    constexpr DirectX::XMFLOAT3 RESET_CAPSULE_ROTATION = { -2.0f, 1.0f, 0.0f };
    constexpr float RESET_CAPSULE_RADIUS = 0.1f;
    constexpr float RESET_CAPSULE_HEIGHT = 1.0f;
    constexpr DirectX::XMFLOAT4 RESET_CAPSULE_COLOR = { 1.0f, 0.0f, 0.0f, 1.0f };

    // デバッグUI
    constexpr float DEBUG_UI_DRAG_STEP = 0.01f;
    constexpr float DEBUG_UI_MIN_RANGE = -2.0f;
    constexpr float DEBUG_UI_MAX_RANGE = 2.0f;

    // 発光
    constexpr float DEFAULT_EMISSIVE_STRENGTH = 40.0f;
    constexpr float SWORD_EMISSIVE_STRENGTH = 2.1f;
    constexpr DirectX::XMFLOAT3 DEFAULT_EMISSIVE_COLOR = { 1.0f, 0.6f, 0.2f };
    constexpr DirectX::XMFLOAT3 FLASH_COLOR = { 1.0f, 0.0f, 0.0f };

    // トレイル
    constexpr float TRAIL_MAX_LIFETIME = 0.5f;
    constexpr DirectX::XMFLOAT3 TRAIL_TIP_OFFSET = { 0.040f, -0.005f, 1.670f };
    constexpr DirectX::XMFLOAT3 TRAIL_BASE_OFFSET = { 0.0f, 0.0f, 0.2900f };
    constexpr DirectX::XMFLOAT3 TRAIL_RESET_TIP = { 0.0f, 0.0f, 1.0f };
    constexpr DirectX::XMFLOAT3 TRAIL_RESET_BASE = { 0.0f, 0.0f, -0.5f };

    // 左手オフセット
    constexpr float LEFT_HAND_OFFSET_X = 0.0f;
    constexpr float LEFT_HAND_OFFSET_Y = 0.0f;
    constexpr float LEFT_HAND_OFFSET_Z = 0.0f;

    // シネマティック出現
    constexpr float SPAWN_START_DELAY = 1.0f;
    constexpr float SPAWN_DURATION = 3.0f;
    constexpr float SPAWN_GLOW_RAMP_UP_END = 0.3f;
    constexpr float SPAWN_GLOW_SUSTAIN_END = 0.7f;
    constexpr float SPAWN_PULSE_SPEED = 10.0f;
    constexpr float SPAWN_PULSE_STRENGTH = 0.3f;
    constexpr float SPAWN_EDGE_WIDTH = 0.2f;
    constexpr float SPAWN_BASE_INTENSITY = 3.0f;
    constexpr float SPAWN_SWORD_INTENSITY_RATIO = 0.5f;
    constexpr float SPAWN_SWORD_EDGE_WIDTH = 0.1f;

    // パワーチャージエフェクト
    constexpr float BERSERK_EFFECT_SCALE = 9.0f;
    constexpr float NORMAL_EFFECT_SCALE = 6.0f;
    constexpr float WARNING_VOLUME = 0.6f;

    // サウンド音量
    constexpr float DEFAULT_SOUND_VOLUME = 0.4f;
    constexpr float TELEPORT_SOUND_VOLUME = 0.5f;
    constexpr float ATTACK_COMBO_SLASH_VOLUME = 0.5f;
    constexpr float POWER_CHARGE_MULTI_SLASH_VOLUME = 0.4f;

    // カメラシェイク
    constexpr float REGULAR_HIT_TRAUMA = 0.8f;
    constexpr float LAUNCH_HIT_TRAUMA = 1.0f;
    constexpr float STUN_HIT_TRAUMA = 1.2f;
    constexpr float KNOCKDOWN_TRAUMA = 2.5f;
    constexpr float BERSERK_CAMERA_SHAKE_TRAUMA = 1.0f;

    // ヒット状態
    constexpr float REGULAR_HIT_ANIM_DURATION = 0.817f;
    constexpr float STUN_DURATION_DEFAULT = 1.0f;
    constexpr float STUN_DURATION_AFTER_THIRD_HIT = 1.2f;

    constexpr float LAUNCH_PHASE_DURATION = 1.2f;
    constexpr float HIT_TELEPORT_DISTANCE = 100.0f;
    constexpr float HIT_TELEPORT_Y_OFFSET = 8.0f;
    constexpr float HIT_TELEPORT_INVINCIBLE_TIME = 0.8f;

    // ノックダウン
    constexpr float KNOCKDOWN_START_DURATION = 1.0f;
    constexpr float KNOCKDOWN_LOOP_DURATION = 1.26f;
    constexpr float KNOCKDOWN_END_DURATION = 1.46f;

    // 攻撃フレーム
    constexpr int ATTACK_RUN_LOOP_LIMIT = 3;
    constexpr int ATTACK_HIT_START_FRAME = 15;
    constexpr int ATTACK_HIT_END_FRAME = 22;
    constexpr int ATTACK_COUNTER_START_FRAME = 0;
    constexpr int ATTACK_COUNTER_END_FRAME = 24;
    constexpr float ATTACK_COUNTER_RANGE = 25.0f;
    constexpr float ATTACK_COUNTER_EFFECT_SCALE = 10.0f;
    constexpr float ATTACK_WARNING_AREA_Y = 0.3f;

    // パワーチャージ
    constexpr float POWER_CHARGE_HOLD_DURATION = 2.0f;
    constexpr float POWER_CHARGE_START_Y_OFFSET = -4.0f;
    constexpr float POWER_CHARGE_TIME_SLOW_FACTOR = 0.6f;
    constexpr int POWER_CHARGE_TOTAL_SLASHES = 6;

    // 斬撃フレーム
    constexpr int SLASH_1_START = 0;
    constexpr int SLASH_1_END = 6;
    constexpr int SLASH_2_START = 8;
    constexpr int SLASH_2_END = 14;
    constexpr int SLASH_3_START = 15;
    constexpr int SLASH_3_END = 24;
    constexpr int SLASH_4_START = 25;
    constexpr int SLASH_4_END = 34;
    constexpr int SLASH_5_START = 35;
    constexpr int SLASH_5_END = 44;
    constexpr int SLASH_6_START = 45;
    constexpr int SLASH_6_END = 65;

    // スターフェ攻撃
    constexpr int STARFE_TOTAL_PROJECTILES = 3;
    constexpr float STARFE_PROJECTILE_DELAY = 1.2f;
    constexpr float STARFE_SMOKE_DURATION = 1.0f;
    constexpr float STARFE_SMOKE_SCALE = 1.0f;
    constexpr float STARFE_SPAWN_HEIGHT = 12.0f;
    constexpr float STARFE_TARGET_HEIGHT_OFFSET = 8.0f;

    // コンボ攻撃
    constexpr float COMBO_TIME_SCALE = 0.9f;
    constexpr float COMBO_EMISSIVE_FLASH_DURATION = 3.5f;
    constexpr float COMBO_EMISSIVE_RED = 5.0f;
    constexpr float COMBO_PULSE_SPEED = 10.0f;
    constexpr int COMBO_COUNTER_START_FRAME = 0;
    constexpr int COMBO_COUNTER_FRAMES = 24;
    constexpr int COMBO_ATTACK_3_FIRST_HIT_START = 10;
    constexpr int COMBO_ATTACK_3_FIRST_HIT_END = 20;
    constexpr int COMBO_ATTACK_3_SECOND_HIT_START = 40;
    constexpr int COMBO_ATTACK_3_SECOND_HIT_END = 50;
    constexpr int COMBO_ATTACK_3_RESET_FRAME = 50;

    // バーサーク
    constexpr int BERSERK_ATTACK_COUNT = 2;
    constexpr float BERSERK_DURATION = 10.0f;
    constexpr float BERSERK_SCALE = 10.8f;
    constexpr float BERSERK_WEIGHT = 2.0f;
    constexpr float BERSERK_TELEPORT_Y_OFFSET = 10.0f;
    constexpr float BERSERK_TIME_SCALE = 0.3f;
    constexpr float BERSERK_TIME_SCALE_DURATION = 0.5f;
    constexpr float BERSERK_START_Y_POSITION = 0.4f;
    constexpr float BERSERK_IDLE_TO_ATTACK_TIME = 1.0f;
    constexpr float BERSERK_ATTACK_COOLDOWN = 1.5f;
    constexpr float BERSERK_AIM_LOOP_DURATION = 1.0f;
    constexpr float BERSERK_PROJECTILE_DELAY = 0.35f;
    constexpr float BERSERK_METEOR_IMPACT_DELAY = 1.4f;
    constexpr float BERSERK_HAND_FORWARD_OFFSET = 0.7f;
    constexpr float BERSERK_PULSE_SPEED = 8.0f;
    constexpr float BERSERK_WARNING_AREA_Y = 0.6f;
    constexpr float BERSERK_WARNING_AREA_SCALE = 12.0f;
    constexpr float BERSERK_LIGHTNING_SPAWN_PERCENT = 0.45f;
    constexpr float BERSERK_METEOR_SPAWN_PERCENT = 0.05f;
    constexpr float BERSERK_BLAST_SPAWN_PERCENT = 0.01f;
    constexpr float BERSERK_METEOR_SPAWN_VOLUME = 0.4f;
    constexpr float BERSERK_METEOR_STRIKE_VOLUME = 0.6f;
    constexpr float BERSERK_SPAWN_SOUND_VOLUME = 0.5f;
    constexpr float BERSERK_BERSERK_ON_VOLUME = 0.3f;
    constexpr float BERSERK_DANGER_ZONE_WARNING_VOLUME = 0.6f;
    constexpr float BERSERK_DANGER_ZONE_HIT_VOLUME = 0.6f;
    constexpr float BERSERK_METEOR_EFFECT_SCALE = 1.5f;
    constexpr float BERSERK_BLAST_EFFECT_SCALE = 2.0f;
    constexpr float BERSERK_SPAWN_EFFECT_SCALE = 1.0f;
    constexpr float BERSERK_AREA_METEOR_SCALE = 2.0f;
    constexpr float BERSERK_WEAPON_RADIUS_MULTIPLIER = 3.0f;
    constexpr float BERSERK_WEAPON_HEIGHT_MULTIPLIER = 1.5f;
    constexpr float BERSERK_FRAME_RATE = 60.0f;
    constexpr float BERSERK_SLASH_FRAME = 75.0f;
    constexpr float BERSERK_BLAST_FRAME = 40.0f;
    constexpr float BERSERK_DANGER_ZONE_FRAMES = 100.0f;
    // バーサク発光色
    static constexpr DirectX::XMFLOAT3 BERSERK_EMISSIVE_COLOR = { 5.0f, 0.2f, 0.2f };
    // バーサクパルス色（基本値と変動幅）
    static constexpr float BERSERK_PULSE_R_BASE = 3.0f;
    static constexpr float BERSERK_PULSE_R_VARY = 2.0f;
    static constexpr float BERSERK_PULSE_G_BASE = 0.1f;
    static constexpr float BERSERK_PULSE_G_VARY = 0.1f;
    static constexpr float BERSERK_PULSE_B_BASE = 0.1f;
    static constexpr float BERSERK_PULSE_B_VARY = 0.1f;

    // 危険エリア
    constexpr float DANGER_ZONE_CENTER_X = -5.5f;
    constexpr float DANGER_ZONE_CENTER_Y = 0.0f;
    constexpr float DANGER_ZONE_CENTER_Z = 0.5f;
    constexpr float DANGER_ZONE_SIZE_X = 40.0f;
    constexpr float DANGER_ZONE_SIZE_Y = 35.0f;
    constexpr float DANGER_ZONE_SIZE_Z = 40.0f;

    // テレポート
    constexpr float TELEPORT_Y_OFFSET = 10.0f;
    constexpr float TELEPORT_TIME_SCALE = 1.0f;
    constexpr float TELEPORT_TIME_SCALE_DURATION = 8.0f;
    constexpr float TELEPORT_TIME_RESET_DURATION = 3.0f;
    constexpr float TELEPORT_CLOSE_RANGE_MIN = 8.0f;
    constexpr float TELEPORT_CLOSE_RANGE_MAX = 10.0f;
    constexpr float TELEPORT_FAR_RANGE_MIN = 90.0f;
    constexpr float TELEPORT_FAR_RANGE_MAX = 105.0f;
    constexpr float TELEPORT_COMBO_ATTACK_DISTANCE = 15.0f;
    constexpr int TELEPORT_BEHAVIOR_COUNT = 2;
    constexpr int TELEPORT_ANGLE_FULL_CIRCLE = 360;
    constexpr int TELEPORT_RANGED_BEHAVIOR_CHANCE = 50;

    // 攻撃カウント（敵固有）
    constexpr int ATTACK_PART_ONE = 1;
    constexpr int ATTACK_PART_TWO = 2;
    constexpr int COMBO_STEP_THIRD_ATTACK = 3;

    // AIポジショニング
    constexpr int POSITIONING_DECISION_HISTORY_MAX = 20;
    constexpr int POSITIONING_SPAM_THRESHOLD = 2;
    constexpr float POSITIONING_SURPRISE_ATTACK_DISTANCE = 25.0f;
    constexpr float POSITIONING_BASE_ATTACK_RANGE = 80.0f;
    constexpr float POSITIONING_BASE_WALK_RANGE = 25.5f;
    constexpr float POSITIONING_SPAM_DETECTION_WINDOW = 2.0f;
    constexpr float POSITIONING_EVALUATION_INTERVAL = 0.1f;
    constexpr float POSITIONING_TELEPORT_COOLDOWN = 0.3f;
    constexpr float POSITIONING_FACING_DOT_THRESHOLD = 0.5f;
    constexpr float POSITIONING_OPTIMAL_DISTANCE = 25.0f;
    constexpr float POSITIONING_STOP_DISTANCE = 40.5f;
    constexpr float POSITIONING_BACK_DASH_DISTANCE = 15.0f;

    // AI性格
    constexpr int POSITIONING_UNPREDICTABLE_ATTACK_CHANCE = 30;
    constexpr int POSITIONING_SPECIAL_CHOICE_COMBO_THRESHOLD = 40;
    constexpr int POSITIONING_SPECIAL_CHOICE_POWER_THRESHOLD = 70;
    constexpr int POSITIONING_BASIC_MIXUP_CHANCE = 20;
    constexpr int POSITIONING_PROJECTILE_TELEPORT_CHANCE = 50;
    constexpr int POSITIONING_RANDOM_PERCENT_MAX = 100;
    constexpr int POSITIONING_RANDOM_FLOAT_SCALE = 100;

    // AI乱数範囲
    constexpr int POSITIONING_RANDOM_RANGE_101 = 101;
    constexpr int POSITIONING_RANDOM_RANGE_81 = 81;
    constexpr int POSITIONING_RANDOM_RANGE_91 = 91;

    // 重攻撃
    constexpr float HEAVY_ATTACK_STRENGTH_THRESHOLD = 0.7f;
}