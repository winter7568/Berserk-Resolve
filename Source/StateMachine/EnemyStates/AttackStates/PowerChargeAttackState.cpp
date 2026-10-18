#include "PowerChargeAttackState.h"
#include "GameSystems/HitManagement/HitManager.h"
#include "GameSystems/Time/TimeManager.h"

void EnemyPowerChargeState::Enter(Enemy& enemy)
{
    currentPhase = ChargePhase::CHARGE_START;
    phaseTimer = 0.0f;
    slashesPerformed = 0;

    // 6回のスラッシュのヒット状態をリセットする
    for (int i = 0; i < EnemyConstants::POWER_CHARGE_TOTAL_SLASHES; i++)
    {
        slashHit[i] = false;
    }

    // 開始位置を保存する
    DirectX::XMFLOAT3 spawnPosition = enemy.GetPosition();
    spawnPosition.y -= EnemyConstants::POWER_CHARGE_START_Y_OFFSET;

    chargeStartPosition = spawnPosition;
    chargeStartPositionSaved = true;
    positionLockedThisSequence = false;

    // 保存した位置でエフェクトを開始する
    enemy.StartPowerChargeEffect(chargeStartPosition);

    // 演出のため時間を遅くする
    TimeManager::Instance().SetTimeScale(
        EnemyConstants::POWER_CHARGE_TIME_SLOW_FACTOR,
        EnemyConstants::TELEPORT_TIME_SCALE_DURATION
    );

    // チャージ開始アニメーションを再生する
    enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::POWER_ATTACK_START), false);
}

void EnemyPowerChargeState::Update(Enemy& enemy, float elapsedTime)
{
    phaseTimer += elapsedTime;

    switch (currentPhase)
    {
    case ChargePhase::CHARGE_START:
        UpdateChargeStart(enemy);
        break;

    case ChargePhase::CHARGE_HOLD:
        UpdateChargeHold(enemy, elapsedTime);
        break;

    case ChargePhase::DOMAIN_SLASHES:
        UpdateDomainSlashes(enemy);
        break;
    }
}

void EnemyPowerChargeState::Exit(Enemy& enemy)
{
    // 後片付け
    enemy.SetWeaponHitActive(false);
    TimeManager::Instance().ResetTimeScale(EnemyConstants::TELEPORT_TIME_RESET_DURATION);

    // エフェクトを停止する
    enemy.StopPowerChargeEffect();
    enemy.StopEnemySound(EnemySound::MULTI_SLASH);

    chargeStartPositionSaved = false;
}

void EnemyPowerChargeState::InstantlyFacePlayer(Enemy& enemy)
{
    DirectX::XMFLOAT2 dir = enemy.GetDirectionToPlayer();

    if (dir.x != 0.0f || dir.y != 0.0f)
    {
        // 補間なしで直接角度を設定する
        float targetAngle = std::atan2(dir.x, dir.y);

        DirectX::XMFLOAT3 currentAngle = enemy.GetAngle();
        currentAngle.y = targetAngle;
        enemy.SetAngle(currentAngle);
    }
}

void EnemyPowerChargeState::UpdateChargeStart(Enemy& enemy)
{
    const auto& anim = enemy.model->GetAnimations().at(enemy.animationIndex);

    if (phaseTimer >= anim.secondsLength)
    {
        // チャージ保持フェーズへ移行する
        currentPhase = ChargePhase::CHARGE_HOLD;
        phaseTimer = 0.0f;

        enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::POWER_ATTACK_LOOP), true);
    }
}

void EnemyPowerChargeState::UpdateChargeHold(Enemy& enemy, float elapsedTime)
{
    // チャージ中はプレイヤーの方向を向く
    enemy.SmoothlyTrackPlayer(elapsedTime, EnemyConstants::TURN_SPEED_NORMAL);

    // 2秒間チャージを保持する
    if (phaseTimer >= EnemyConstants::POWER_CHARGE_HOLD_DURATION)
    {
        currentPhase = ChargePhase::DOMAIN_SLASHES;
        phaseTimer = 0.0f;
        slashesPerformed = 0;

        // スラッシュ開始時の位置を保存する
        savedPosition = enemy.GetPosition();

        // スラッシュ前に正面を向ける
        InstantlyFacePlayer(enemy);

        // ドメインスラッシュのアニメーションを再生する
        enemy.StartAnimation(enemy.GetAnimIndex(EnemyAnim::POWER_ATTACK_END), false);
        enemy.PlayEnemySound(EnemySound::MULTI_SLASH, EnemyConstants::POWER_CHARGE_MULTI_SLASH_VOLUME);

        // 武器ヒットを有効にする
        enemy.SetWeaponHitActive(true);
    }
}

