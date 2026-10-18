#include <cmath>

#include "HitStates.h"
#include "BerserkState.h"
#include "GameLib/TransformUtils.h"
#include "CameraFiles/CameraShake.h"
#include "GameSystems/Particle/ParticleManager.h"

// ============================================================================
// RegularHitState
// ============================================================================

void RegularHitState::Enter(Enemy& enemy) {
    // ヒット時のカメラ揺れ
    CameraShake::Instance().AddTrauma(EnemyConstants::REGULAR_HIT_TRAUMA);

    auto& hitData = enemy.GetLastHitData();

    // 攻撃方向を計算して対応するアニメーションを再生
    std::string newHitDirection = CalculateHitDirection(enemy, hitData);
    PlayDirectionalHitAnimation(enemy, newHitDirection);

    // ヒットリアクション時間をリセット
    timer = EnemyConstants::REGULAR_HIT_ANIM_DURATION;
}

void RegularHitState::Update(Enemy& enemy, float elapsedTime) {
    // 死亡条件チェック
    if (enemy.GetHealth() <= 0 && !enemy.GetStateManager().IsInState(EnemyStateID::DIE)) {
        enemy.GetStateManager().PushState(&EnemyDieState::Instance());
        return;
    }

    timer -= elapsedTime;

    if (timer <= 0.0f) {
        auto& hitData = enemy.GetLastHitData();

        enemy.GetStateManager().PopState();

        // 3段目の攻撃ならスタンさせる
        if (hitData.comboStep == EnemyConstants::COMBO_STEP_THIRD_ATTACK) {
            auto& stunState = StunState::Instance();
            stunState.SetStunDuration(EnemyConstants::STUN_DURATION_AFTER_THIRD_HIT);
            enemy.GetStateManager().PushState(&stunState);
        }
    }
}

void RegularHitState::Exit(Enemy& enemy) {
    // クリーンアップは不要
}

std::string RegularHitState::CalculateHitDirection(const Enemy& enemy, const Character::HitData& hitData) {
    DirectX::XMFLOAT3 hitDir = hitData.hitDirection;

    // 敵の前方向ベクトル
    DirectX::XMFLOAT3 enemyForward = {
        sin(enemy.GetAngle().y),
        0.0f,
        cos(enemy.GetAngle().y)
    };

    // 前後左右を判定するための計算
    float dotProduct = enemyForward.x * hitDir.x + enemyForward.z * hitDir.z;
    float crossY = enemyForward.x * hitDir.z - enemyForward.z * hitDir.x;

    std::string direction;

    // 前後左右を文字で決定
    if (dotProduct > EnemyConstants::DIRECTION_DOT_FRONT) {
        direction = "F";
    }
    else if (dotProduct < EnemyConstants::DIRECTION_DOT_BACK) {
        direction = "B";
    }
    else if (crossY > 0.0f) {
        direction = "R";
    }
    else {
        direction = "L";
    }

    return direction;
}

void RegularHitState::PlayDirectionalHitAnimation(Enemy& enemy, const std::string& direction) {
    // 方向に応じたアニメーション名を作る
    std::string animName = "Hit_Combat_" + direction + "_Seq_0";
    int hitAnim = enemy.model->GetAnimationIndex(animName.c_str());

    // 見つからなければ前方向を使う
    if (hitAnim == -1) {
        hitAnim = enemy.model->GetAnimationIndex("Hit_Combat_F_Seq_0");
    }

    enemy.StartAnimation(hitAnim, false);
}

// ============================================================================
// LaunchHitState
// ============================================================================

void LaunchHitState::Enter(Enemy& enemy) {
    // 打ち上げ時のカメラ揺れ
    CameraShake::Instance().AddTrauma(EnemyConstants::LAUNCH_HIT_TRAUMA);

    currentPhase = LaunchPhase::Launch;
    phaseTimer = 0.0f;

    // 打ち上げ用アニメーション
    enemy.animationIndex = enemy.GetAnimIndex(EnemyAnim::DEATH);
    phaseDuration = enemy.model->GetAnimations().at(enemy.animationIndex).secondsLength;

    enemy.StartAnimation(enemy.animationIndex, false);
}

