#include "ParticleSprite.h"
#include "GameLib/GpuResourceUtils.h"
#include "GameLib/TransformUtils.h"


void ParticleSprite::Initialize(ID3D11Device* device)
{
    D3D11_TEXTURE2D_DESC texture2d_desc;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;

    // パーティクル用スプライトシートを読み込む
    GpuResourceUtils::LoadTexture(device, SpriteSheetConfig::TEXTURE_PATH,
        shader_resource_view.GetAddressOf(),
        &texture2d_desc);

    // パーティクルシステム生成
    // 横4列 縦5行のスプライトシートを使用
    particles = std::make_unique<particleSystem>(
        device,
        shader_resource_view,
        SpriteSheetConfig::COLS,
        SpriteSheetConfig::ROWS,
        SpriteSheetConfig::MAX_PARTICLES
    );
}

void ParticleSprite::Update(float elapsedTime)
{
    // パーティクルの更新
    if (particles)
        particles->update(elapsedTime);
}

void ParticleSprite::Render(const RenderContext& rc)
{
    if (particles)
    {
        // 加算合成で発光表現を行う
        const float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        rc.deviceContext->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Additive), blendFactor, 0xFFFFFFFF);

        // 深度はテストのみで書き込みしない
        rc.deviceContext->OMSetDepthStencilState(rc.renderState->GetDepthStencilState(DepthState::TestOnly), 0);

        // カリングなしで描画
        rc.deviceContext->RSSetState(rc.renderState->GetRasterizerState(RasterizerState::SolidCullNone));

        // パーティクル描画
        particles->render(rc);
    }
}

void ParticleSprite::SpawnBackgroundEnergyField(float elapsedTime)
{
    // エネルギーフィールド用の生成タイマー
    static float energySpawnTimer = 0.0f;
    energySpawnTimer += elapsedTime;

    // 一定間隔で生成
    if (energySpawnTimer >= EnergyFieldConfig::SPAWN_INTERVAL)
    {
        energySpawnTimer = 0.0f;

        // 一度に大量生成して空間を満たす
        for (int i = 0; i < EnergyFieldConfig::PARTICLES_PER_WAVE; ++i)
        {
            // ランダム位置生成
            DirectX::XMFLOAT3 p = {
                TransformUtils::RandomCentered(EnergyFieldConfig::POS_RANGE_XZ, EnergyFieldConfig::POS_SCALE_XZ),
                TransformUtils::RandomPositiveOnly(EnergyFieldConfig::POS_RANGE_Y, EnergyFieldConfig::POS_SCALE_Y),
                TransformUtils::RandomCentered(EnergyFieldConfig::POS_RANGE_XZ, EnergyFieldConfig::POS_SCALE_XZ)
            };

            // ランダム速度
            DirectX::XMFLOAT3 v = {
                TransformUtils::RandomCentered(EnergyFieldConfig::VEL_RANGE, EnergyFieldConfig::VEL_SCALE),
                TransformUtils::RandomCentered(EnergyFieldConfig::VEL_RANGE_Y, EnergyFieldConfig::VEL_SCALE_Y),
                TransformUtils::RandomCentered(EnergyFieldConfig::VEL_RANGE, EnergyFieldConfig::VEL_SCALE)
            };

            // ゆっくり落下させる
            DirectX::XMFLOAT3 a = { 0.0f, EnergyFieldConfig::ACCELERATION_Y, 0.0f };

            // サイズランダム
            float s = TransformUtils::RandomPositive(EnergyFieldConfig::SIZE_BASE, EnergyFieldConfig::SIZE_RANGE, EnergyFieldConfig::SIZE_SCALE);
            DirectX::XMFLOAT2 size = { s, s };

            // ライフタイム
            float lifetime = TransformUtils::RandomPositive(EnergyFieldConfig::LIFETIME_BASE, EnergyFieldConfig::LIFETIME_RANGE, EnergyFieldConfig::LIFETIME_SCALE);

            // 使用するスプライトフレーム
            int spriteIndex = (TransformUtils::RandomInt(0, 1) == 0) ? EnergyFieldConfig::SPRITE_INDEX_A : EnergyFieldConfig::SPRITE_INDEX_B;

            particles->set(spriteIndex, lifetime, p, v, a, size, false, 0.0f);
        }
    }
}

