#include "ParticleManager.h"

void ParticleManager::Initialize(ID3D11Device* device)
{
    sprite = std::make_unique<ParticleSprite>();
    sprite->Initialize(device);

    teleport = std::make_unique<ParticleTeleport>();
    teleport->Initialize(device);

    electricity = std::make_unique<ParticleElectricity>();
    electricity->Initialize(device);

    staffProjectile = std::make_unique<ParticleStaffProjectile>();
    staffProjectile->Initialize(device);

    flame = std::make_unique<ParticleFlame>();
    flame->Initialize(device);
}

void ParticleManager::Update(float elapsedTime)
{
    if (sprite) sprite->Update(elapsedTime);
    if (teleport) teleport->Update(elapsedTime);
    if (electricity) electricity->Update(elapsedTime);
    if (staffProjectile) staffProjectile->Update(elapsedTime);
    if (flame) flame->Update(elapsedTime);
}

void ParticleManager::Render(const RenderContext& rc)
{
    if (sprite) sprite->Render(rc);
    if (teleport) teleport->Render(rc);
    if (electricity) electricity->Render(rc);
    if (staffProjectile) staffProjectile->Render(rc);
    if (flame) flame->Render(rc);
}

// ============================================================================
// ParticleSprite Methods
// ============================================================================

void ParticleManager::SpawnBackgroundEnergyField(float elapsedTime)
{
    if (sprite) sprite->SpawnBackgroundEnergyField(elapsedTime);
}

void ParticleManager::SpawnPlayerBulletTrail(
    const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT3& direction,
    float lifetime)
{
    if (sprite) sprite->SpawnPlayerBulletTrail(position, direction, lifetime);
}

void ParticleManager::SpawnSlashTrail(
    const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT3& direction,
    const DirectX::XMFLOAT3& orientation,
    float slashLength,
    float lifetime)
{
    if (sprite) sprite->SpawnSlashTrail(position, direction, orientation, slashLength, lifetime);
}

void ParticleManager::SpawnSlashEdgeGlow(
    const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT3& orientation,
    float slashLength)
{
    if (sprite) sprite->SpawnSlashEdgeGlow(position, orientation, slashLength);
}

void ParticleManager::SpawnStaffAuraParticles(const DirectX::XMFLOAT3& tip, const DirectX::XMFLOAT3& base, const DirectX::XMFLOAT3& dir, float elapsedTime)
{
    if (sprite) sprite->SpawnStaffAuraParticles(tip, base, dir, elapsedTime);
}

// ============================================================================
// ParticleTeleport Methods
// ============================================================================

void ParticleManager::SpawnTeleportCharge(const DirectX::XMFLOAT3& position)
{
    if (teleport) teleport->SpawnTeleportCharge(position);
}

void ParticleManager::SpawnTeleportFlash(const DirectX::XMFLOAT3& position)
{
    if (teleport) teleport->SpawnTeleportFlash(position);
}

void ParticleManager::SpawnTeleportArrival(const DirectX::XMFLOAT3& position)
{
    if (teleport) teleport->SpawnTeleportArrival(position);
}

void ParticleManager::SpawnFullSequenceForward(const DirectX::XMFLOAT3& position)
{
    if (teleport) teleport->SpawnFullSequenceForward(position);
}

void ParticleManager::SpawnFullSequenceReverse(const DirectX::XMFLOAT3& position)
{
    if (teleport) teleport->SpawnFullSequenceReverse(position);
}

// ============================================================================
// ParticleElectricity Methods
// ============================================================================

void ParticleManager::SpawnElectricityEffect(
    const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT3& direction,
    float intensity,
    float duration)
{
    if (electricity) electricity->SpawnElectricityEffect(position, direction, intensity, duration);
}

void ParticleManager::SpawnLightningImpact(
    const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT3& normal,
    float intensity)
{
    if (electricity) electricity->SpawnLightningImpact(position, normal, intensity);
}

void ParticleManager::SpawnLightningBeam(
    const DirectX::XMFLOAT3& startPos,
    const DirectX::XMFLOAT3& endPos,
    float intensity,
    float duration)
{
    if (electricity) electricity->SpawnLightningBeam(startPos, endPos, intensity, duration);
}

// ============================================================================
// ParticleStaffProjectile Methods
// ============================================================================

void ParticleManager::SpawnStaffProjectile(
    const DirectX::XMFLOAT3& position,
    const DirectX::XMFLOAT3& direction,
    float speed,
    float projectileLifetime)
{
    if (staffProjectile) staffProjectile->SpawnStaffProjectile(position, direction, speed, projectileLifetime);
}

void ParticleManager::StopStaffProjectileRendering()
{
    if (staffProjectile) staffProjectile->StopRendering();
}

void ParticleManager::StartStaffProjectileRendering()
{
    if (staffProjectile) staffProjectile->StartRendering();
}

// ============================================================================
// ParticleFlame Methods
// ============================================================================

bool ParticleManager::SpawnSmokeAnimation(const DirectX::XMFLOAT3& position, float duration)
{
    if (flame) return flame->SpawnSmokeAnimation(position, duration);
    return false;
}

bool ParticleManager::IsSmokeComplete(const DirectX::XMFLOAT3& position) const
{
    if (flame) return flame->IsSmokeComplete(position);
    return true;
}