void EnemyPowerChargeState::UpdateDomainSlashes(Enemy& enemy)
{
    // 一度ヒットしたら位置を固定して移動を止める
    if (slashesPerformed > 0)
    {
        enemy.SetPosition(savedPosition);
        enemy.SetVelocity({ 0.0f, 0.0f, 0.0f });
    }

    int currentFrame = static_cast<int>(enemy.animationSeconds * EnemyConstants::BERSERK_FRAME_RATE);
    int currentSlash = -1;

    // 現在のスラッシュウィンドウを判定する
    bool inHitWindow = IsInSlashWindow(currentFrame, currentSlash);

    // ヒットウィンドウ中でまだヒットしていないスラッシュの場合
    if (inHitWindow)
    {
        InstantlyFacePlayer(enemy);
        if (!enemy.IsWeaponHitActive() && !slashHit[currentSlash])
        {
            enemy.SetWeaponHitActive(true);

            // 攻撃追跡用の情報を設定する
            enemy.SetCurrentAttackIndex(currentSlash);
            enemy.SetCurrentAttackStateID(EnemyStateID::POWER_CHARGE);
            enemy.SetCurrentSlashIndex(currentSlash);
        }

        // ヒット検出を処理する
        if (enemy.IsWeaponHitActive() && !slashHit[currentSlash])
        {
            ProcessSlashHit(enemy, currentSlash);
        }
    }
    else if (enemy.IsWeaponHitActive())
    {
        enemy.SetWeaponHitActive(false);
    }

    // ドメインスラッシュのアニメーションが終了したら終了する
    const auto& anim = enemy.model->GetAnimations().at(enemy.animationIndex);
    if (enemy.animationSeconds >= anim.secondsLength)
    {
        FinishDomainSlashes(enemy);
    }
}

bool EnemyPowerChargeState::IsInSlashWindow(int currentFrame, int& outSlashIndex) const
{
    if (currentFrame >= EnemyConstants::SLASH_1_START && currentFrame <= EnemyConstants::SLASH_1_END)
    {
        outSlashIndex = 0;
        return true;
    }
    else if (currentFrame >= EnemyConstants::SLASH_2_START && currentFrame <= EnemyConstants::SLASH_2_END)
    {
        outSlashIndex = 1;
        return true;
    }
    else if (currentFrame >= EnemyConstants::SLASH_3_START && currentFrame <= EnemyConstants::SLASH_3_END)
    {
        outSlashIndex = 2;
        return true;
    }
    else if (currentFrame >= EnemyConstants::SLASH_4_START && currentFrame <= EnemyConstants::SLASH_4_END)
    {
        outSlashIndex = 3;
        return true;
    }
    else if (currentFrame >= EnemyConstants::SLASH_5_START && currentFrame <= EnemyConstants::SLASH_5_END)
    {
        outSlashIndex = 4;
        return true;
    }
    else if (currentFrame >= EnemyConstants::SLASH_6_START && currentFrame <= EnemyConstants::SLASH_6_END)
    {
        outSlashIndex = 5;
        return true;
    }

    outSlashIndex = -1;
    return false;
}

void EnemyPowerChargeState::ProcessSlashHit(Enemy& enemy, int currentSlash)
{
    auto& hitManager = HitManager::Instance();
    bool hitOccurred = hitManager.ProcessEnemyWeaponHit(enemy, Player::Instance());

    if (hitOccurred)
    {
        slashesPerformed++;
        slashHit[currentSlash] = true;

        // 最初のヒット時のみ位置を固定する
        if (!positionLockedThisSequence)
        {
            savedPosition = enemy.GetPosition();
            positionLockedThisSequence = true;
        }

        // このスラッシュのヒット後はヒットボックスを無効にする
        enemy.SetWeaponHitActive(false);
    }
}

void EnemyPowerChargeState::FinishDomainSlashes(Enemy& enemy)
{
    enemy.SetWeaponHitActive(false);
    TimeManager::Instance().ResetTimeScale(EnemyConstants::TELEPORT_TIME_RESET_DURATION);
    enemy.GetStateManager().PopState();
    enemy.GetStateManager().PushState(&EnemyIdleState::Instance());
}