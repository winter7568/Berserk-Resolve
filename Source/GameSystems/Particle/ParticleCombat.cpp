#include "ParticleCombat.h"
#include "GameLib/GpuResourceUtils.h"
#include "GameLib/TransformUtils.h"

// ============================================================================
// ParticleElectricity Implementation
// ============================================================================

void ParticleElectricity::Initialize(ID3D11Device* device)
{
    D3D11_TEXTURE2D_DESC texture2d_desc;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;

    // 6フレーム構成の電気スプライトシートを読み込み
    GpuResourceUtils::LoadTexture(device, ElectricityConstants::TEXTURE_PATH,
        shader_resource_view.GetAddressOf(),
        &texture2d_desc);

    // 6列1行のグリッドでパーティクルシステムを初期化
    particles = std::make_unique<particleSystem>(device,
        shader_resource_view,
        ElectricityConfig::COLS,
        ElectricityConfig::ROWS,
        ElectricityConstants::MAX_PARTICLES);
}

void ParticleElectricity::Update(float elapsedTime)
{
    if (particles)
    {
        // パーティクルの更新処理
        particles->update(elapsedTime);
    }
}

void ParticleElectricity::Render(const RenderContext& rc)
{
    if (particles)
    {
        // 発光表現のため加算ブレンドを設定
        const float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        rc.deviceContext->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Additive), blendFactor, 0xFFFFFFFF);
        rc.deviceContext->OMSetDepthStencilState(rc.renderState->GetDepthStencilState(DepthState::TestOnly), 0);
        rc.deviceContext->RSSetState(rc.renderState->GetRasterizerState(RasterizerState::SolidCullNone));

        // 描画処理
        particles->render(rc);
    }
}

void ParticleElectricity::SpawnElectricityEffect(const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT3& direction,
    float intensity,
    float duration)
{
    if (!particles) return;

    // モデル全体に沿って電気を分布させる
    for (int i = 0; i < ElectricityConstants::NUM_POINTS; ++i) {
        // モデル上の位置を計算
        float t = (float)i / (float)(ElectricityConstants::NUM_POINTS - 1);
        float posAlongModel = (t - 0.5f) * ElectricityConstants::MODEL_LENGTH;

        DirectX::XMFLOAT3 pointPos = {
            position.x + direction.x * posAlongModel,
            position.y + direction.y * posAlongModel,
            position.z + direction.z * posAlongModel
        };

        // 各ポイントに複数の電気パーティクルを生成
        for (int j = 0; j < TransformUtils::RandomInt(ElectricityConstants::PARTICLES_PER_POINT_MIN, ElectricityConstants::PARTICLES_PER_POINT_MAX); ++j) {
            // ランダムなオフセットで自然なばらつきを追加
            DirectX::XMFLOAT3 offsetPos = {
                pointPos.x + TransformUtils::RandomCentered(ElectricityConstants::OFFSET_RANGE, ElectricityConstants::OFFSET_SCALE),
                pointPos.y + TransformUtils::RandomCentered(ElectricityConstants::OFFSET_RANGE, ElectricityConstants::OFFSET_SCALE),
                pointPos.z + TransformUtils::RandomCentered(ElectricityConstants::OFFSET_RANGE, ElectricityConstants::OFFSET_SCALE)
            };

            particles->set(
                TransformUtils::RandomInt(0, ElectricityConstants::SPRITE_FRAME_RANGE - 1),
                TransformUtils::RandomScaledLifetime(duration, ElectricityConstants::LIFETIME_RANGE, ElectricityConstants::LIFETIME_SCALE),
                offsetPos,
                {
                    TransformUtils::RandomCentered(ElectricityConstants::OFFSET_RANGE, ElectricityConstants::VELOCITY_SCALE),
                    TransformUtils::RandomCentered(ElectricityConstants::OFFSET_RANGE, ElectricityConstants::VELOCITY_SCALE),
                    TransformUtils::RandomCentered(ElectricityConstants::OFFSET_RANGE, ElectricityConstants::VELOCITY_SCALE)
                },
                { 0, 0, 0 },
                { ElectricityConstants::SIZE_X, ElectricityConstants::SIZE_Y },
                true,
                ElectricityConstants::ANIMATION_SPEED,
                DirectX::XMConvertToRadians(ElectricityConstants::ROTATION),
                0.0f
            );
        }
    }
}