void LaunchHitState::Update(Enemy& enemy, float elapsedTime) {
    // 死亡チェック
    if (enemy.GetHealth() <= 0 && !enemy.GetStateManager().IsInState(EnemyStateID::DIE)) {
        enemy.GetStateManager().PushState(&EnemyDieState::Instance());
        return;
    }

    phaseTimer += elapsedTime;

    switch (currentPhase) {
    case LaunchPhase::Launch:
        // 一定時間後にテレポート処理
        if (phaseTimer >= EnemyConstants::LAUNCH_PHASE_DURATION) {
            Player& player = Player::Instance();
            DirectX::XMFLOAT3 playerPos = player.GetPosition();

            // プレイヤーの後ろ側にランダムで位置を決める
            DirectX::XMFLOAT3 playerForward = player.GetForwardVector();
            float behindAngle = atan2f(playerForward.x, playerForward.z) + DirectX::XM_PI;

            float angleOffset = (TransformUtils::RandomInt(0, 119) - 60.0f) * CharacterConstants::RADIANS_PER_DEGREE;
            float finalAngle = behindAngle + angleOffset;
            float distance = EnemyConstants::HIT_TELEPORT_DISTANCE;

            DirectX::XMFLOAT3 safePosition;
            safePosition.x = playerPos.x + cosf(finalAngle) * distance;
            safePosition.z = playerPos.z + sinf(finalAngle) * distance;
            safePosition.y = 0.0f;

            // 消えるエフェクト
            DirectX::XMFLOAT3 disappearPos = enemy.position;
            disappearPos.y += EnemyConstants::HIT_TELEPORT_Y_OFFSET;
            ParticleManager::Instance().SpawnFullSequenceForward(disappearPos);

            // 位置を移動
            enemy.position = safePosition;

            // 出現エフェクト
            DirectX::XMFLOAT3 appearPos = safePosition;
            appearPos.y += EnemyConstants::HIT_TELEPORT_Y_OFFSET;
            ParticleManager::Instance().SpawnFullSequenceReverse(appearPos);

            // プレイヤーの方向を向く
            DirectX::XMFLOAT3 toPlayer = {
                playerPos.x - safePosition.x,
                0.0f,
                playerPos.z - safePosition.z
            };
            float len = sqrtf(toPlayer.x * toPlayer.x + toPlayer.z * toPlayer.z);
            if (len > 0.0f) {
                enemy.angle.y = atan2f(toPlayer.x, toPlayer.z);
            }

            // ステート終了
            enemy.GetStateManager().PopState();

            // 無敵時間を付ける
            enemy.invincibleTimer = EnemyConstants::HIT_TELEPORT_INVINCIBLE_TIME;

            // アイドルに戻す
            enemy.animationIndex = enemy.GetAnimIndex(EnemyAnim::IDLE_COMBAT);
            enemy.animationLoop = true;
            enemy.model->PlayAnimation(enemy.animationIndex, true, enemy.animationBlendSeconds);

            currentPhase = LaunchPhase::Finished;
        }
        break;

    case LaunchPhase::Finished:
        break;
    }
}

void LaunchHitState::Exit(Enemy& enemy) {
    // クリーンアップは不要
}

// ============================================================================
// StunState
// ============================================================================

void StunState::Enter(Enemy& enemy) {
    // スタン時のカメラ揺れ
    CameraShake::Instance().AddTrauma(EnemyConstants::STUN_HIT_TRAUMA);

    // スタン用アニメーション
    enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::HIT_LAUNCH), false);

    // スタン時間を設定
    stunTimer = stunDuration;
}

void StunState::Update(Enemy& enemy, float elapsedTime) {
    // 死亡チェック
    if (enemy.GetHealth() <= 0 && !enemy.GetStateManager().IsInState(EnemyStateID::DIE)) {
        enemy.GetStateManager().PushState(&EnemyDieState::Instance());
        return;
    }

    stunTimer -= elapsedTime;

    // スタン終了
    if (stunTimer <= 0.0f) {
        enemy.GetStateManager().PopState();
    }
}

void StunState::Exit(Enemy& enemy) {
    // 動きを止める
    enemy.velocity.x = 0.0f;
    enemy.velocity.z = 0.0f;
}

// ============================================================================
// KnockDownState
// ============================================================================

