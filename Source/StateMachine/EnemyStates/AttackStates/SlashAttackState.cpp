#include "SlashAttackState.h"
#include "StateMachine/EnemyStates/TeleportState.h"
#include "GameSystems/HitManagement/HitManager.h"
#include "GameSystems/Effect/EffectManager.h"
#include "GameSystems/UI/UIManager.h"

void EnemyAttackState::Enter(Enemy& enemy)
{
    currentPhase = AttackPhase::RUN_START;
    runLoopCount = 0;

    // 走り開始アニメーションを再生する
    enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::RUN_START_F), false);
}

void EnemyAttackState::Update(Enemy& enemy, float elapsedTime)
{
    // 攻撃中は常にプレイヤーの方向を向く
    float turnSpeed = (currentPhase == AttackPhase::ATTACK)
        ? EnemyConstants::TURN_SPEED_FAST
        : EnemyConstants::TURN_SPEED_NORMAL;
    enemy.SmoothlyTrackPlayer(elapsedTime, turnSpeed);

    const auto& anim = enemy.model->GetAnimations().at(enemy.animationIndex);
    int currentFrame = static_cast<int>(enemy.animationSeconds * EnemyConstants::BERSERK_FRAME_RATE);

    // 攻撃フェーズ中のみカウンターウィンドウとヒット検出を処理する
    if (currentPhase == AttackPhase::ATTACK)
    {
        UpdateCounterWindow(enemy, currentFrame);
        UpdateHitDetection(enemy, currentFrame);
    }
    else
    {
        enemy.SetInCounterWindow(false);
    }

    // 現在のアニメーションが終了したかをチェックする
    if (enemy.animationSeconds >= anim.secondsLength)
    {
        HandleAnimationTransition(enemy);
    }
}

void EnemyAttackState::Exit(Enemy& enemy)
{
    // エフェクトを停止する
    EffectManager::Instance().StopEffect(counterEffectHandle);
    UIManager::GetInstance().SetCounterPromptVisible(false);
    counterEffectHandle = -1;
    runLoopCount = 0;
    enemy.SetWeaponHitActive(false);
    hasHitThisAttack = false;
    soundPlayedForThisAttack = false;
    enemy.StopEnemySound(EnemySound::SLASH);
}

void EnemyAttackState::UpdateCounterWindow(Enemy& enemy, int currentFrame)
{
    // カウンター有効フレームは0から24まで
    bool inCounterWindow = (currentFrame >= EnemyConstants::ATTACK_COUNTER_START_FRAME &&
        currentFrame <= EnemyConstants::ATTACK_COUNTER_END_FRAME);

    // 距離もチェックする
    if (inCounterWindow)
    {
        inCounterWindow = (enemy.CalculateDistanceToPlayer() <= EnemyConstants::ATTACK_COUNTER_RANGE);
    }

    enemy.SetInCounterWindow(inCounterWindow);
    bool playerCountered = Player::Instance().GetStateManager().IsInState(PlayerStateID::COUNTER);
    UIManager::GetInstance().SetCounterPromptVisible(inCounterWindow && !playerCountered);
    DirectX::XMFLOAT3 warningAreaPos = Player::Instance().GetPosition();
    warningAreaPos.y = EnemyConstants::ATTACK_WARNING_AREA_Y;

    // カウンターウィンドウ中は警告エフェクトを表示する
    if (inCounterWindow && counterEffectHandle < 0)
    {
        counterEffectHandle = EffectManager::Instance().PlayEffectWithHandle(
            "LightWarningArea", warningAreaPos, EnemyConstants::ATTACK_COUNTER_EFFECT_SCALE
        );
    }
    else if (inCounterWindow && counterEffectHandle >= 0)
    {
        // エフェクトの位置を更新する
        EffectManager::Instance().SetEffectPosition(counterEffectHandle, warningAreaPos);
    }
    else if (playerCountered && counterEffectHandle >= 0)
    {
        // プレイヤーがカウンター中も位置を更新し続ける
        EffectManager::Instance().SetEffectPosition(counterEffectHandle, warningAreaPos);
    }
    else if (!inCounterWindow && counterEffectHandle >= 0 && !playerCountered)
    {
        // ウィンドウ終了かつカウンターなしでエフェクトを停止する
        EffectManager::Instance().StopEffect(counterEffectHandle);
        counterEffectHandle = -1;
    }
}

