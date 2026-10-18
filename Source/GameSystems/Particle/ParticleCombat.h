#pragma once
#include <memory>
#include "Particle.h"

// 電気系エフェクト管理クラス
// 弓チャージや雷エフェクトを担当
class ParticleElectricity
{
public:
    // 初期化
    void Initialize(ID3D11Device* device);

    // 更新
    void Update(float elapsedTime);

    // 描画
    void Render(const RenderContext& rc);

    // 電気エフェクト生成
    void SpawnElectricityEffect(
        const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& direction,
        float intensity = 1.0f,
        float duration = 0.5f);

    // 雷ヒットエフェクト生成
    void SpawnLightningImpact(
        const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& normal,
        float intensity = 1.0f);

    // 雷ビーム生成
    void SpawnLightningBeam(
        const DirectX::XMFLOAT3& startPos,
        const DirectX::XMFLOAT3& endPos,
        float intensity = 1.0f,
        float duration = 0.2f);

private:
    std::unique_ptr<particleSystem> particles;

    // スプライトシート設定
    struct ElectricityConfig {
        static constexpr int TOTAL_FRAMES = 6;
        static constexpr int ROWS = 1;
        static constexpr int COLS = 6;
    };

    struct ElectricityConstants
    {
        // スプライトシート設定
        static constexpr int MAX_PARTICLES = 1000;
        static constexpr const char* TEXTURE_PATH = "Data/Sprite/Electricity.png";

        // エフェクト基本設定
        static constexpr float MODEL_LENGTH = 4.0f;
        static constexpr int NUM_POINTS = 6;
        static constexpr int PARTICLES_PER_POINT_MIN = 2;
        static constexpr int PARTICLES_PER_POINT_MAX = 3;
        static constexpr int OFFSET_RANGE = 100;              
        static constexpr float OFFSET_SCALE = 0.02f;
        static constexpr float VELOCITY_SCALE = 0.005f;
        static constexpr float SIZE_X = 0.3f;
        static constexpr float SIZE_Y = 0.8f;
        static constexpr float ANIMATION_SPEED = 15.0f;
        static constexpr float ROTATION = 90.0f;

        // ランダムパターン
        static constexpr int LIFETIME_RANGE = 50;           
        static constexpr float LIFETIME_SCALE = 0.01f;
        static constexpr int SPRITE_FRAME_RANGE = 6;

        // 雷インパクト設定
        static constexpr int NUM_BURST = 8;
        static constexpr int BURST_RANGE = 100;              
        static constexpr float BURST_SCALE = 0.03f;
        static constexpr float BURST_SPEED = 10.0f;
        static constexpr float BURST_ACCELERATION = -5.0f;
        static constexpr float BURST_SIZE = 0.8f;
        static constexpr float BURST_SIZE_Y = 2.0f;
        static constexpr float BURST_LIFETIME_BASE = 0.4f;
        static constexpr int BURST_LIFETIME_RANGE = 20;      
        static constexpr float BURST_LIFETIME_SCALE = 0.01f;
        static constexpr float BURST_ANIMATION_SPEED = 25.0f;
        static constexpr float BURST_ROTATION = 180.0f;

        // スパーク設定
        static constexpr int NUM_SPARKS = 15;
        static constexpr int SPARK_RANGE = 100;              
        static constexpr float SPARK_SCALE = 0.02f;
        static constexpr float SPARK_SPEED = 15.0f;
        static constexpr float SPARK_ACCELERATION = -8.0f;
        static constexpr float SPARK_SIZE = 0.4f;
        static constexpr float SPARK_LIFETIME_BASE = 0.3f;
        static constexpr int SPARK_LIFETIME_RANGE = 10;      
        static constexpr float SPARK_LIFETIME_SCALE = 0.01f;
        static constexpr float SPARK_ANIMATION_SPEED = 20.0f;
        static constexpr float SPARK_ROTATION = 90.0f;

        // 雷ビーム設定
        static constexpr int BEAM_PARTICLES_BASE = 80;
        static constexpr float BEAM_PARTICLES_SCALE = 0.3f;
        static constexpr int BEAM_PARTICLES_MAX = 200;
        static constexpr float BEAM_SPREAD = 0.5f;
        static constexpr int BEAM_OFFSET_RANGE = 100;        
        static constexpr float BEAM_OFFSET_SCALE = 0.02f;
        static constexpr float BEAM_SIZE_BASE = 0.1f;
        static constexpr int BEAM_SIZE_RANGE = 20;           
        static constexpr float BEAM_SIZE_SCALE = 0.02f;
        static constexpr float BEAM_VELOCITY_SCALE = 0.001f;
        static constexpr float BEAM_ANIMATION_SPEED = 30.0f;
    };
};


// 杖の弾エフェクト管理
class ParticleStaffProjectile
{
public:
    // 初期化
    void Initialize(ID3D11Device* device);

    // 更新
    void Update(float elapsedTime);

    // 描画
    void Render(const RenderContext& rc);

    // 杖弾エフェクト生成
    void SpawnStaffProjectile(
        const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& direction,
        float speed,
        float projectileLifetime = -1.0f
    );

    // 描画停止
    void StopRendering() { shouldRender = false; }

    // 描画再開
    void StartRendering() { shouldRender = true; }

private:
    std::unique_ptr<particleSystem> particles;
    bool shouldRender = true;
    struct StaffProjectileConstants
    {
        static constexpr int MAX_PARTICLES = 3;
        static constexpr int SPRITE_COLS = 5;
        static constexpr int SPRITE_ROWS = 5;
        static constexpr const char* TEXTURE_PATH = "Data/Sprite/StaffProjectile.png";

        static constexpr float MAX_DISTANCE = 600.0f;
        static constexpr float ANIMATION_BASE_SPEED = 30.0f;
        static constexpr float SIZE = 10.0f;
    };
};