void KnockDownState::Enter(Enemy& enemy) {
    // ダウン時のカメラ揺れ
    CameraShake::Instance().AddTrauma(EnemyConstants::KNOCKDOWN_TRAUMA);

    currentPhase = KnockDownPhase::Start;
    phaseTimer = 0.0f;

    // 倒れ始めのアニメーション
    enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::KNOCK_DOWN_START), false);
}

void KnockDownState::Update(Enemy& enemy, float elapsedTime) {
    // 死亡チェック
    if (enemy.GetHealth() <= 0 && !enemy.GetStateManager().IsInState(EnemyStateID::DIE)) {
        enemy.GetStateManager().PushState(&EnemyDieState::Instance());
        return;
    }

    phaseTimer += elapsedTime;

    switch (currentPhase) {
    case KnockDownPhase::Start:
        // 開始が終わったらループへ
        if (phaseTimer >= startDuration) {
            currentPhase = KnockDownPhase::Loop;
            phaseTimer = 0.0f;

            enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::KNOCK_DOWN_LOOP), true);
        }
        break;

    case KnockDownPhase::Loop:
        // 一定時間倒れたまま
        if (phaseTimer >= loopDuration) {
            currentPhase = KnockDownPhase::End;
            phaseTimer = 0.0f;

            enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::KNOCK_DOWN_END), false);
        }
        break;

    case KnockDownPhase::End:
        // 起き上がりが終わったら終了
        if (phaseTimer >= endDuration) {
            currentPhase = KnockDownPhase::Finished;

            enemy.GetStateManager().PopState();

            // 復帰後に別の行動へ
            enemy.GetStateManager().PushState(&BerserkState::Instance());
        }
        break;

    case KnockDownPhase::Finished:
        break;
    }
}

void KnockDownState::Exit(Enemy& enemy) {
    // 移動を止める
    enemy.velocity.x = 0.0f;
    enemy.velocity.z = 0.0f;
}

// ============================================================================
// EnemyDieState
// ============================================================================

void EnemyDieState::Enter(Enemy& enemy) {
    // 死亡アニメーション
    enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::DEATH), false);

    // 動きを完全に止める
    enemy.velocity = { 0.0f, 0.0f, 0.0f };

    // 死亡までのタイマー
    dieTimer = CharacterConstants::DEATH_TIMER;
}

void EnemyDieState::Update(Enemy& enemy, float elapsedTime) {
    dieTimer -= elapsedTime;

    // 一定時間後に消える演出を開始
    if (dieTimer <= CharacterConstants::DEATH_DISSOLVE_START) {
        enemy.PlayEnemySound(EnemySound::DEATH, CharacterConstants::DEATH_SOUND_VOLUME);

        dissolveTimer += elapsedTime;

        // 徐々に消すための値を計算
        float t = dissolveTimer / CharacterConstants::DEATH_DISSOLVE_START;
        float dissolveValue = t * t * (CharacterConstants::SMOOTH_STEP_A - CharacterConstants::SMOOTH_STEP_B * t);

        enemy.model->SetDissolveThreshold(dissolveValue);
        enemy.GetSword().model->SetDissolveThreshold(dissolveValue);

        // エッジの発光を調整
        float glowIntensity = CharacterConstants::DEATH_DISSOLVE_EDGE_INTENSITY * (1.0f - dissolveValue);
        enemy.model->SetDissolveEdgeIntensity(glowIntensity);
        enemy.model->SetDissolveEdgeWidth(CharacterConstants::DEATH_DISSOLVE_EDGE_WIDTH);

        if (enemy.GetSword().model) {
            enemy.GetSword().model->SetDissolveEdgeIntensity(glowIntensity * CharacterConstants::DEATH_SWORD_EDGE_INTENSITY_RATIO);
            enemy.GetSword().model->SetDissolveEdgeWidth(CharacterConstants::DEATH_SWORD_EDGE_WIDTH);
        }
    }

    // 完全に終了
    if (dieTimer <= 0.0f) {
        enemy.isDead = true;
    }
}

void EnemyDieState::Exit(Enemy& enemy) {
    // サウンド停止とリセット
    enemy.StopEnemySound(EnemySound::DEATH);
    dissolveTimer = 0.0f;
}