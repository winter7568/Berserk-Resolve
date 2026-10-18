#include "Enemy.h"
#include "GameLib/GpuResourceUtils.h"
#include "StateMachine/EnemyStates/PositioningState.h"

// ============================================================================
// PRIVATE METHODS - Setup Helpers
// ============================================================================

void Enemy::LoadModels(ID3D11Device* device)
{
    model = std::make_unique<Model>(device, "Data/Model/Enemy/Enemy_2.fbx");
    sword.model = std::make_unique<Model>(device, "Data/Model/Sword/Sword.gltf");

    model->emissiveStrengthMultiplier = EnemyConstants::DEFAULT_EMISSIVE_STRENGTH;
    sword.model->emissiveStrengthMultiplier = EnemyConstants::SWORD_EMISSIVE_STRENGTH;
}

void Enemy::SetupDissolveEffect(ID3D11Device* device)
{
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> dissolveTexture;
    GpuResourceUtils::LoadTexture(device, "Data/Textures/NoiseTexture.png",
        dissolveTexture.GetAddressOf());

    model->EnableDissolve(true);
    sword.model->EnableDissolve(true);

    model->SetDissolveMask(dissolveTexture.Get());
    sword.model->SetDissolveMask(dissolveTexture.Get());

    // 最初は完全に透明（出現演出用）
    model->SetDissolveThreshold(1.0f);
    sword.model->SetDissolveThreshold(1.0f);

    // シネマティック出現状態の初期化
    cinematicSpawnActive = false;
    cinematicSpawnComplete = false;
}

void Enemy::SetupInitialTransform()
{
    position = EnemyConstants::INITIAL_POSITION;
    scale = EnemyConstants::INITIAL_SCALE;
    angle = EnemyConstants::INITIAL_ANGLE;
    velocity = EnemyConstants::INITIAL_VELOCITY;
}

void Enemy::SetupColliders()
{
    // 武器の当たり判定設定
    weaponCapsule.radius = EnemyConstants::WEAPON_CAPSULE_RADIUS;
    weaponCapsule.height = EnemyConstants::WEAPON_CAPSULE_HEIGHT;
    weaponCapsule.offset = EnemyConstants::WEAPON_CAPSULE_OFFSET;
    weaponCapsule.rotation = EnemyConstants::WEAPON_CAPSULE_ROTATION;

    // 本体の当たり判定設定
    capsuleParams.enabled = true;
    capsuleParams.radius = CharacterConstants::BODY_CAPSULE_RADIUS;
    capsuleParams.height = CharacterConstants::BODY_CAPSULE_HEIGHT;
    capsuleParams.offset = CharacterConstants::BODY_CAPSULE_OFFSET;
    capsuleParams.rotation = CharacterConstants::BODY_CAPSULE_ROTATION;
    capsuleParams.color = CharacterConstants::BODY_CAPSULE_COLOR;
}

void Enemy::SetupStateManager()
{
    // 初期ステート設定（位置取りAI）
    stateManager.PushState(&EnemyPositioningState::Instance());
}

void Enemy::SetupStats()
{
    health = EnemyConstants::MAX_HEALTH;
    weight = EnemyConstants::DEFAULT_WEIGHT;
    isGround = true;
    invincibleTimer = 0.0f;
}

void Enemy::SetupTrailSystem(ID3D11Device* device)
{
    TrailSystem::TrailConfig trailConfig;
    trailConfig.startOffset = swordTrailBaseOffset;
    trailConfig.endOffset = swordTrailTipOffset;
    trailConfig.maxLifetime = EnemyConstants::TRAIL_MAX_LIFETIME;
    trailConfig.maxSegments = CharacterConstants::TRAIL_MAX_SEGMENTS;
    trailConfig.normalMapPath = "Data/Textures/energy_Streak.png";
    trailConfig.maskPath = "Data/Textures/Trail.png";

    swordTrail.Initialize(device, trailConfig);
}

// ============================================================================
// PRIVATE METHODS - Reset Helpers
// ============================================================================

void Enemy::ResetTransform()
{
    position = EnemyConstants::INITIAL_POSITION;
    velocity = EnemyConstants::INITIAL_VELOCITY;
    angle = EnemyConstants::RESET_ANGLE;
}