void EnemyAttackState::UpdateHitDetection(Enemy& enemy, int currentFrame)
{
    // ヒット有効フレームは15から22まで
    bool inHitWindow = (currentFrame >= EnemyConstants::ATTACK_HIT_START_FRAME &&
        currentFrame <= EnemyConstants::ATTACK_HIT_END_FRAME);

    if (inHitWindow && !hasHitThisAttack)
    {
        enemy.SetWeaponHitActive(true);

        // 攻撃ごとに一度だけ音を再生する
        if (!soundPlayedForThisAttack)
        {
            enemy.PlayEnemySound(EnemySound::SLASH, EnemyConstants::DEFAULT_SOUND_VOLUME);
            soundPlayedForThisAttack = true;
        }
    }
    else
    {
        enemy.SetWeaponHitActive(false);
    }

    // 衝突検出
    if (enemy.IsWeaponHitActive() && !hasHitThisAttack)
    {
        auto& hitManager = HitManager::Instance();
        bool hitOccurred = hitManager.ProcessEnemyWeaponHit(enemy, Player::Instance());

        if (hitOccurred)
        {
            // ヒットしたら即座にヒットボックスを無効にする
            enemy.SetWeaponHitActive(false);
            hasHitThisAttack = true;
        }
    }
}

void EnemyAttackState::UpdateRunLoop(Enemy& enemy)
{
    // 攻撃範囲内に入ったら攻撃フェーズへ移行する
    if (enemy.CalculateDistanceToPlayer() <= EnemyConstants::ATTACK_RANGE)
    {
        currentPhase = AttackPhase::ATTACK;
        enemy.animationIndex = enemy.GetAnimIndex(EnemyAnim::RUN_ATTACK_01);
        enemy.animationSeconds = 0.0f;
        enemy.oldAnimationSeconds = 0.0f;
        enemy.model->PlayAnimation(enemy.animationIndex, false, enemy.animationBlendSeconds);
        runLoopCount = 0;
    }
    else
    {
        runLoopCount++;
        
        if (runLoopCount >= EnemyConstants::ATTACK_RUN_LOOP_LIMIT)
        {
            // 現在のステート（AttackState）をポップする
            enemy.GetStateManager().PopState();

            // テレポートステートをプッシュする（着地時にコンボ攻撃へ移行する）
            enemy.GetStateManager().PushState(&EnemyTeleportState::Instance());

            // カウンターをリセットする
            runLoopCount = 0;
            return;
        }

        // まだ遠いので走りループを継続する
        enemy.animationSeconds = 0.0f;
        enemy.oldAnimationSeconds = 0.0f;
        enemy.model->PlayAnimation(enemy.animationIndex, false, enemy.animationBlendSeconds);
    }
}

void EnemyAttackState::HandleAnimationTransition(Enemy& enemy)
{
    switch (currentPhase)
    {
    case AttackPhase::RUN_START:
        // 走りループへ移行する
        currentPhase = AttackPhase::RUN_LOOP;
        enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::RUN_LOOP_F), false);
        break;

    case AttackPhase::RUN_LOOP:
        UpdateRunLoop(enemy);
        break;

    case AttackPhase::ATTACK:
        // 攻撃終了 待機状態に戻る
        hasHitThisAttack = false;
        soundPlayedForThisAttack = false;
        enemy.GetStateManager().PopState();
        enemy.GetStateManager().PushState(&EnemyIdleState::Instance());
        break;
    }
}