#include "MoveStates.h"
#include "TeleportState.h"
#include "GameLib/TransformUtils.h"
#include "PositioningState.h"
#include "GameObjects/Player/Player.h"
#include "StateMachine/EnemyStates/AttackStates/ComboAttackState.h"

float EnemyBlockState::blockCooldownTimer = 0.0f;

void EnemyIdleState::Enter(Enemy& enemy) {
    enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::IDLE_COMBAT), true);

    // アイドル時間をリセット
    idleTimer = 0.0f;
}

void EnemyIdleState::Update(Enemy& enemy, float elapsedTime) {
    idleTimer += elapsedTime;

    // ガードするべきならすぐ切り替え
    if (EnemyBlockState::ShouldBlockNow(enemy, elapsedTime)) {
        enemy.GetStateManager().PopState();
        enemy.GetStateManager().PushState(&EnemyBlockState::Instance());
        return;
    }

    float distance = enemy.CalculateDistanceToPlayer();

    // 近い距離ならダッシュするか判断
    if (distance <= EnemyConstants::BACK_DASH_DISTANCE) {
        if (enemy.ShouldDash(distance)) {
            enemy.GetStateManager().PopState();
            enemy.GetStateManager().PushState(&EnemyDashState::Instance());
            return;
        }
    }

    // 長く止まりすぎないようにする
    if (idleTimer > EnemyConstants::MAX_IDLE_TIME) {
        enemy.GetStateManager().PopState();
        return;
    }

    // プレイヤーが遠いならすぐ移動に戻る
    if (distance > EnemyConstants::WALK_START_DISTANCE) {
        enemy.GetStateManager().PopState();
        return;
    }

    // 攻撃距離に入ったらアイドルをやめる
    if (distance <= EnemyConstants::ATTACK_RANGE) {
        enemy.GetStateManager().PopState();
        return;
    }
}

void EnemyIdleState::Exit(Enemy& enemy) {
    // 特に何もしない
}

void EnemyWalkState::Enter(Enemy& enemy) {
    currentPhase = WalkPhase::Start;
    phaseTimer = 0.0f;

    // 歩き始めのアニメーション
    int animIndex = enemy.GetAnimIndex(EnemyAnim::WALK_START_F);
    enemy.StartAnimation(animIndex, false);
    phaseDuration = enemy.model->GetAnimations().at(animIndex).secondsLength;
}

void EnemyWalkState::Update(Enemy& enemy, float elapsedTime) {
    // ガードが必要なら優先
    if (EnemyBlockState::ShouldBlockNow(enemy, elapsedTime)) {
        enemy.GetStateManager().PopState();
        enemy.GetStateManager().PushState(&EnemyBlockState::Instance());
        return;
    }

    phaseTimer += elapsedTime;

    DirectX::XMFLOAT3 targetPos = Player::Instance().GetPosition();
    DirectX::XMFLOAT3 enemyPos = enemy.GetPosition();

    float dirX = targetPos.x - enemyPos.x;
    float dirZ = targetPos.z - enemyPos.z;
    float distance = std::sqrt(dirX * dirX + dirZ * dirZ);

    // プレイヤーの方向を向く
    if (distance > 0.01f) {
        dirX /= distance;
        dirZ /= distance;
        enemy.Turn(elapsedTime, dirX, dirZ, EnemyConstants::TURN_SPEED_NORMAL);
    }

    switch (currentPhase) {
    case WalkPhase::Start:
        // 開始アニメーションが終わったらループへ
        if (phaseTimer >= phaseDuration) {
            int loopAnimIndex = enemy.GetAnimIndex(EnemyAnim::WALK_LOOP_F);
            enemy.StartAnimation(loopAnimIndex, true);
            phaseDuration = enemy.model->GetAnimations().at(loopAnimIndex).secondsLength;
            phaseTimer = 0.0f;
            currentPhase = WalkPhase::Loop;
        }
        break;

    case WalkPhase::Loop: {
        const float animLength = enemy.model->GetAnimations().at(enemy.animationIndex).secondsLength;

        // 近づいたらすぐ止まる
        if (distance <= EnemyConstants::WALK_STOP_DISTANCE) {
            int stopAnimIndex = enemy.GetAnimIndex(EnemyAnim::WALK_STOP_F);
            enemy.StartAnimation(stopAnimIndex, false);
            phaseDuration = enemy.model->GetAnimations().at(stopAnimIndex).secondsLength;
            phaseTimer = 0.0f;
            currentPhase = WalkPhase::Stop;
        }
        // まだ遠いなら歩きを続ける
        else if (phaseTimer >= animLength) {
            phaseTimer -= animLength;
        }
        break;
    }

    case WalkPhase::Stop:
        // 停止アニメーションが終わったら終了
        if (phaseTimer >= phaseDuration) {
            enemy.GetStateManager().PopState();
            enemy.GetStateManager().PushState(&EnemyIdleState::Instance());
        }
        break;
    }
}

