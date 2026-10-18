#pragma once

namespace HitConstants
{

    // 共有で使う基本の値
    constexpr int MAX_HIT_HISTORY = 10;                            // ヒット履歴の最大数
    constexpr float LENGTH_EPSILON = 0.001f;                       // ベクトルの長さがゼロかどうか判定するための最小値
    constexpr float DEFAULT_HIT_DIRECTION_Z = 1.0f;                // 方向が取れない時のデフォルト方向
    constexpr float HIT_SOUND_VOLUME = 0.4f;                       // ヒット音の音量
    constexpr float HIT_STOP_MULTIPLIER_NORMAL = 0.4f;             // 通常ヒットストップの倍率
    constexpr float BLOCK_HIT_STOP_MULTIPLIER = 0.3f;              // ガード時のヒットストップ倍率
    constexpr float KNOCKDOWN_HEALTH_RATIO = 0.5f;                 // ノックダウンする体力割合
    constexpr float EFFECT_SCALE = 3.0f;                           // エフェクトの大きさ
    constexpr float DAMAGE_FALLBACK = 0.0f;                        // ダメージの補助値
    constexpr float REGULAR_HIT_STOP_DURATION = 0.08f;             // 通常ヒットの停止時間
    constexpr float STUN_HIT_STOP_DURATION = 0.1f;                 // スタンヒットの停止時間
    constexpr float LAUNCH_HIT_STOP_DURATION = 0.3f;               // 打ち上げヒットの停止時間
    constexpr float REGULAR_HIT_STOP_WEAKEN_FACTOR = 0.7f;         // 通常ヒットの停止時間を弱める倍率
    constexpr float PROJECTILE_HIT_STOP_MULTIPLIER = 0.35f;        // 飛び道具ヒットの停止倍率
    constexpr float PROJECTILE_TRAUMA = 0.4f;                      // カメラ揺れの強さ
    constexpr float ENEMY_WEAPON_HIT_STOP_MULTIPLIER = 0.45f;      // 敵武器ヒット時の停止倍率
    constexpr float ENEMY_PROJECTILE_HIT_STOP_MULTIPLIER = 0.45f;  // 敵飛び道具ヒット時の停止倍率

    // コンボ関連
    constexpr int COMBO_STEP_OFFSET = 1;              // コンボ段階のオフセット
    constexpr int PROJECTILE_COMBO_STEP = 0;          // 飛び道具のコンボ段階

    // プレイヤー用の値
    namespace Player
    {
        constexpr float DEFAULT_ATTACK_STRENGTH = 0.5f;   // 基本の攻撃強さ
        constexpr float LIGHT_ATTACK_STRENGTH = 0.3f;     // 弱攻撃の強さ
        constexpr float HEAVY_ATTACK_STRENGTH = 0.8f;     // 強攻撃の強さ

        constexpr int SECOND_ATTACK_INDEX = 2;            // コンボ2段目
        constexpr int THIRD_ATTACK_INDEX = 3;             // コンボ3段目

        constexpr float STUN_HIT_FORCE = 15.0f;           // スタン攻撃の吹き飛ばし力
        constexpr int STUN_DAMAGE = 25;                   // スタン攻撃のダメージ

        constexpr float LAUNCH_HIT_FORCE = 25.0f;         // 打ち上げ攻撃の力
        constexpr int LAUNCH_DAMAGE = 45;                 // 打ち上げ攻撃のダメージ

        constexpr float REGULAR_HIT_FORCE_BASE = 8.0f;    // 通常攻撃の基本力
        constexpr int REGULAR_DAMAGE_BASE = 10;           // 通常攻撃の基本ダメージ

        constexpr float COMBO_FORCE_MULTIPLIER = 1.5f;    // コンボによる力の増加
        constexpr int COMBO_DAMAGE_MULTIPLIER = 2;        // コンボによるダメージ増加

        constexpr float AIR_ATTACK_HIT_FORCE = 12.0f;     // 空中攻撃の力
        constexpr int AIR_ATTACK_DAMAGE = 15;             // 空中攻撃のダメージ

        constexpr float PROJECTILE_HIT_FORCE = 5.0f;      // 飛び道具の力
        constexpr int PROJECTILE_DAMAGE = 8;              // 飛び道具のダメージ

        constexpr float COUNTER_HIT_FORCE = 25.0f;        // カウンター攻撃の力
        constexpr int COUNTER_DAMAGE = 50;                // カウンター攻撃のダメージ

        constexpr float DEFAULT_HIT_FORCE = 8.0f;         // デフォルトの力
        constexpr int DEFAULT_DAMAGE = 10;                // デフォルトのダメージ

        constexpr float BLOCK_HIT_FORCE_MULTIPLIER = 5.0f;// ガード時の反発力
        constexpr float BLOCK_TRAUMA = 0.3f;              // ガード時のカメラ揺れ
        constexpr float POWER_CHARGE_TRAUMA = 0.5f;       // 強攻撃時のカメラ揺れ
    }

    // 敵用の値
    namespace Enemy
    {
        constexpr float DEFAULT_HIT_FORCE = 8.0f;         // 基本の攻撃力
        constexpr int DEFAULT_DAMAGE = 8;                 // 基本ダメージ

        constexpr float STUN_HIT_FORCE = 15.0f;           // スタン攻撃の力
        constexpr int STUN_DAMAGE = 15;                   // スタン攻撃のダメージ

        constexpr float LAUNCH_HIT_FORCE = 25.0f;         // 打ち上げ攻撃の力
        constexpr int LAUNCH_DAMAGE = 20;                 // 打ち上げ攻撃のダメージ

        constexpr int POWER_CHARGE_DAMAGE = 20;           // チャージ攻撃のダメージ
        constexpr float POWER_CHARGE_HIT_FORCE = 25.0f;   // チャージ攻撃の力

        constexpr int POWER_CHARGE_ATTACK_DAMAGE = 50;    // チャージ中攻撃のダメージ
        constexpr float POWER_CHARGE_ATTACK_HIT_FORCE = 40.0f; // チャージ中攻撃の力

        constexpr float COMBO_ATTACK_HIT_FORCE_LIGHT = 12.0f; // 軽いコンボ攻撃の力
        constexpr int COMBO_ATTACK_DAMAGE_LIGHT = 10;         // 軽いコンボ攻撃のダメージ

        constexpr float COMBO_ATTACK_HIT_FORCE_MEDIUM = 15.0f;// 中程度コンボ攻撃の力
        constexpr int COMBO_ATTACK_DAMAGE_MEDIUM = 15;        // 中程度コンボ攻撃のダメージ

        constexpr int FIRST_SLASH_INDEX = 1;              // 1回目の斬撃
        constexpr int SECOND_SLASH_INDEX = 2;             // 2回目の斬撃
        constexpr int DOMAIN_SLASH_COMBO_STEP = 1;        // 特殊攻撃のコンボ段階

        constexpr float PROJECTILE_LAUNCH_THRESHOLD = 25.0f; // 打ち上げ判定のしきい値
        constexpr float PROJECTILE_STUN_THRESHOLD = 15.0f;   // スタン判定のしきい値
        constexpr float PROJECTILE_FORCE_HITSTOP_THRESHOLD = 12.0f; // ヒットストップ判定のしきい値

        constexpr float LIGHTNING_IMPACT_SCALE = 8.0f;    // 雷エフェクトの大きさ
    }

}