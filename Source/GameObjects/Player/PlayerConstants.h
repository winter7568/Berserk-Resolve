#pragma once
#include <DirectXMath.h>

// プレイヤー定数
namespace PlayerConstants
{
    // 基本ステータス
    constexpr int MAX_HEALTH = 500;
    constexpr float DEFAULT_WEIGHT = 0.6f;

    // マナ
    constexpr float MAX_MANA = 100.0f;
    constexpr float MANA_REGEN_RATE = 5.0f;

    // 移動
    constexpr float JUMP_POWER = 55.0f;
    constexpr float DOUBLE_JUMP_POWER = 49.5f;
    constexpr float AIR_SPEED = 6.0f;
    constexpr float AIR_CONTROL = 0.4f;

    // ダッシュ
    constexpr float DASH_MAX_HOLD_TIME = 0.5f;
    constexpr float DASH_MIN_SCALE = 1.0f;
    constexpr float DASH_MAX_SCALE = 4.0f;
    constexpr float DASH_SPEED = 15.0f;
    constexpr float AIR_DASH_SPEED = 12.0f;
    constexpr float DASH_TURN_SPEED = 3.0f;
    constexpr float DASH_ANIM_SPEED_SCALE = 0.1f;

    // 回転速度
    constexpr float TURN_SPEED_NORMAL = 7.0f;
    constexpr float TURN_SPEED_FAST = 12.0f;
    constexpr float AIR_TURN_SPEED = 5.0f;

    // 重力と空気抵抗
    constexpr float FALL_GRAVITY_MULTIPLIER = 3.8f;
    constexpr float RISE_GRAVITY_MULTIPLIER = 1.5f;
    constexpr float AIR_DRAG = 0.9f;
    constexpr float AIR_DRAG_IDLE = 0.98f;
    constexpr float FALLING_AIR_CONTROL_RATIO = 0.7f;
    constexpr float FALLING_DRAG = 0.85f;
    constexpr float FALLING_DRAG_IDLE = 0.95f;
    constexpr float FALLING_MOVE_DELTA_RATIO = 0.15f;

    // 走りアニメーションスケール
    constexpr float RUN_START_MOTION_SCALE = 1.2f;
    constexpr float RUN_LOOP_MOTION_SCALE = 2.0f;
    constexpr float RUN_STOP_MOTION_SCALE = 0.4f;
    constexpr float RUN_START_TURN_SPEED_RATIO = 0.8f;
    constexpr float RUN_START_ACCEL_FACTOR = 2.0f;

    // 近接攻撃
    constexpr float MELEE_MAX_AUTO_FACE_DISTANCE = 40.0f;
    constexpr float AUTO_FACE_TURNSPEED = 10.0f;
    constexpr float ANIMATION_PROGRESS_THRESHOLD = 0.8f;
    constexpr float MAX_BUFFER_DURATION = 0.15f;

    // 遠距離攻撃
    constexpr float STAFF_PROJECTILE_SPEED = 340.0f;
    constexpr float PROJ_MAX_AUTO_FACE_DISTANCE = 360.0f;
    constexpr float PROJECTILE_SPAWN_FORWARD_OFFSET = 2.0f;
    constexpr float PROJECTILE_ATTACK_MANA_THRESHOLD = 10.0f;

    // 空中攻撃
    constexpr float AIR_ATTACK_MANA_THRESHOLD = 25.0f;
    constexpr float BEAM_SWEET_SPOT_MIN_DISTANCE = 15.0f;
    constexpr float BEAM_MAX_LENGTH = 150.0f;
    constexpr float BEAM_FOV_DEGREES = 45.0f;
    constexpr float BEAM_HIT_INTENSITY = 15.0f;
    constexpr float BEAM_MISS_INTENSITY = 10.0f;
    constexpr float BEAM_DURATION = 0.5f;
    constexpr float BEAM_TARGET_Y_OFFSET = 8.0f;
    constexpr float STAFF_FORWARD_OFFSET_MAX = 0.15f;

    // カウンター
    constexpr float COUNTER_FACING_CONE = 0.866f;
    constexpr float COUNTER_INVINCIBLE_DURATION = 1.0f;
    constexpr float JUMP_BACK_DURATION = 0.3f;
    constexpr float COUNTER_TIME_SCALE = 0.15f;
    constexpr float COUNTER_TIME_SCALE_DURATION = 8.0f;
    constexpr float COMBO_COUNTER_PUSHBACK_MULTIPLIER = 0.5f;
    constexpr float COUNTER_SOUND_VOLUME = 0.5f;
    constexpr float COUNTER_TIME_RESET_DURATION = 3.0f;
    constexpr float COUNTER_ATTACK_TIME_RESET = 1.0f;
    constexpr float COUNTER_EXIT_TIME_RESET = 3.0f;
    constexpr float INITIAL_JUMP_BACK_VELOCITY = 80.0f;
    constexpr int JUMP_BACK_FRAMES = 45;
    constexpr int JUMP_BACK_END_FRAMES = 34;
    constexpr int COUNTER_ATTACK_FRAMES = 40;