void EnemyWalkState::Exit(Enemy& enemy) {
    // タイマーをリセット
    phaseTimer = 0.0f;
}

void EnemyDashState::Enter(Enemy& enemy) {
    // 後ろダッシュのアニメーションを使う
    enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::DODGE_B), false);
}

void EnemyDashState::Update(Enemy& enemy, float elapsedTime) {
    // ガードが必要なら優先
    if (EnemyBlockState::ShouldBlockNow(enemy, elapsedTime)) {
        enemy.GetStateManager().PopState();
        enemy.GetStateManager().PushState(&EnemyBlockState::Instance());
        return;
    }

    const Model::Animation& currentAnim = enemy.model->GetAnimations().at(enemy.animationIndex);

    // アニメーションが終わったら終了
    if (enemy.animationSeconds >= currentAnim.secondsLength) {
        enemy.GetStateManager().PopState();
        return;
    }
}

void EnemyDashState::Exit(Enemy& enemy) {
    // クリーンアップは不要
}

void EnemyBlockState::Enter(Enemy& enemy) {
    currentPhase = BlockPhase::START;
    phaseTimer = 0.0f;
    currentStamina = params.maxStamina;

    // ガード開始アニメーション
    enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::BLOCK_START), false);
}

void EnemyBlockState::Update(Enemy& enemy, float elapsedTime) {
    Player& player = Player::Instance();
    DirectX::XMFLOAT3 playerPos = player.GetPosition();
    DirectX::XMFLOAT3 enemyPos = enemy.GetPosition();

    float distance = enemy.CalculateDistanceToPlayer();
    // プレイヤーが遠すぎたらガードをやめる
    if (distance > EnemyConstants::MAX_BLOCK_DISTANCE && currentPhase != BlockPhase::BREAK) {
        Exit(enemy);
        enemy.GetStateManager().PopState();
        enemy.GetStateManager().PushState(&EnemyIdleState::Instance());
        return;
    }

    bool isPlayerAttacking = player.IsAttacking();
    static float noAttackTimer = 0.0f;

    // 攻撃していない時間をカウント
    if (!isPlayerAttacking) {
        noAttackTimer += elapsedTime;

        // 一定時間攻撃がなければガード終了
        if (noAttackTimer > EnemyConstants::BLOCK_NO_ATTACK_TIMEOUT && currentPhase != BlockPhase::BREAK) {
            enemy.GetStateManager().PopState();
            enemy.GetStateManager().PushState(&EnemyIdleState::Instance());
            return;
        }
    }
    else {
        // 攻撃中はリセット
        noAttackTimer = 0.0f;
    }

    // プレイヤーの方向を向く
    DirectX::XMFLOAT3 toPlayer = {
        playerPos.x - enemyPos.x,
        0.0f,
        playerPos.z - enemyPos.z
    };

    float length = sqrt(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);
    if (length > 0.001f) {
        toPlayer.x /= length;
        toPlayer.z /= length;
    }

    enemy.Turn(elapsedTime, toPlayer.x, toPlayer.z, EnemyConstants::TURN_SPEED_FAST);

    phaseTimer += elapsedTime;

    // ガードの状態更新
    UpdateBlockPhase(enemy, elapsedTime);
}

