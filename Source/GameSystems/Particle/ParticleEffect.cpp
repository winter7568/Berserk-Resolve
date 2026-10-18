#include "ParticleEffect.h"
#include "GameLib/GpuResourceUtils.h"

// ============================================================================
// ParticleTeleport Implementation
// ============================================================================

void ParticleTeleport::Initialize(ID3D11Device* device)
{
    D3D11_TEXTURE2D_DESC texture2d_desc;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;

    // テレポート用スプライトシートを読み込み
    GpuResourceUtils::LoadTexture(device, TeleportConfig::TEXTURE_PATH,
        shader_resource_view.GetAddressOf(),
        &texture2d_desc);

    // 四掛ける四のスプライトシートで初期化
    particles = std::make_unique<particleSystem>(device,
        shader_resource_view,
        TeleportConfig::SPRITE_COLS,
        TeleportConfig::SPRITE_ROWS,
        TeleportConfig::MAX_PARTICLES);
}

void ParticleTeleport::Update(float elapsedTime)
{
    // パーティクル更新
    if (particles)
    {
        particles->update(elapsedTime);
    }
}

void ParticleTeleport::Render(const RenderContext& rc)
{
    if (particles)
    {
        // 発光表現のため加算合成を使用
        const float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        rc.deviceContext->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Additive), blendFactor, 0xFFFFFFFF);

        // 深度はテストのみ
        rc.deviceContext->OMSetDepthStencilState(rc.renderState->GetDepthStencilState(DepthState::TestOnly), 0);

        // カリングなしで描画
        rc.deviceContext->RSSetState(rc.renderState->GetRasterizerState(RasterizerState::SolidCullNone));

        // パーティクル描画
        particles->render(rc);
    }
}

void ParticleTeleport::SpawnTeleportCharge(const DirectX::XMFLOAT3& position)
{
    if (!particles) return;

    // チャージフェーズ用の単一パーティクル フレーム0から3 エネルギー収束
    particles->set(
        AnimationFrames::CHARGE_START,          // フレーム0から開始
        TeleportConfig::CHARGE_LIFETIME,        // フレーム0から3を再生するための時間
        position,                               // 体の中心付近に表示
        { 0, 0, 0 },                            // 移動なし その場に固定
        { 0, 0, 0 },                            // 加速度なし
        { TeleportConfig::CHARGE_SIZE, TeleportConfig::CHARGE_SIZE },  // 視認性を上げるため大きめサイズ
        true,                                   // スプライトアニメーションを使用
        TeleportConfig::CHARGE_ANIM_SPEED,      // フレーム0から3を再生する速度
        0.0f,                                   // 回転なし
        0.0f                                    // 回転速度なし
    );
}

void ParticleTeleport::SpawnTeleportFlash(const DirectX::XMFLOAT3& position)
{
    if (!particles) return;

    // フラッシュフェーズ用の単一パーティクル フレーム4から7 強い発光表現
    particles->set(
        AnimationFrames::FLASH_START,           // フレーム4から開始
        TeleportConfig::FLASH_LIFETIME,         // 短時間でフラッシュを再生 フレーム4から7
        position,                               // 指定位置を中心に表示
        { 0, 0, 0 },                            // 移動なし
        { 0, 0, 0 },                            // 加速度なし
        { TeleportConfig::FLASH_SIZE, TeleportConfig::FLASH_SIZE },  // フラッシュ用に大きめサイズ
        true,                                   // スプライトアニメーションを使用
        TeleportConfig::FLASH_ANIM_SPEED,       // 高速でフレーム4から7を再生
        0.0f,                                   // 回転なし
        0.0f                                    // 回転速度なし
    );
}

void ParticleTeleport::SpawnTeleportArrival(const DirectX::XMFLOAT3& position)
{
    if (!particles) return;

    // 到着フェーズ用の単一パーティクル フレーム8から15 エネルギー拡散
    particles->set(
        AnimationFrames::ARRIVAL_START,         // フレーム8から開始
        TeleportConfig::ARRIVAL_LIFETIME,       // フレーム8から15を再生するための時間
        position,                               // 体の中心付近に表示
        { 0, 0, 0 },                            // 移動なし
        { 0, 0, 0 },                            // 加速度なし
        { TeleportConfig::ARRIVAL_SIZE, TeleportConfig::ARRIVAL_SIZE },  // 拡散表現のため大きめサイズ
        true,                                   // スプライトアニメーションを使用
        TeleportConfig::ARRIVAL_ANIM_SPEED,     // フレーム8から15を再生する速度
        0.0f,                                   // 回転なし
        0.0f                                    // 回転速度なし
    );
}

