#include "ComboAttackState.h"
#include "GameSystems/HitManagement/HitManager.h"
#include "GameSystems/UI/UIManager.h"
#include "GameSystems/Time/TimeManager.h"
#include "GameSystems/Effect/EffectManager.h"

void EnemyComboAttackState::Enter(Enemy& enemy)
{
    currentAttack = CharacterConstants::FIRST_ATTACK;
    currentFrame = 0;
    hasHitThisAttack = false;

    enemy.SetCurrentAttackIndex(currentAttack);

    // 予告として発光色を赤く光らせる
    if (!enemy.IsBerserk())
    {
        enemy.FlashEmissive(
            DirectX::XMFLOAT3(EnemyConstants::COMBO_EMISSIVE_RED, 0.0f, 0.0f),
            EnemyConstants::COMBO_EMISSIVE_FLASH_DURATION
        );
    }

    // 演出のため時間を少し遅くする
    TimeManager::Instance().SetTimeScale(
        EnemyConstants::COMBO_TIME_SCALE,
        EnemyConstants::TELEPORT_TIME_SCALE_DURATION
    );

    // 最初の攻撃アニメーションを再生する
    enemy.StartAnimation(enemy.GetAnimIndex(attackAnimations[currentAttack]), false);
}

void EnemyComboAttackState::Update(Enemy& enemy, float elapsedTime)
{
    // コンボ中は常にプレイヤーの方向を向く
    enemy.SmoothlyTrackPlayer(elapsedTime, EnemyConstants::TURN_SPEED_FAST);

    // アニメーション時間をフレーム数に変換する
    currentFrame = static_cast<int>(enemy.animationSeconds * EnemyConstants::BERSERK_FRAME_RATE);

    // カウンターウィンドウを更新する（最初の攻撃のみ）
    UpdateCounterWindow(enemy);

    // 発光色をパルスさせる
    UpdateEmissivePulse(enemy, elapsedTime);

    // ヒット検出を更新する
    UpdateHitDetection(enemy);

    // 次の攻撃へ移行するか判定する
    CheckTransition(enemy);

    // アニメーションが終了した場合の処理
    CheckAttackCompletion(enemy);
}

void EnemyComboAttackState::Exit(Enemy& enemy)
{
    TimeManager::Instance().ResetTimeScale(EnemyConstants::TELEPORT_TIME_RESET_DURATION);
    UIManager::GetInstance().SetCounterPromptVisible(false);
    enemy.SetWeaponHitActive(false);
    enemy.SetCurrentSlashIndex(0);

    enemy.ResetEmissiveColor();

    if (counterEffectHandle >= 0)
    {
        EffectManager::Instance().StopEffect(counterEffectHandle);
        counterEffectHandle = -1;
    }

    enemy.StopEnemySound(EnemySound::SLASH);
}

void EnemyComboAttackState::UpdateCounterWindow(Enemy& enemy)
{
    // 最初の攻撃のみカウンターウィンドウを検出する
    if (currentAttack == CharacterConstants::FIRST_ATTACK)
    {
        bool inCounterWindow = (currentFrame >= EnemyConstants::COMBO_COUNTER_START_FRAME &&
            currentFrame <= EnemyConstants::COMBO_COUNTER_FRAMES);

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

        if (inCounterWindow && counterEffectHandle < 0)
        {
            counterEffectHandle = EffectManager::Instance().PlayEffectWithHandle(
                "LightWarningArea", warningAreaPos, EnemyConstants::ATTACK_COUNTER_EFFECT_SCALE
            );
        }
        else if (inCounterWindow && counterEffectHandle >= 0)
        {
            EffectManager::Instance().SetEffectPosition(counterEffectHandle, warningAreaPos);
        }
        else if (playerCountered && counterEffectHandle >= 0)
        {
            EffectManager::Instance().SetEffectPosition(counterEffectHandle, warningAreaPos);
        }
        else if (!inCounterWindow && counterEffectHandle >= 0 && !playerCountered)
        {
            EffectManager::Instance().StopEffect(counterEffectHandle);
            counterEffectHandle = -1;
        }
    }
    else
    {
        enemy.SetInCounterWindow(false);
    }
}

void EnemyComboAttackState::UpdateEmissivePulse(Enemy& enemy, float elapsedTime)
{
    // コンボ中は発光色をパルスさせる
    static float pulseTimer = 0.0f;
    pulseTimer += elapsedTime;

    float pulse = (sin(pulseTimer * EnemyConstants::COMBO_PULSE_SPEED) + 1.0f) * 0.5f;

    DirectX::XMFLOAT3 pulsedColor = {};
    pulsedColor.x = 1.0f + pulse * 1.0f;
    pulsedColor.y = 0.0f;
    pulsedColor.z = 0.0f;

    if (!enemy.IsBerserk())
    {
        enemy.SetEmissiveColor(pulsedColor);
    }
}

