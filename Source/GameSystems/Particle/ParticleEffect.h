#pragma once
#include <memory>
#include "Particle.h"

// テレポート用エフェクト管理クラス
// チャージ フラッシュ 到着の各段階を管理
class ParticleTeleport
{
public:
    // 初期化
    void Initialize(ID3D11Device* device);

    // 更新
    void Update(float elapsedTime);

    // 描画
    void Render(const RenderContext& rc);

    // チャージエフェクト生成
    void SpawnTeleportCharge(const DirectX::XMFLOAT3& position);

    // フラッシュエフェクト生成
    void SpawnTeleportFlash(const DirectX::XMFLOAT3& position);

    // 到着エフェクト生成
    void SpawnTeleportArrival(const DirectX::XMFLOAT3& position);

    // 一連の順方向再生
    void SpawnFullSequenceForward(const DirectX::XMFLOAT3& position);

    // 一連の逆再生
    void SpawnFullSequenceReverse(const DirectX::XMFLOAT3& position);

private:
    std::unique_ptr<particleSystem> particles;

    // アニメーションフレーム範囲定義
    struct AnimationFrames {
        static constexpr int CHARGE_START = 0;    // チャージ開始フレーム
        static constexpr int CHARGE_END = 3;
        static constexpr int FLASH_START = 4;     // フラッシュ開始フレーム
        static constexpr int FLASH_END = 7;
        static constexpr int ARRIVAL_START = 8;   // 到着開始フレーム
        static constexpr int ARRIVAL_END = 15;
        static constexpr int TOTAL_FRAMES = 16;
    };
    // 初期化
    struct TeleportConfig
    {
        static constexpr int MAX_PARTICLES = 3;
        static constexpr int SPRITE_COLS = 4;
        static constexpr int SPRITE_ROWS = 4;
        static constexpr const char* TEXTURE_PATH = "Data/Sprite/Wave.png";

        static constexpr float CHARGE_LIFETIME = 1.2f;
        static constexpr float CHARGE_SIZE = 30.0f;
        static constexpr float CHARGE_ANIM_SPEED = 2.5f;

        static constexpr float FLASH_LIFETIME = 0.4f;
        static constexpr float FLASH_SIZE = 40.0f;
        static constexpr float FLASH_ANIM_SPEED = 10.0f;

        static constexpr float ARRIVAL_LIFETIME = 1.5f;
        static constexpr float ARRIVAL_SIZE = 30.5f;
        static constexpr float ARRIVAL_ANIM_SPEED = 5.3f;

        static constexpr float FORWARD_LIFETIME = 0.25f;
        static constexpr float FORWARD_SIZE = 30.0f;
        static constexpr float FORWARD_ANIM_SPEED = 64.0f;

        static constexpr float REVERSE_LIFETIME = 0.3f;
        static constexpr float REVERSE_SIZE = 30.0f;
        static constexpr float REVERSE_ANIM_SPEED = 26.6f;
        static constexpr int REVERSE_START_FRAME = 8;
    };
};


// 炎 スモーク系エフェクト管理
class ParticleFlame
{
public:
    // 初期化
    void Initialize(ID3D11Device* device);

    // 更新
    void Update(float elapsedTime);

    // 描画
    void Render(const RenderContext& rc);

    // スモークアニメーション生成
    bool SpawnSmokeAnimation(const DirectX::XMFLOAT3& position, float duration = 0.8f);

    // スモーク終了判定
    bool IsSmokeComplete(const DirectX::XMFLOAT3& position) const;

    // 描画停止
    void StopRendering() { shouldRender = false; }

    // 描画再開
    void StartRendering() { shouldRender = true; }

private:
    std::unique_ptr<particleSystem> particles;
    bool shouldRender = true;

    // スモーク状態管理
    struct SmokeAnimation {
        DirectX::XMFLOAT3 position;
        float elapsedTime;
        float duration;
        bool isActive;
    };

    std::vector<SmokeAnimation> activeAnimations;

    struct FlameConfig
    {
        static constexpr int MAX_PARTICLES = 1;
        static constexpr int SPRITE_COLS = 16;
        static constexpr int SPRITE_ROWS = 5;
        static constexpr const char* TEXTURE_PATH = "Data/Sprite/Flame.png";

        static constexpr float SMOKE_OFFSET_X = 3.0f;
        static constexpr float SMOKE_OFFSET_Y = 5.0f;
        static constexpr float SMOKE_SIZE = 7.0f;
        static constexpr float SMOKE_DEFAULT_DURATION = 0.8f;
        static constexpr float ANIMATION_BASE_SPEED = 80.0f;
        static constexpr float COMPLETE_DISTANCE = 4.0f;
    };
};