void ParticleTeleport::SpawnFullSequenceForward(const DirectX::XMFLOAT3& position)
{
    if (!particles) return;

    // 全フレームを順方向で一括再生 テレポート全体の流れを表現
    particles->set(
        0,                                      // フレーム0から開始
        TeleportConfig::FORWARD_LIFETIME,       // アニメーション全体の再生時間
        position,
        { 0, 0, 0 },
        { 0, 0, 0 },
        { TeleportConfig::FORWARD_SIZE, TeleportConfig::FORWARD_SIZE },
        true,
        TeleportConfig::FORWARD_ANIM_SPEED,     // 全16フレームを短時間で再生
        0.0f,
        0.0f
    );
}

void ParticleTeleport::SpawnFullSequenceReverse(const DirectX::XMFLOAT3& position)
{
    if (!particles) return;

    // 途中フレームから開始して逆方向的な演出を作る
    particles->set(
        TeleportConfig::REVERSE_START_FRAME,    // フレーム8から開始 拡散フェーズからスタート
        TeleportConfig::REVERSE_LIFETIME,
        position,
        { 0, 0, 0 },
        { 0, 0, 0 },
        { TeleportConfig::REVERSE_SIZE, TeleportConfig::REVERSE_SIZE },
        true,
        TeleportConfig::REVERSE_ANIM_SPEED,     // フレーム8から15を短時間で再生
        0.0f,
        0.0f
    );
}

// ============================================================================
// ParticleFlame Implementation
// ============================================================================

void ParticleFlame::Initialize(ID3D11Device* device)
{
    D3D11_TEXTURE2D_DESC texture2d_desc;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;

    // 煙テクスチャを読み込み
    GpuResourceUtils::LoadTexture(device, FlameConfig::TEXTURE_PATH,
        shader_resource_view.GetAddressOf(),
        &texture2d_desc);

    // 16列5行のグリッドで初期化
    particles = std::make_unique<particleSystem>(device,
        shader_resource_view,
        FlameConfig::SPRITE_COLS,
        FlameConfig::SPRITE_ROWS,
        FlameConfig::MAX_PARTICLES);
}

void ParticleFlame::Update(float elapsedTime)
{
    if (particles) {
        // パーティクル更新
        particles->update(elapsedTime);
    }

    // アクティブな煙アニメーションの更新
    for (auto& anim : activeAnimations) {
        if (anim.isActive) {
            anim.elapsedTime += elapsedTime;
            if (anim.elapsedTime >= anim.duration) {
                anim.isActive = false;
            }
        }
    }

    // 完了したアニメーションを削除
    activeAnimations.erase(
        std::remove_if(activeAnimations.begin(), activeAnimations.end(),
            [](const SmokeAnimation& anim) { return !anim.isActive; }),
        activeAnimations.end()
    );
}

void ParticleFlame::Render(const RenderContext& rc)
{
    // 描画無効または未初期化なら処理しない
    if (!shouldRender || !particles) return;

    // 加算ブレンド設定
    const float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    rc.deviceContext->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Additive), blendFactor, 0xFFFFFFFF);
    rc.deviceContext->OMSetDepthStencilState(rc.renderState->GetDepthStencilState(DepthState::TestOnly), 0);
    rc.deviceContext->RSSetState(rc.renderState->GetRasterizerState(RasterizerState::SolidCullNone));

    // 描画
    particles->render(rc);
}

bool ParticleFlame::SpawnSmokeAnimation(const DirectX::XMFLOAT3& position, float duration)
{
    if (!particles) return false;

    // アクティブな煙アニメーションを登録
    activeAnimations.push_back({ position, 0.0f, duration, true });

    // 指定時間で全フレーム再生するための速度
    float animationSpeed = FlameConfig::ANIMATION_BASE_SPEED / duration;

    // 開始位置を少し上にオフセット
    DirectX::XMFLOAT3 startPos = {
        position.x + FlameConfig::SMOKE_OFFSET_X,
        position.y + FlameConfig::SMOKE_OFFSET_Y,
        position.z
    };

    particles->set(
        0,                                      // 開始フレーム
        duration,                               // 再生時間
        startPos,
        { 0, 0.0f, 0 },
        { 0, 0.0f, 0 },
        { FlameConfig::SMOKE_SIZE, FlameConfig::SMOKE_SIZE },
        true,                                   // アニメーション有効
        animationSpeed,                         // フレーム再生速度
        0.0f,
        0.0f
    );

    return true;
}

bool ParticleFlame::IsSmokeComplete(const DirectX::XMFLOAT3& position) const
{
    // 指定位置にアクティブな煙があるか確認
    for (const auto& anim : activeAnimations) {
        if (anim.isActive) {
            float dx = anim.position.x - position.x;
            float dy = anim.position.y - position.y;
            float dz = anim.position.z - position.z;
            float distanceSq = dx * dx + dy * dy + dz * dz;

            // 一定距離以内ならまだ再生中
            if (distanceSq < FlameConfig::COMPLETE_DISTANCE) {
                return false;
            }
        }
    }

    // 該当するアニメーションが無ければ完了
    return true;
}