void EnemyBlockState::UpdateBlockPhase(Enemy& enemy, float elapsedTime) {
    switch (currentPhase) {
    case BlockPhase::START:
        // 構え終わったら維持状態へ
        if (phaseTimer >= params.startDuration) {
            ChangePhase(enemy, BlockPhase::HOLD);
        }
        break;

    case BlockPhase::HOLD:
        // スタミナを少し回復する
        currentStamina = min(params.maxStamina,
            currentStamina + params.staminaRegenRate * elapsedTime);

        // 長く構えすぎたら終了へ
        if (phaseTimer > EnemyConstants::BLOCK_HOLD_MAX_TIME) {
            ChangePhase(enemy, BlockPhase::RECOVERY);
        }
        break;

    case BlockPhase::HIT:
        if (phaseTimer >= params.hitDuration) {
            // スタミナが残っていれば戻る
            if (currentStamina >= params.breakThreshold) {
                ChangePhase(enemy, BlockPhase::HOLD);
            }
            else {
                OnBlockBreak(enemy);
            }
        }
        break;

    case BlockPhase::RECOVERY:
        if (phaseTimer >= params.recoveryDuration) {
            enemy.GetStateManager().PopState();
            enemy.GetStateManager().PushState(&EnemyIdleState::Instance());
        }
        break;

    case BlockPhase::BREAK:
        if (phaseTimer >= params.breakDuration) {
            enemy.GetStateManager().PopState();
            enemy.GetStateManager().PushState(&EnemyIdleState::Instance());
        }
        break;
    }
}

void EnemyBlockState::ChangePhase(Enemy& enemy, BlockPhase newPhase) {
    currentPhase = newPhase;
    phaseTimer = 0.0f;

    switch (newPhase) {
    case BlockPhase::HOLD:
        enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::BLOCK_LOOP), true);
        break;

    case BlockPhase::HIT:
        enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::BLOCK_HIT), false);
        break;

    case BlockPhase::RECOVERY:
        enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::BLOCK_END), false);
        break;

    case BlockPhase::BREAK:
        enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::BLOCK_HIT_BREAK), false);
        break;
    }

    currentPhase = newPhase;
}

void EnemyBlockState::OnBlockSuccess(Enemy& enemy, float attackStrength) {
    if (currentPhase == BlockPhase::BREAK || currentPhase == BlockPhase::RECOVERY) {
        return;
    }

    // 攻撃の強さでスタミナを減らす
    float staminaDrain = (attackStrength > EnemyConstants::HEAVY_ATTACK_STRENGTH_THRESHOLD) ?
        params.staminaDrainHeavy : params.staminaDrainLight;

    currentStamina -= staminaDrain;

    // カウンターするか判断
    if (enemy.ShouldCounterAfterBlock() && currentPhase != BlockPhase::HIT) {
        enemy.GetStateManager().PopState();
        enemy.GetStateManager().PushState(&EnemyComboAttackState::Instance());
        return;
    }

    // スタミナが足りなければガード崩れ
    if (currentStamina < params.breakThreshold) {
        OnBlockBreak(enemy);
        return;
    }

    // ヒット反応に入る
    if (currentPhase == BlockPhase::HOLD || currentPhase == BlockPhase::START) {
        ChangePhase(enemy, BlockPhase::HIT);
    }
}

void EnemyBlockState::OnBlockBreak(Enemy& enemy) {
    ChangePhase(enemy, BlockPhase::BREAK);

    // スタミナを空にする
    currentStamina = 0.0f;
}

bool EnemyBlockState::ShouldBlockNow(const Enemy& enemy, float elapsedTime) {
    // クールダウン中はしない
    if (blockCooldownTimer > 0.0f) {
        blockCooldownTimer -= elapsedTime;
        return false;
    }

    float distance = enemy.CalculateDistanceToPlayer();
    bool isPlayerAttacking = Player::Instance().IsAttacking();
    if (!isPlayerAttacking) return false;
    if (distance >= EnemyConstants::MIN_BLOCK_DISTANCE) return false;
    if (!enemy.IsFacingPlayer()) return false;

    // 次のガードまでの待ち時間を設定
    float cooldownRange = EnemyConstants::BLOCK_COOLDOWN_MAX - EnemyConstants::BLOCK_COOLDOWN_MIN;
    blockCooldownTimer = EnemyConstants::BLOCK_COOLDOWN_MIN +
        (TransformUtils::RandomInt(0, 99) / 100.0f) * cooldownRange;
    return true;
}

void EnemyBlockState::Exit(Enemy& enemy) {
    // クリーンアップは不要
}