#include "StarfeAttackState.h"
#include "GameSystems/Particle/ParticleManager.h"
#include "GameSystems/Projectile/ProjectileManager.h"
#include "GameObjects/Player/Player.h"

void EnemyStarfeAttackState::Enter(Enemy& enemy)
{
    currentPhase = StarfePhase::SIDESTEP_START;
    phaseTimer = 0.0f;
    projectileTimer = 0.0f;
    projectilesFired = 0;

    // 左へのサイドステップ開始アニメーションを再生する
    enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::WALK_START_F_L_90), false);
}

void EnemyStarfeAttackState::Update(Enemy& enemy, float elapsedTime)
{
    phaseTimer += elapsedTime;

    // 全フェーズ中プレイヤーの方向を向く
    enemy.SmoothlyTrackPlayer(elapsedTime, EnemyConstants::TURN_SPEED_NORMAL);

    switch (currentPhase)
    {
    case StarfePhase::SIDESTEP_START:
        UpdateSideStepStart(enemy);
        break;

    case StarfePhase::SIDESTEP_LOOP:
        UpdateSideStepLoop(enemy);
        break;

    case StarfePhase::PROJECTILE_FIRE:
        UpdateProjectileFire(enemy, elapsedTime);
        break;

    case StarfePhase::SIDESTEP_STOP:
        UpdateSideStepStop(enemy);
        break;
    }
}

void EnemyStarfeAttackState::Exit(Enemy& enemy)
{
    // 後片付け
    pendingProjectiles.clear();
    projectilesFired = 0;
    phaseTimer = 0.0f;
    projectileTimer = 0.0f;
    enemy.StopEnemySound(EnemySound::PROJECTILE);
}

void EnemyStarfeAttackState::UpdateSideStepStart(Enemy& enemy)
{
    const auto& anim = enemy.model->GetAnimations().at(enemy.animationIndex);

    if (phaseTimer >= anim.secondsLength)
    {
        currentPhase = StarfePhase::SIDESTEP_LOOP;
        phaseTimer = 0.0f;

        enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::WALK_LOOP_F_L_90), true);
    }
}

void EnemyStarfeAttackState::UpdateSideStepLoop(Enemy& enemy)
{
    // ループから即座に射撃フェーズへ移行する
    currentPhase = StarfePhase::PROJECTILE_FIRE;
    projectileTimer = 0.0f;
    projectilesFired = 0;
}

void EnemyStarfeAttackState::UpdateProjectileFire(Enemy& enemy, float elapsedTime)
{
    projectileTimer += elapsedTime;

    // 1.2秒間隔で3発の弾を発射する
    if (projectileTimer >= EnemyConstants::STARFE_PROJECTILE_DELAY &&
        projectilesFired < EnemyConstants::STARFE_TOTAL_PROJECTILES)
    {
        FireProjectileWithSmoke(enemy);
    }

    // 保留中の弾を更新する
    UpdatePendingProjectiles(enemy, elapsedTime);

    // 全弾発射したら停止フェーズへ移行する
    if (AllProjectilesFired())
    {
        currentPhase = StarfePhase::SIDESTEP_STOP;
        phaseTimer = 0.0f;

        enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::WALK_STOP_F_L_90), false);
    }
}

void EnemyStarfeAttackState::UpdateSideStepStop(Enemy& enemy) const
{
    const auto& anim = enemy.model->GetAnimations().at(enemy.animationIndex);

    if (phaseTimer >= anim.secondsLength)
    {
        enemy.GetStateManager().PopState();
        enemy.GetStateManager().PushState(&EnemyIdleState::Instance());
    }
}

void EnemyStarfeAttackState::FireProjectileWithSmoke(Enemy& enemy)
{
    // 煙エフェクトを開始する
    DirectX::XMFLOAT3 spawnPos = CalculateSpawnPosition(enemy, projectilesFired);
    ParticleManager::Instance().SpawnSmokeAnimation(spawnPos, EnemyConstants::STARFE_SMOKE_SCALE);
    enemy.PlayEnemySound(EnemySound::PROJECTILE, EnemyConstants::DEFAULT_SOUND_VOLUME);

    // 煙の後に弾を発射するため保留する
    pendingProjectiles.push_back({ spawnPos, projectilesFired, 0.0f, false });

    projectilesFired++;
    projectileTimer = 0.0f;
}

void EnemyStarfeAttackState::UpdatePendingProjectiles(Enemy& enemy, float elapsedTime)
{
    for (auto& pending : pendingProjectiles)
    {
        pending.delayTimer += elapsedTime;

        // 1秒後に実際の弾を発射する
        if (pending.delayTimer >= EnemyConstants::STARFE_SMOKE_DURATION && !pending.fired)
        {
            FireProjectileNow(enemy, pending.spawnPos, pending.index);
            pending.fired = true;
        }
    }

    // 発射済みの弾を削除する
    CleanupFiredProjectiles();
}

void EnemyStarfeAttackState::CleanupFiredProjectiles()
{
    pendingProjectiles.erase(
        std::remove_if(pendingProjectiles.begin(), pendingProjectiles.end(),
            [](const PendingProjectile& p) { return p.fired; }),
        pendingProjectiles.end()
    );
}

bool EnemyStarfeAttackState::AllProjectilesFired() const
{
    return projectilesFired >= EnemyConstants::STARFE_TOTAL_PROJECTILES &&
        pendingProjectiles.empty();
}

DirectX::XMFLOAT3 EnemyStarfeAttackState::CalculateSpawnPosition(Enemy& enemy, int index)
{
    DirectX::XMFLOAT3 enemyPos = enemy.GetPosition();

    return {
        enemyPos.x,
        enemyPos.y + EnemyConstants::STARFE_SPAWN_HEIGHT,
        enemyPos.z
    };
}

void EnemyStarfeAttackState::FireProjectileNow(Enemy& enemy, const DirectX::XMFLOAT3& spawnPos, int index)
{
    DirectX::XMFLOAT3 playerPos = Player::Instance().GetPosition();

    // プレイヤーへの方向ベクトルを計算する
    DirectX::XMFLOAT3 toPlayer = {
        playerPos.x - spawnPos.x,
        (playerPos.y + EnemyConstants::STARFE_TARGET_HEIGHT_OFFSET) - spawnPos.y,
        playerPos.z - spawnPos.z
    };

    float length = sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y + toPlayer.z * toPlayer.z);
    if (length > 0.001f)
    {
        toPlayer.x /= length;
        toPlayer.y /= length;
        toPlayer.z /= length;
    }

    auto& projectileManager = ProjectileManager::Instance();
    projectileManager.CreateProjectile("enemy", "enemy_orb", spawnPos, toPlayer);
}