void ParticleSprite::SpawnPlayerBulletTrail(
    const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT3& direction,
    float lifetime)
{
    if (!particles) return;

    // 毎フレーム少量生成して軌跡を作る
    for (int i = 0; i < TransformUtils::RandomInt(BulletTrailConfig::COUNT_MIN, BulletTrailConfig::COUNT_MAX); ++i)
    {
        // 弾の後ろ側に配置
        float behindDistance = TransformUtils::RandomPositive(BulletTrailConfig::BEHIND_BASE, BulletTrailConfig::BEHIND_RANGE, BulletTrailConfig::BEHIND_SCALE);
        DirectX::XMFLOAT3 trailPos = {
            position.x - direction.x * behindDistance,
            position.y - direction.y * behindDistance,
            position.z - direction.z * behindDistance
        };

        // 横方向にばらつきを持たせる
        trailPos.x += TransformUtils::RandomCentered(BulletTrailConfig::OFFSET_RANGE, BulletTrailConfig::OFFSET_SCALE * BulletTrailConfig::SPREAD);
        trailPos.y += TransformUtils::RandomCentered(BulletTrailConfig::OFFSET_RANGE, BulletTrailConfig::OFFSET_SCALE * BulletTrailConfig::SPREAD);
        trailPos.z += TransformUtils::RandomCentered(BulletTrailConfig::OFFSET_RANGE, BulletTrailConfig::OFFSET_SCALE * BulletTrailConfig::SPREAD);

        // 後ろ方向へ飛ばす
        DirectX::XMFLOAT3 trailVelocity = {
            -direction.x * BulletTrailConfig::SPEED + TransformUtils::RandomCentered(BulletTrailConfig::VEL_RANGE, BulletTrailConfig::VEL_SCALE),
            -direction.y * BulletTrailConfig::SPEED + TransformUtils::RandomCentered(BulletTrailConfig::VEL_RANGE, BulletTrailConfig::VEL_SCALE),
            -direction.z * BulletTrailConfig::SPEED + TransformUtils::RandomCentered(BulletTrailConfig::VEL_RANGE, BulletTrailConfig::VEL_SCALE)
        };

        // サイズにばらつき
        float randomSize = TransformUtils::RandomPositive(BulletTrailConfig::SIZE_BASE, BulletTrailConfig::SIZE_RANGE, BulletTrailConfig::SIZE_SCALE);

        particles->set(
            BulletTrailConfig::SPRITE_INDEX,
            TransformUtils::RandomPositive(BulletTrailConfig::LIFETIME_BASE, BulletTrailConfig::LIFETIME_RANGE, BulletTrailConfig::LIFETIME_SCALE),
            trailPos,
            trailVelocity,
            { 0, 0, 0 },
            { randomSize, randomSize },
            false,
            0.0f,
            0.0f,
            0.0f
        );
    }
}