void ParticleElectricity::SpawnLightningImpact(const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT3& normal,
    float intensity)
{
    if (!particles) return;

    // 中心から放射状に広がる大きな雷エフェクト
    for (int i = 0; i < ElectricityConstants::NUM_BURST; ++i) {
        // 法線方向をベースにランダムな方向を生成
        DirectX::XMFLOAT3 burstDir = {
            normal.x + TransformUtils::RandomCentered(ElectricityConstants::BURST_RANGE, ElectricityConstants::BURST_SCALE),
            normal.y + TransformUtils::RandomCentered(ElectricityConstants::BURST_RANGE, ElectricityConstants::BURST_SCALE),
            normal.z + TransformUtils::RandomCentered(ElectricityConstants::BURST_RANGE, ElectricityConstants::BURST_SCALE)
        };

        // 正規化
        float length = sqrtf(burstDir.x * burstDir.x + burstDir.y * burstDir.y + burstDir.z * burstDir.z);
        if (length > 0.001f) {
            burstDir.x /= length;
            burstDir.y /= length;
            burstDir.z /= length;
        }

        // 大きな雷パーティクルを生成
        particles->set(
            TransformUtils::RandomInt(0, ElectricityConstants::SPRITE_FRAME_RANGE - 1),
            TransformUtils::RandomPositive(ElectricityConstants::BURST_LIFETIME_BASE, ElectricityConstants::BURST_LIFETIME_RANGE, ElectricityConstants::BURST_LIFETIME_SCALE),
            position,
            { burstDir.x * ElectricityConstants::BURST_SPEED, burstDir.y * ElectricityConstants::BURST_SPEED, burstDir.z * ElectricityConstants::BURST_SPEED },
            { 0, ElectricityConstants::BURST_ACCELERATION, 0 },
            { ElectricityConstants::BURST_SIZE * intensity, ElectricityConstants::BURST_SIZE_Y * intensity },
            true,
            ElectricityConstants::BURST_ANIMATION_SPEED,
            DirectX::XMConvertToRadians(ElectricityConstants::BURST_ROTATION),
            0.0f
        );
    }

    // 周囲に散る小さなスパーク
    for (int i = 0; i < ElectricityConstants::NUM_SPARKS; ++i) {
        DirectX::XMFLOAT3 sparkDir = {
            TransformUtils::RandomCentered(ElectricityConstants::SPARK_RANGE, ElectricityConstants::SPARK_SCALE),
            TransformUtils::RandomCentered(ElectricityConstants::SPARK_RANGE, ElectricityConstants::SPARK_SCALE),
            TransformUtils::RandomCentered(ElectricityConstants::SPARK_RANGE, ElectricityConstants::SPARK_SCALE)
        };

        particles->set(
            TransformUtils::RandomInt(0, ElectricityConstants::SPRITE_FRAME_RANGE - 1),
            TransformUtils::RandomPositive(ElectricityConstants::SPARK_LIFETIME_BASE, ElectricityConstants::SPARK_LIFETIME_RANGE, ElectricityConstants::SPARK_LIFETIME_SCALE),
            position,
            { sparkDir.x * ElectricityConstants::SPARK_SPEED, sparkDir.y * ElectricityConstants::SPARK_SPEED, sparkDir.z * ElectricityConstants::SPARK_SPEED },
            { 0, ElectricityConstants::SPARK_ACCELERATION, 0 },
            { ElectricityConstants::SPARK_SIZE, ElectricityConstants::SPARK_SIZE },
            true,
            ElectricityConstants::SPARK_ANIMATION_SPEED,
            DirectX::XMConvertToRadians(ElectricityConstants::SPARK_ROTATION),
            0.0f
        );
    }
}