void Enemy::ResetStats()
{
    health = EnemyConstants::MAX_HEALTH;
    invincibleTimer = 0.0f;
    isGround = true;
    isDead = false;
}

void Enemy::ResetState()
{
    stateManager.ClearStates();
}

void Enemy::ResetAttackState()
{
    currentAttackIndex = 0;
    currentAttackStateID = EnemyStateID::NONE;
    currentSlashIndex = 0;
}

void Enemy::ResetCombatFlags()
{
    isWeaponHitActive = false;
    isInCounterWindow = false;
}

void Enemy::ResetSpawnAndDownState()
{
    cinematicSpawnActive = false;
    cinematicSpawnComplete = false;
    hasBeenKnockedDown = false;
    bodyGlow = false;
}

// ============================================================================
// PUBLIC METHODS - Cinematic Spawn
// ============================================================================

void Enemy::StartCinematicSpawn()
{
    cinematicSpawnActive = true;
    cinematicSpawnComplete = false;
    cinematicSpawnTimer = 0.0f;

    // モデルを完全に消した状態にする
    model->SetDissolveThreshold(1.0f);
    sword.model->SetDissolveThreshold(1.0f);
}

void Enemy::UpdateCinematicSpawn(float elapsedTime)
{
    if (!cinematicSpawnActive || cinematicSpawnComplete) return;

    cinematicSpawnTimer += elapsedTime;

    if (cinematicSpawnTimer < EnemyConstants::SPAWN_START_DELAY) {
        return;
    }

    // 進行度（0〜1）
    float t = (cinematicSpawnTimer - EnemyConstants::SPAWN_START_DELAY) / EnemyConstants::SPAWN_DURATION;

    if (t < 1.0f) {
        // スムーズステップ関数の係数
        float smoothT = t * t * (CharacterConstants::SMOOTH_STEP_A - CharacterConstants::SMOOTH_STEP_B * t);

        // ディゾルブしきい値
        float dissolveThreshold = 1.0f - t;
        model->SetDissolveThreshold(dissolveThreshold);
        if (sword.model) sword.model->SetDissolveThreshold(dissolveThreshold);

        // 発光強度の計算（ランプアップ/ダウン）
        float glowIntensity;
        constexpr float GLOW_RAMP_FACTOR = 1.0f / EnemyConstants::SPAWN_GLOW_RAMP_UP_END;

        if (t < EnemyConstants::SPAWN_GLOW_RAMP_UP_END) {
            glowIntensity = t * GLOW_RAMP_FACTOR;
        }
        else if (t < EnemyConstants::SPAWN_GLOW_SUSTAIN_END) {
            glowIntensity = 1.0f;
        }
        else {
            glowIntensity = GLOW_RAMP_FACTOR * (1.0f - t);
        }

        // 点滅効果
        float pulse = sinf(cinematicSpawnTimer * EnemyConstants::SPAWN_PULSE_SPEED) *
            EnemyConstants::SPAWN_PULSE_STRENGTH + (1.0f - EnemyConstants::SPAWN_PULSE_STRENGTH);

        float finalIntensity = glowIntensity * EnemyConstants::SPAWN_BASE_INTENSITY * pulse;

        model->SetDissolveEdgeIntensity(finalIntensity);
        model->SetDissolveEdgeWidth(EnemyConstants::SPAWN_EDGE_WIDTH);

        if (sword.model) {
            sword.model->SetDissolveEdgeIntensity(finalIntensity * EnemyConstants::SPAWN_SWORD_INTENSITY_RATIO);
            sword.model->SetDissolveEdgeWidth(EnemyConstants::SPAWN_SWORD_EDGE_WIDTH * glowIntensity);
        }
    }
    else {
        // 完了
        model->SetDissolveThreshold(0.0f);
        model->SetDissolveEdgeIntensity(0.0f);
        model->SetDissolveEdgeWidth(0.0f);

        if (sword.model) {
            sword.model->SetDissolveThreshold(0.0f);
            sword.model->SetDissolveEdgeIntensity(0.0f);
            sword.model->SetDissolveEdgeWidth(0.0f);
        }

        cinematicSpawnComplete = true;
        cinematicSpawnActive = false;
    }
}