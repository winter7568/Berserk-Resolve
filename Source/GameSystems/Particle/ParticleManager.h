#pragma once
#include <memory>
#include "ParticleSprite.h"
#include "ParticleCombat.h"
#include "ParticleEffect.h"
#include "GameLib/RenderContext.h"

// パーティクルシステム全体を管理するクラス
class ParticleManager
{
public:
    static ParticleManager& Instance()
    {
        static ParticleManager instance;
        return instance;
    }

    // コピー禁止
    ParticleManager(const ParticleManager&) = delete;
    ParticleManager& operator=(const ParticleManager&) = delete;

    // 全パーティクルシステムの初期化
    void Initialize(ID3D11Device* device);

    // 全パーティクルシステムの更新
    void Update(float elapsedTime);

    // 全パーティクルシステムの描画
    void Render(const RenderContext& rc);

    // ========================================================================
    // ParticleSprite Methods
    // ========================================================================

    void SpawnBackgroundEnergyField(float elapsedTime);

    void SpawnPlayerBulletTrail(
        const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& direction,
        float lifetime
    );

    void SpawnSlashTrail(
        const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& direction,
        const DirectX::XMFLOAT3& orientation,
        float slashLength,
        float lifetime
    );

    void SpawnSlashEdgeGlow(
        const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& orientation,
        float slashLength
    );

    void SpawnStaffAuraParticles(
        const DirectX::XMFLOAT3& tip,
        const DirectX::XMFLOAT3& base,
        const DirectX::XMFLOAT3& dir,
        float elapsedTime
    );

    // ========================================================================
    // ParticleTeleport Methods
    // ========================================================================

    void SpawnTeleportCharge(const DirectX::XMFLOAT3& position);
    void SpawnTeleportFlash(const DirectX::XMFLOAT3& position);
    void SpawnTeleportArrival(const DirectX::XMFLOAT3& position);
    void SpawnFullSequenceForward(const DirectX::XMFLOAT3& position);
    void SpawnFullSequenceReverse(const DirectX::XMFLOAT3& position);

    // ========================================================================
    // ParticleElectricity Methods
    // ========================================================================

    void SpawnElectricityEffect(
        const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& direction,
        float intensity = 1.0f,
        float duration = 0.5f
    );

    void SpawnLightningImpact(
        const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& normal,
        float intensity = 1.0f
    );

    void SpawnLightningBeam(
        const DirectX::XMFLOAT3& startPos,
        const DirectX::XMFLOAT3& endPos,
        float intensity = 1.0f,
        float duration = 0.2f
    );

    // ========================================================================
    // ParticleStaffProjectile Methods
    // ========================================================================

    void SpawnStaffProjectile(
        const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& direction,
        float speed,
        float projectileLifetime = -1.0f
    );

    void StopStaffProjectileRendering();
    void StartStaffProjectileRendering();

    // ========================================================================
    // ParticleFlame Methods
    // ========================================================================

    bool SpawnSmokeAnimation(const DirectX::XMFLOAT3& position, float duration = 0.8f);
    bool IsSmokeComplete(const DirectX::XMFLOAT3& position) const;

private:
    ParticleManager() = default;
    ~ParticleManager() = default;

    std::unique_ptr<ParticleSprite> sprite;
    std::unique_ptr<ParticleTeleport> teleport;
    std::unique_ptr<ParticleElectricity> electricity;
    std::unique_ptr<ParticleStaffProjectile> staffProjectile;
    std::unique_ptr<ParticleFlame> flame;
};