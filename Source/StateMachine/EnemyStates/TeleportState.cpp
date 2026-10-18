#include "TeleportState.h"
#include "GameSystems/Time/TimeManager.h"
#include "GameSystems/Particle/ParticleManager.h"
#include "GameLib/TransformUtils.h"
#include "GameObjects/Player/Player.h"
#include "StateMachine/EnemyStates/AttackStates/ComboAttackState.h"
#include "StateMachine/EnemyStates/AttackStates/StarfeAttackState.h"
#include "StateMachine/EnemyStates/AttackStates/SlashAttackState.h"

void EnemyTeleportState::Enter(Enemy & enemy)
{
    // 時間スケール変更
    TimeManager::Instance().SetTimeScale(EnemyConstants::TELEPORT_TIME_SCALE, EnemyConstants::TELEPORT_TIME_SCALE_DURATION);

    currentPhase = TeleportPhase::JUMP_START;
    phaseTimer = 0.0f;

    // テレポート先を決める
    CalculateTeleportDestination();

    teleportStartPos = { enemy.position.x, enemy.position.y + EnemyConstants::TELEPORT_Y_OFFSET, enemy.position.z };
    teleportEndPos = { teleportDestination.x, teleportDestination.y + EnemyConstants::TELEPORT_Y_OFFSET, teleportDestination.z };

    enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::JUMP_START), false);
}

void EnemyTeleportState::Update(Enemy& enemy, float elapsedTime)
{
    phaseTimer += elapsedTime;

    switch (currentPhase) {
    case TeleportPhase::JUMP_START:
    {
        const auto& anim = enemy.model->GetAnimations().at(enemy.animationIndex);

        if (phaseTimer >= anim.secondsLength) {
            currentPhase = TeleportPhase::INVISIBLE_LOOP;
            phaseTimer = 0.0f;

            enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::JUMP_LOOP), true);
            enemy.position = teleportDestination;

            // プレイヤーの方向を見る
            FacePlayer(enemy);

            enemy.PlayEnemySound(EnemySound::TELEPORT, EnemyConstants::TELEPORT_SOUND_VOLUME);
            ParticleManager::Instance().SpawnFullSequenceForward(teleportStartPos);
            ParticleManager::Instance().SpawnFullSequenceReverse(teleportEndPos);

            // 非表示にする
            enemy.SetVisibility(false);
        }
    }
    break;

    case TeleportPhase::INVISIBLE_LOOP:
    {
        const auto& loopAnim = enemy.model->GetAnimations().at(enemy.animationIndex);

        if (phaseTimer >= loopAnim.secondsLength) {
            currentPhase = TeleportPhase::JUMP_LAND;
            phaseTimer = 0.0f;

            enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::JUMP_STOP), false);
            enemy.SetVisibility(true);
        }
    }
    break;

    case TeleportPhase::JUMP_LAND:
    {
        const auto& anim = enemy.model->GetAnimations().at(enemy.animationIndex);

        if (phaseTimer >= anim.secondsLength) {
            enemy.GetStateManager().PopState();

            float distance = enemy.CalculateDistanceToPlayer();

            if (isRangedTeleport) {
                enemy.GetStateManager().PushState(&EnemyStarfeAttackState::Instance());
            }
            else if (distance <= EnemyConstants::TELEPORT_COMBO_ATTACK_DISTANCE) {
                enemy.GetStateManager().PushState(&EnemyComboAttackState::Instance());
            }
            else {
                // 攻撃するか判定
                bool shouldAttack = (rand() % CharacterConstants::PERCENTAGE_BASE) < EnemyConstants::TELEPORT_RANGED_BEHAVIOR_CHANCE;

                if (shouldAttack) {
                    enemy.GetStateManager().PushState(&EnemyAttackState::Instance());
                }
                else {
                    enemy.GetStateManager().PushState(&EnemyIdleState::Instance());
                }
            }

            isRangedTeleport = false;

            // 時間スケール戻す
            TimeManager::Instance().ResetTimeScale(EnemyConstants::TELEPORT_TIME_RESET_DURATION);
        }
    }
    break;
    }
}

void EnemyTeleportState::Exit(Enemy& enemy)
{
    enemy.StopEnemySound(EnemySound::TELEPORT);
    enemy.SetVisibility(true);

    isRangedTeleport = false;
}

void EnemyTeleportState::CalculateTeleportDestination()
{
    DirectX::XMFLOAT3 playerPos = Player::Instance().GetPosition();

    int behavior = TransformUtils::RandomInt(0, EnemyConstants::TELEPORT_BEHAVIOR_COUNT - 1);

    float angle = TransformUtils::RandomInt(0, EnemyConstants::TELEPORT_ANGLE_FULL_CIRCLE - 1) * CharacterConstants::RADIANS_PER_DEGREE;

    switch (behavior) {
    case 0:
    {
        float range = EnemyConstants::TELEPORT_CLOSE_RANGE_MAX - EnemyConstants::TELEPORT_CLOSE_RANGE_MIN;
        float closeDistance = EnemyConstants::TELEPORT_CLOSE_RANGE_MIN + TransformUtils::RandomInt(0, (int)range);
        teleportDestination.x = playerPos.x + cosf(angle) * closeDistance;
        teleportDestination.z = playerPos.z + sinf(angle) * closeDistance;
        isRangedTeleport = false;
    }
    break;

    case 1:
    {
        float range = EnemyConstants::TELEPORT_FAR_RANGE_MAX - EnemyConstants::TELEPORT_FAR_RANGE_MIN;
        float farDistance = EnemyConstants::TELEPORT_FAR_RANGE_MIN + TransformUtils::RandomInt(0, (int)range);
        teleportDestination.x = playerPos.x + cosf(angle) * farDistance;
        teleportDestination.z = playerPos.z + sinf(angle) * farDistance;
        isRangedTeleport = true;
    }
    break;
    }

    teleportDestination.y = 0.0f;
}

void EnemyTeleportState::FacePlayer(Enemy& enemy)
{

    DirectX::XMFLOAT2 dir = enemy.GetDirectionToPlayer();

    if (dir.x != 0.0f || dir.y != 0.0f)
    {
        float targetYaw = atan2f(dir.x, dir.y);
        enemy.angle.y = targetYaw;
    }
}