void ParticleSprite::SpawnSlashTrail(
    const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT3& direction,
    const DirectX::XMFLOAT3& orientation,
    float slashLength,
    float lifetime)
{
    if (!particles) return;

    // スラッシュ両端にパーティクル生成
    for (int i = 0; i < SlashTrailConfig::NUM_ENDS; ++i)
    {
        float t = (float)i / (float)(SlashTrailConfig::NUM_ENDS - 1);
        float posAlongSlash = (t - 0.5f) * slashLength;

        // スラッシュ上の位置を計算
        DirectX::XMFLOAT3 endPos = {
            position.x + orientation.x * posAlongSlash,
            position.y + orientation.y * posAlongSlash,
            position.z + orientation.z * posAlongSlash
        };

        // 微小ランダムオフセット
        endPos.x += TransformUtils::RandomCentered(SlashTrailConfig::OFFSET_RANGE, SlashTrailConfig::OFFSET_SCALE);
        endPos.y += TransformUtils::RandomCentered(SlashTrailConfig::OFFSET_RANGE, SlashTrailConfig::OFFSET_SCALE);
        endPos.z += TransformUtils::RandomCentered(SlashTrailConfig::OFFSET_RANGE, SlashTrailConfig::OFFSET_SCALE);

        // 軌跡として後方へ流す
        DirectX::XMFLOAT3 trailVelocity = {
            -direction.x * SlashTrailConfig::SPEED + TransformUtils::RandomCentered(SlashTrailConfig::VEL_RANGE, SlashTrailConfig::VEL_SCALE),
            -direction.y * SlashTrailConfig::SPEED + TransformUtils::RandomCentered(SlashTrailConfig::VEL_RANGE, SlashTrailConfig::VEL_SCALE),
            -direction.z * SlashTrailConfig::SPEED + TransformUtils::RandomCentered(SlashTrailConfig::VEL_RANGE, SlashTrailConfig::VEL_SCALE)
        };

        float randomSize = TransformUtils::RandomPositive(SlashTrailConfig::SIZE_BASE, SlashTrailConfig::SIZE_RANGE, SlashTrailConfig::SIZE_SCALE);

        particles->set(
            SlashTrailConfig::SPRITE_INDEX,
            TransformUtils::RandomPositive(SlashTrailConfig::LIFETIME_BASE, SlashTrailConfig::LIFETIME_RANGE, SlashTrailConfig::LIFETIME_SCALE),
            endPos,
            trailVelocity,
            { 0, SlashTrailConfig::ACCELERATION_Y, 0 },
            { randomSize, randomSize },
            true,
            SlashTrailConfig::ANIMATION_SPEED,
            0.0f,
            0.0f
        );
    }
}

void ParticleSprite::SpawnSlashEdgeGlow(
    const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT3& orientation,
    float slashLength)
{
    if (!particles) return;

    // スラッシュの始点と終点を計算
    float halfLength = slashLength * 0.5f;

    DirectX::XMFLOAT3 startPos = {
        position.x - orientation.x * halfLength,
        position.y - orientation.y * halfLength,
        position.z - orientation.z * halfLength
    };

    DirectX::XMFLOAT3 endPos = {
        position.x + orientation.x * halfLength,
        position.y + orientation.y * halfLength,
        position.z + orientation.z * halfLength
    };

    // スラッシュ全体に沿って発光粒子を配置
    for (int i = 0; i < SlashEdgeGlowConfig::NUM_PARTICLES; ++i)
    {
        float t = i / (float)(SlashEdgeGlowConfig::NUM_PARTICLES - 1);

        DirectX::XMFLOAT3 edgePos = {
            startPos.x + (endPos.x - startPos.x) * t,
            startPos.y + (endPos.y - startPos.y) * t,
            startPos.z + (endPos.z - startPos.z) * t
        };

        // 微小ランダムオフセット
        edgePos.x += TransformUtils::RandomCentered(SlashEdgeGlowConfig::OFFSET_RANGE, SlashEdgeGlowConfig::OFFSET_SCALE);
        edgePos.y += TransformUtils::RandomCentered(SlashEdgeGlowConfig::OFFSET_RANGE, SlashEdgeGlowConfig::OFFSET_SCALE);
        edgePos.z += TransformUtils::RandomCentered(SlashEdgeGlowConfig::OFFSET_RANGE, SlashEdgeGlowConfig::OFFSET_SCALE);

        // わずかな動きだけ与える
        DirectX::XMFLOAT3 particleVelocity = {
            TransformUtils::RandomCentered(SlashEdgeGlowConfig::VEL_RANGE, SlashEdgeGlowConfig::VEL_SCALE),
            TransformUtils::RandomCentered(SlashEdgeGlowConfig::VEL_RANGE, SlashEdgeGlowConfig::VEL_SCALE),
            TransformUtils::RandomCentered(SlashEdgeGlowConfig::VEL_RANGE, SlashEdgeGlowConfig::VEL_SCALE)
        };

        // 発光用サイズ
        float size = TransformUtils::RandomPositive(SlashEdgeGlowConfig::SIZE_BASE, SlashEdgeGlowConfig::SIZE_RANGE, SlashEdgeGlowConfig::SIZE_SCALE);

        particles->set(
            SlashEdgeGlowConfig::SPRITE_INDEX,
            SlashEdgeGlowConfig::LIFETIME,
            edgePos,
            particleVelocity,
            { 0, 0, 0 },
            { size, size },
            false,
            0.0f,
            0.0f,
            0.0f
        );
    }
}