void ParticleElectricity::SpawnLightningBeam(
    const DirectX::XMFLOAT3& startPos,
    const DirectX::XMFLOAT3& endPos,
    float intensity,
    float duration)
{
    if (!particles) return;

    // ビームの方向と長さを計算
    DirectX::XMFLOAT3 beamDir = {
        endPos.x - startPos.x,
        endPos.y - startPos.y,
        endPos.z - startPos.z
    };

    float beamLength = sqrtf(beamDir.x * beamDir.x + beamDir.y * beamDir.y + beamDir.z * beamDir.z);
    if (beamLength < 0.001f) return;

    // 正規化
    beamDir.x /= beamLength;
    beamDir.y /= beamLength;
    beamDir.z /= beamLength;

    // ビーム長に応じたパーティクル数を設定
    int numParticles = ElectricityConstants::BEAM_PARTICLES_BASE + (int)(beamLength * ElectricityConstants::BEAM_PARTICLES_SCALE);
    numParticles = min(numParticles, ElectricityConstants::BEAM_PARTICLES_MAX);

    for (int i = 0; i < numParticles; ++i) {
        float t = (float)i / (float)numParticles;

        // ビーム全体に沿った位置を計算
        DirectX::XMFLOAT3 particlePos = {
            startPos.x + beamDir.x * beamLength * t,
            startPos.y + beamDir.y * beamLength * t,
            startPos.z + beamDir.z * beamLength * t
        };

        // 横方向に広がりを持たせる
        particlePos.x += TransformUtils::RandomCentered(ElectricityConstants::BEAM_OFFSET_RANGE, ElectricityConstants::BEAM_OFFSET_SCALE * ElectricityConstants::BEAM_SPREAD);
        particlePos.y += TransformUtils::RandomCentered(ElectricityConstants::BEAM_OFFSET_RANGE, ElectricityConstants::BEAM_OFFSET_SCALE * ElectricityConstants::BEAM_SPREAD);
        particlePos.z += TransformUtils::RandomCentered(ElectricityConstants::BEAM_OFFSET_RANGE, ElectricityConstants::BEAM_OFFSET_SCALE * ElectricityConstants::BEAM_SPREAD);

        // パーティクルサイズにばらつきを追加
        float size = TransformUtils::RandomPositive(ElectricityConstants::BEAM_SIZE_BASE, ElectricityConstants::BEAM_SIZE_RANGE, ElectricityConstants::BEAM_SIZE_SCALE);

        particles->set(
            TransformUtils::RandomInt(0, ElectricityConstants::SPRITE_FRAME_RANGE - 1),
            duration,
            particlePos,
            {
                TransformUtils::RandomCentered(ElectricityConstants::BEAM_OFFSET_RANGE, ElectricityConstants::BEAM_VELOCITY_SCALE),
                TransformUtils::RandomCentered(ElectricityConstants::BEAM_OFFSET_RANGE, ElectricityConstants::BEAM_VELOCITY_SCALE),
                TransformUtils::RandomCentered(ElectricityConstants::BEAM_OFFSET_RANGE, ElectricityConstants::BEAM_VELOCITY_SCALE)
            },
            { 0, 0, 0 },
            { size * intensity, size * intensity },
            true,
            ElectricityConstants::BEAM_ANIMATION_SPEED,
            0.0f,
            0.0f
        );
    }
}

// ============================================================================
// ParticleStaffProjectile Implementation
// ============================================================================

void ParticleStaffProjectile::Initialize(ID3D11Device* device)
{
    D3D11_TEXTURE2D_DESC texture2d_desc;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;

    // 5×5のスタッフ弾スプライトシートを読み込み
    GpuResourceUtils::LoadTexture(device, StaffProjectileConstants::TEXTURE_PATH,
        shader_resource_view.GetAddressOf(),
        &texture2d_desc);

    // 5列5行のグリッドで初期化
    particles = std::make_unique<particleSystem>(device,
        shader_resource_view,
        StaffProjectileConstants::SPRITE_COLS,
        StaffProjectileConstants::SPRITE_ROWS,
        StaffProjectileConstants::MAX_PARTICLES);
}

void ParticleStaffProjectile::Update(float elapsedTime)
{
    if (particles) {
        // パーティクル更新
        particles->update(elapsedTime);
    }
}

void ParticleStaffProjectile::Render(const RenderContext& rc)
{
    // 描画無効または未初期化なら処理しない
    if (!shouldRender || !particles) return;

    // 加算ブレンドで発光表現
    const float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    rc.deviceContext->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Additive), blendFactor, 0xFFFFFFFF);
    rc.deviceContext->OMSetDepthStencilState(rc.renderState->GetDepthStencilState(DepthState::TestOnly), 0);
    rc.deviceContext->RSSetState(rc.renderState->GetRasterizerState(RasterizerState::SolidCullNone));

    // 描画実行
    particles->render(rc);
}

void ParticleStaffProjectile::SpawnStaffProjectile(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& direction, float speed, float projectileLifetime)
{
    if (!particles) return;

    float lifetime;
    float animationSpeed;
    {
        // 飛距離から寿命を計算
        lifetime = StaffProjectileConstants::MAX_DISTANCE / speed;

        // 寿命に応じてアニメーション速度を調整
        animationSpeed = StaffProjectileConstants::ANIMATION_BASE_SPEED / lifetime;
    }

    particles->set(
        0, // 開始フレーム
        lifetime, // 弾の寿命と一致させる
        position,
        { direction.x * speed, direction.y * speed, direction.z * speed },
        { 0, 0, 0 },
        { StaffProjectileConstants::SIZE, StaffProjectileConstants::SIZE },
        true, // スプライトアニメーション有効
        animationSpeed, // 寿命に合わせた速度
        0.0f,
        0.0f
    );
}