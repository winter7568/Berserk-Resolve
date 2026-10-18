#pragma once
#include <memory>
#include "Particle.h"

// 汎用スプライトパーティクル管理クラス
// 弾の軌跡や斬撃エフェクトなどを担当
class ParticleSprite
{
public:
    // 初期化
    void Initialize(ID3D11Device* device);

    // 毎フレーム更新
    void Update(float elapsedTime);

    // 描画
    void Render(const RenderContext& rc);

    // 背景用のエネルギーフィールド生成
    void SpawnBackgroundEnergyField(float elapsedTime);

    // プレイヤー弾の軌跡生成
    void SpawnPlayerBulletTrail(
        const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& direction,
        float lifetime);

    // 斬撃の軌跡生成
    void SpawnSlashTrail(
        const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& direction,
        const DirectX::XMFLOAT3& orientation,
        float slashLength,
        float lifetime);

    // 斬撃のエッジ発光エフェクト
    void SpawnSlashEdgeGlow(
        const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& orientation,
        float slashLength);

    // スタッフオーラパーティクル生成
    void SpawnStaffAuraParticles(
        const DirectX::XMFLOAT3& tip,
        const DirectX::XMFLOAT3& base,
        const DirectX::XMFLOAT3& dir,
        float elapsedTime
    );

private:
    std::unique_ptr<particleSystem> particles;

    // スプライトシート設定
    struct SpriteSheetConfig
    {
        static constexpr int COLS = 4;
        static constexpr int ROWS = 5;
        static constexpr const char* TEXTURE_PATH = "Data/Sprite/ParticleSprite.png";
        static constexpr int MAX_PARTICLES = 10000;
    };

    // エネルギーフィールド設定
    struct EnergyFieldConfig
    {
        static constexpr float SPAWN_INTERVAL = 0.35f;
        static constexpr int PARTICLES_PER_WAVE = 360;
        static constexpr int POS_RANGE_XZ = 11600;
        static constexpr int POS_RANGE_Y = 8000;
        static constexpr float POS_SCALE_XZ = 0.1f;
        static constexpr float POS_SCALE_Y = 0.01f;
        static constexpr int VEL_RANGE = 200;
        static constexpr float VEL_SCALE = 0.01f;
        static constexpr int VEL_RANGE_Y = 100;
        static constexpr float VEL_SCALE_Y = 0.02f;
        static constexpr float ACCELERATION_Y = -0.1f;
        static constexpr float SIZE_BASE = 0.8f;
        static constexpr int SIZE_RANGE = 20;
        static constexpr float SIZE_SCALE = 0.05f;
        static constexpr float LIFETIME_BASE = 2.0f;
        static constexpr int LIFETIME_RANGE = 20;
        static constexpr float LIFETIME_SCALE = 0.1f;
        static constexpr int SPRITE_INDEX_A = 7;
        static constexpr int SPRITE_INDEX_B = 16;
    };

    // プレイヤー弾軌跡設定
    struct BulletTrailConfig
    {
        static constexpr int COUNT_MIN = 1;
        static constexpr int COUNT_MAX = 2;
        static constexpr float BEHIND_BASE = 0.5f;
        static constexpr int BEHIND_RANGE = 100;
        static constexpr float BEHIND_SCALE = 0.02f;
        static constexpr float SPREAD = 0.4f;
        static constexpr int OFFSET_RANGE = 200;
        static constexpr float OFFSET_SCALE = 0.01f;
        static constexpr float SPEED = 8.0f;
        static constexpr int VEL_RANGE = 100;
        static constexpr float VEL_SCALE = 0.03f;
        static constexpr float SIZE_BASE = 0.1f;
        static constexpr int SIZE_RANGE = 10;
        static constexpr float SIZE_SCALE = 0.01f;
        static constexpr float LIFETIME_BASE = 0.6f;
        static constexpr int LIFETIME_RANGE = 10;
        static constexpr float LIFETIME_SCALE = 0.01f;
        static constexpr int SPRITE_INDEX = 10;
    };

    // 斬撃軌跡設定
    struct SlashTrailConfig
    {
        static constexpr int NUM_ENDS = 2;
        static constexpr int OFFSET_RANGE = 100;
        static constexpr float OFFSET_SCALE = 0.005f;
        static constexpr float SPEED = 2.0f;
        static constexpr int VEL_RANGE = 100;
        static constexpr float VEL_SCALE = 0.02f;
        static constexpr float SIZE_BASE = 0.15f;
        static constexpr int SIZE_RANGE = 10;
        static constexpr float SIZE_SCALE = 0.02f;
        static constexpr float LIFETIME_BASE = 0.4f;
        static constexpr int LIFETIME_RANGE = 10;
        static constexpr float LIFETIME_SCALE = 0.02f;
        static constexpr int SPRITE_INDEX = 5;
        static constexpr float ANIMATION_SPEED = 6.0f;
        static constexpr float ACCELERATION_Y = 0.5f;
    };

    // 斬撃エッジ発光設定
    struct SlashEdgeGlowConfig
    {
        static constexpr int NUM_PARTICLES = 4;
        static constexpr int OFFSET_RANGE = 100;
        static constexpr float OFFSET_SCALE = 0.005f;
        static constexpr int VEL_RANGE = 100;
        static constexpr float VEL_SCALE = 0.01f;
        static constexpr float LIFETIME = 0.3f;
        static constexpr float SIZE_BASE = 0.25f;
        static constexpr int SIZE_RANGE = 10;
        static constexpr float SIZE_SCALE = 0.01f;
        static constexpr int SPRITE_INDEX = 12;
    };

    // スタッフオーラ設定
    struct StaffAuraConfig
    {
        static constexpr int SPRITE_INDEX = 18;
        static constexpr float SPAWN_INTERVAL = 0.05f;
        static constexpr int PARTICLES_PER_WAVE = 3;
        static constexpr float SPREAD_RANGE = 1.8f;
        static constexpr float SPREAD_Y_FACTOR = 0.5f;
        static constexpr float FORWARD_FACTOR_MIN = 0.1f;
        static constexpr int FORWARD_RANGE = 101;
        static constexpr float FORWARD_SCALE = 0.01f;
        static constexpr float DRIFT_RANGE = 0.2f;
        static constexpr int DRIFT_RANGE_VALUE = 401;
        static constexpr float DRIFT_DIVISOR = 1000.0f;
        static constexpr float DRIFT_MULTIPLIER = 2.0f;
        static constexpr float DECELERATION_RATE = 0.05f;
        static constexpr float SIZE_BASE = 0.30f;
        static constexpr int SIZE_RANGE = 20;
        static constexpr float SIZE_SCALE = 0.005f;
        static constexpr float LIFETIME_BASE = 0.3f;
        static constexpr int LIFETIME_RANGE = 10;
        static constexpr float LIFETIME_SCALE = 0.05f;
        static constexpr bool ANIMATE_DISABLED = false;
        static constexpr float ANIMATE_SPEED_ZERO = 0.0f;
        static constexpr int POS_RANGE = 200;
        static constexpr float POS_SCALE = 0.01f;
    };
};