void ParticleSprite::SpawnStaffAuraParticles(
    const DirectX::XMFLOAT3& tip,
    const DirectX::XMFLOAT3& base,
    const DirectX::XMFLOAT3& dir,
    float elapsedTime)
{
    if (!particles) return;

    static float auraTimer = 0.0f;
    auraTimer += elapsedTime;

    if (auraTimer >= StaffAuraConfig::SPAWN_INTERVAL)
    {
        auraTimer = 0.0f;

        for (int i = 0; i < StaffAuraConfig::PARTICLES_PER_WAVE; ++i)
        {
            // 武器の先端周辺にランダムな広がりでパーティクルを生成
            DirectX::XMFLOAT3 p = {
                tip.x + TransformUtils::RandomCentered(StaffAuraConfig::POS_RANGE, StaffAuraConfig::POS_SCALE * StaffAuraConfig::SPREAD_RANGE),
                tip.y + TransformUtils::RandomCentered(StaffAuraConfig::POS_RANGE, StaffAuraConfig::POS_SCALE * (StaffAuraConfig::SPREAD_RANGE * StaffAuraConfig::SPREAD_Y_FACTOR)),
                tip.z + TransformUtils::RandomCentered(StaffAuraConfig::POS_RANGE, StaffAuraConfig::POS_SCALE * StaffAuraConfig::SPREAD_RANGE)
            };

            // 優しいドリフト運動を追加
            float forwardFactor = TransformUtils::RandomPositive(
                StaffAuraConfig::FORWARD_FACTOR_MIN,
                StaffAuraConfig::FORWARD_RANGE,
                StaffAuraConfig::FORWARD_SCALE
            );

            float driftScale = (1.0f - fabsf(dir.y)) * StaffAuraConfig::DRIFT_RANGE;
            float driftValue = StaffAuraConfig::DRIFT_MULTIPLIER * driftScale / StaffAuraConfig::DRIFT_DIVISOR;

            DirectX::XMFLOAT3 v = {
                dir.x * forwardFactor + TransformUtils::RandomCentered(StaffAuraConfig::DRIFT_RANGE_VALUE, driftValue),
                dir.y * forwardFactor + TransformUtils::RandomCentered(StaffAuraConfig::DRIFT_RANGE_VALUE, driftValue),
                dir.z * forwardFactor + TransformUtils::RandomCentered(StaffAuraConfig::DRIFT_RANGE_VALUE, driftValue)
            };

            // 徐々に減速する加速度
            DirectX::XMFLOAT3 a = {
                -dir.x * StaffAuraConfig::DECELERATION_RATE,
                -dir.y * StaffAuraConfig::DECELERATION_RATE,
                -dir.z * StaffAuraConfig::DECELERATION_RATE
            };

            // サイズと寿命をランダム化
            float s = TransformUtils::RandomPositive(
                StaffAuraConfig::SIZE_BASE,
                StaffAuraConfig::SIZE_RANGE,
                StaffAuraConfig::SIZE_SCALE
            );
            DirectX::XMFLOAT2 size = { s, s };

            float lifetime = TransformUtils::RandomPositive(
                StaffAuraConfig::LIFETIME_BASE,
                StaffAuraConfig::LIFETIME_RANGE,
                StaffAuraConfig::LIFETIME_SCALE
            );

            particles->set(
                StaffAuraConfig::SPRITE_INDEX,
                lifetime,
                p,
                v,
                a,
                size,
                StaffAuraConfig::ANIMATE_DISABLED,
                StaffAuraConfig::ANIMATE_SPEED_ZERO
            );
        }
    }
}