    // ヒット状態
    constexpr float REGULAR_HIT_TRAUMA = 1.4f;
    constexpr float LAUNCH_HIT_TRAUMA = 2.4f;
    constexpr float STUN_HIT_TRAUMA = 1.8f;
    constexpr float AIR_HIT_DURATION = 0.6f;
    constexpr float REGULAR_HIT_ANIM_DURATION = 0.817f;
    constexpr float AIR_STUN_DURATION = 0.6f;
    constexpr float STUN_DURATION = 1.0f;
    constexpr float HIT_DIRECTION_SIDE_THRESHOLD = 0.5f;
    constexpr float HIT_DIRECTION_FRONT_THRESHOLD = 0.5f;
    constexpr float HIT_DIRECTION_BACK_THRESHOLD = 0.5f;
    constexpr float LAUNCH_FALL_GRAVITY_MULTIPLIER = 3.0f;
    constexpr float DEATH_FALL_GRAVITY_MULTIPLIER = 3.0f;

    // 出現
    constexpr float SPAWN_DURATION = 5.0f;
    constexpr float EFFECT_START_TIME = 4.0f;
    constexpr float DISSOLVE_SPEED = 0.3f;
    constexpr float EFFECT_Y_OFFSET = 2.0f;
    constexpr float EFFECT_SCALE = 1.0f;
    constexpr float SPAWN_SOUND_VOLUME = 0.5f;
    constexpr float SPAWN_CAMERA_SHAKE = 1.5f;
    constexpr float PULSE_FREQUENCY = 5.0f;
    constexpr float PULSE_STRENGTH = 0.5f;
    constexpr float PULSE_BASE = 1.0f;
    constexpr float EDGE_INTENSITY_BASE = 2.0f;
    constexpr float DISSOLVE_COMPLETE = 0.0f;

    // トランスフォーム
    constexpr DirectX::XMFLOAT3 INITIAL_POSITION = { -500.0f, 0.0f, -5.0f };
    constexpr DirectX::XMFLOAT3 INITIAL_SCALE = { 7.0f, 7.0f, 7.0f };
    constexpr DirectX::XMFLOAT3 INITIAL_ANGLE = { 0.0f, -80.0f, 0.0f };
    constexpr DirectX::XMFLOAT3 INITIAL_VELOCITY = { 0.0f, 0.0f, 0.0f };
    constexpr DirectX::XMFLOAT3 RESET_POSITION = { -500.0f, 0.0f, -5.0f };
    constexpr DirectX::XMFLOAT3 RESET_VELOCITY = { 0.0f, 0.0f, 0.0f };
    constexpr DirectX::XMFLOAT3 RESET_ANGLE = { 0.0f, -80.0f, 0.0f };

    // 杖（武器）
    constexpr DirectX::XMFLOAT3 STAFF_SCALE = { 1.2f, 1.2f, 1.2f };
    constexpr DirectX::XMFLOAT3 STAFF_POSITION = { -0.070f, 0.130f, -0.020f };
    constexpr DirectX::XMFLOAT3 STAFF_ANGLE = { 2.630f, 0.260f, -0.160f };
    constexpr float WEAPON_TIP_OFFSET = -0.8f;
    constexpr float WEAPON_BASE_OFFSET = -0.2f;

    // 杖モデル
    constexpr float MODEL_EMISSIVE_STRENGTH = 10.0f;
    constexpr float STAFF_EMISSIVE_STRENGTH = 1.0f;

    // コライダー
    constexpr float WEAPON_CAPSULE_RADIUS = 0.15f;
    constexpr float WEAPON_CAPSULE_HEIGHT = 2.1f;
    constexpr DirectX::XMFLOAT3 WEAPON_CAPSULE_OFFSET = { 0.0f, 0.0f, 0.1f };
    constexpr DirectX::XMFLOAT3 WEAPON_CAPSULE_ROTATION = { 0.0f, 0.0f, 0.0f };
    constexpr DirectX::XMFLOAT4 WEAPON_CAPSULE_COLOR = { 1.0f, 0.0f, 0.0f, 1.0f };

    constexpr float WEAPON_SPHERE_RADIUS = 3.0f;
    constexpr DirectX::XMFLOAT3 WEAPON_SPHERE_OFFSET = { 0.0f, 0.0f, -0.55f };
    constexpr DirectX::XMFLOAT4 WEAPON_SPHERE_COLOR = { 0.0f, 1.0f, 0.0f, 1.0f };

    // トレイル
    constexpr float TRAIL_MAX_LIFETIME = 0.3f;
    constexpr DirectX::XMFLOAT3 TRAIL_TIP_OFFSET = { 0.0f, 0.0f, -0.8f };
    constexpr DirectX::XMFLOAT3 TRAIL_BASE_OFFSET = { 0.0f, 0.0f, -0.2f };

    // 入力
    constexpr float INPUT_DEAD_ZONE = 0.15f;

    // ロックオン
    constexpr float MAX_LOCKON_DISTANCE = 250.0f;

    // サウンド音量
    constexpr float SWING_SOUND_VOLUME = 0.5f;
    constexpr float AIR_SLASH_VOLUME = 0.3f;
    constexpr float STAFF_SPIN_VOLUME = 0.3f;
    constexpr float AIR_BEAM_VOLUME = 0.5f;

    // 時間スロー
    constexpr float TIME_SLOW_FACTOR = 0.3f;
    constexpr float TIME_SLOW_DURATION = 15.0f;
    constexpr float TIME_RESET_DURATION = 8.0f;
}