void EnemyComboAttackState::UpdateHitDetection(Enemy& enemy)
{
    // 3回目の攻撃は2回のスラッシュがある
    if (currentAttack == CharacterConstants::THIRD_ATTACK)
    {
        UpdateHitDetectionForThirdAttack(enemy);
    }
    else
    {
        UpdateHitDetectionForOtherAttacks(enemy);
    }
}

void EnemyComboAttackState::UpdateHitDetectionForThirdAttack(Enemy& enemy)
{
    bool inFirstHitWindow = (currentFrame >= EnemyConstants::COMBO_ATTACK_3_FIRST_HIT_START &&
        currentFrame <= EnemyConstants::COMBO_ATTACK_3_FIRST_HIT_END);

    bool inSecondHitWindow = (currentFrame >= EnemyConstants::COMBO_ATTACK_3_SECOND_HIT_START &&
        currentFrame <= EnemyConstants::COMBO_ATTACK_3_SECOND_HIT_END);

    bool inHitWindow = inFirstHitWindow || inSecondHitWindow;

    if (inFirstHitWindow && currentSlash != 1)
    {
        currentSlash = 1;
        hasHitThisAttack = false;
        enemy.SetWeaponHitActive(true);
        enemy.PlayEnemySound(EnemySound::SLASH, EnemyConstants::ATTACK_COMBO_SLASH_VOLUME);
    }
    else if (inSecondHitWindow && currentSlash != 2)
    {
        currentSlash = 2;
        hasHitThisAttack = false;
        enemy.SetWeaponHitActive(true);
        enemy.StopEnemySound(EnemySound::SLASH);
        enemy.PlayEnemySound(EnemySound::SLASH, EnemyConstants::ATTACK_COMBO_SLASH_VOLUME);
    }
    else if (!inHitWindow)
    {
        enemy.SetWeaponHitActive(false);

        if (currentFrame > EnemyConstants::COMBO_ATTACK_3_RESET_FRAME)
        {
            currentSlash = 0;
        }
    }

    // ヒット検出を処理する
    if (inHitWindow && enemy.IsWeaponHitActive() && !hasHitThisAttack)
    {
        enemy.SetCurrentSlashIndex(currentSlash);

        auto& hitManager = HitManager::Instance();
        bool hitOccurred = hitManager.ProcessEnemyWeaponHit(enemy, Player::Instance());

        if (hitOccurred)
        {
            hasHitThisAttack = true;
            enemy.SetWeaponHitActive(false);
        }
    }
}

void EnemyComboAttackState::UpdateHitDetectionForOtherAttacks(Enemy& enemy)
{
    const AttackTiming& timing = attackTimings[currentAttack];

    // 他の攻撃は単一のヒットウィンドウ
    bool inHitWindow = (currentFrame >= timing.hitStartFrame &&
        currentFrame <= timing.hitEndFrame);

    if (inHitWindow && !hasHitThisAttack)
    {
        enemy.SetWeaponHitActive(true);
        enemy.StopEnemySound(EnemySound::SLASH);
        enemy.PlayEnemySound(EnemySound::SLASH, EnemyConstants::ATTACK_COMBO_SLASH_VOLUME);

        auto& hitManager = HitManager::Instance();
        bool hitOccurred = hitManager.ProcessEnemyWeaponHit(enemy, Player::Instance());

        if (hitOccurred)
        {
            hasHitThisAttack = true;
            enemy.SetWeaponHitActive(false);
        }
    }
    else if (!inHitWindow)
    {
        enemy.SetWeaponHitActive(false);
    }
}

void EnemyComboAttackState::TransitionToNextAttack(Enemy& enemy)
{
    currentAttack++;
    currentFrame = 0;
    hasHitThisAttack = false;

    enemy.SetCurrentSlashIndex(0);

    // 敵の攻撃追跡情報を更新する
    enemy.SetCurrentAttackIndex(currentAttack);

    // 次の攻撃アニメーションを再生する
    enemy.StartAnimation(enemy.GetAnimIndex(attackAnimations[currentAttack]), false);
}

void EnemyComboAttackState::CheckTransition(Enemy& enemy)
{
    const AttackTiming& timing = attackTimings[currentAttack];

    bool shouldTransition = (hasHitThisAttack && timing.transitionFrame != -1 &&
        currentFrame >= timing.transitionFrame);

    if (shouldTransition)
    {
        TransitionToNextAttack(enemy);
    }
}

void EnemyComboAttackState::CheckAttackCompletion(Enemy& enemy)
{
    const AttackTiming& timing = attackTimings[currentAttack];

    if (currentFrame >= timing.totalFrames)
    {
        if (currentAttack == attackTimings.size() - 1)
        {
            // 最終攻撃が完了したらコンボ終了
            enemy.GetStateManager().PopState();
            enemy.GetStateManager().PushState(&EnemyIdleState::Instance());
        }
        else if (!hasHitThisAttack)
        {
            // ヒットなしで終了したらコンボ中断
            enemy.GetStateManager().PopState();
            enemy.GetStateManager().PushState(&EnemyIdleState::Instance());
        }
    }
}