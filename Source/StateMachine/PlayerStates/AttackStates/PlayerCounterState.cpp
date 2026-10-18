#include "PlayerCounterState.h"
#include "StateMachine/PlayerStates/MoveState.h"
#include "CameraFiles/LockOnController.h"
#include "GameLib/Input.h"
#include "GameSystems/HitManagement/HitManager.h"
#include "GameSystems/UI/UIManager.h"
#include "GameObjects/Enemy/Enemy.h"
#include "GameSystems/Time/TimeManager.h"

void PlayerCounterState::Enter(Player& player)
{
    UIManager::GetInstance().SetCounterPromptVisible(false);
    // カウンターフェーズをジャンプバックに設定
    phase = CounterPhase::JumpBack;
    queuedCounter = false;
    hitRegistered = false;

    // 無敵時間を設定
    player.invincibleTimer = PlayerConstants::COUNTER_INVINCIBLE_DURATION;

    // どの攻撃をカウンターしているか確認
    const Enemy& enemy = Enemy::Instance();
    wasComboCounter = enemy.GetStateManager().IsInState(EnemyStateID::COMBO_ATTACK);

    // ジャンプバック速度を設定
    jumpBackVelocity = PlayerConstants::INITIAL_JUMP_BACK_VELOCITY;
    jumpBackTimer = 0.0f;
    jumpBackDuration = PlayerConstants::JUMP_BACK_DURATION;

    // カウンター開始時にスローモーション開始
    TimeManager::Instance().SetTimeScale(
        PlayerConstants::COUNTER_TIME_SCALE,
        PlayerConstants::COUNTER_TIME_SCALE_DURATION
    );

    // プレイヤーの状態を設定
    player.SetCurrentAttackStateID(PlayerStateID::COUNTER);
    player.SetWeaponHitActive(false);

    // ジャンプバック開始アニメーションを再生 ルートモーションなし 移動は物理演算で処理
    player.StartAnimation(player.GetAnimIndex(PlayerAnim::JUMP_BACK_START), false);

    // 後退方向を計算 敵から遠ざかる方向
    CalculateBackwardDirection(player);
}

void PlayerCounterState::Update(Player& player, float elapsedTime)
{
    const float frameRate = CharacterConstants::FRAME_RATE;
    int currentFrame = static_cast<int>(player.animationSeconds * frameRate);

    // ジャンプバック中の物理ベース後退移動
    if (phase == CounterPhase::JumpBack)
    {
        // 減速しながら後退速度を適用
        if (jumpBackTimer < jumpBackDuration)
        {
            float currentVelocity = PlayerConstants::INITIAL_JUMP_BACK_VELOCITY;

            // コンボ攻撃のカウンターの場合は押し戻しを軽減
            if (wasComboCounter)
            {
                currentVelocity *= PlayerConstants::COMBO_COUNTER_PUSHBACK_MULTIPLIER;
            }

            // プレイヤーを後退移動 水平移動に焦点
            player.position.x += backwardDirection.x * currentVelocity * elapsedTime;
            player.position.z += backwardDirection.z * currentVelocity * elapsedTime;
        }
    }

    // ジャンプバック中の左クリック入力をキューに保存
    if (phase == CounterPhase::JumpBack && player.InputMeleeAttack())
    {
        queuedCounter = true;
    }

    switch (phase)
    {
    case CounterPhase::JumpBack:
        UIManager::GetInstance().SetAttackPromptVisible(true);
        if (currentFrame >= PlayerConstants::JUMP_BACK_FRAMES || queuedCounter)
        {
            // 左クリックが時間内に押された場合 カウンター攻撃へ
            if (queuedCounter)
            {
                player.StartAnimation(player.GetAnimIndex(PlayerAnim::COUNTER_ATTACK), false);
                player.PlayPlayerSound(PlayerSound::COUNTER, PlayerConstants::COUNTER_SOUND_VOLUME);
                phase = CounterPhase::CounterAttack;
                return;
            }
            else
            {
                // カウンター入力なし ジャンプバック終了へ
                player.StartAnimation(player.GetAnimIndex(PlayerAnim::JUMP_BACK_END), false);
                TimeManager::Instance().ResetTimeScale(PlayerConstants::COUNTER_TIME_RESET_DURATION);
                phase = CounterPhase::JumpBackEnd;
               return;
            }
        }
        break;

    case CounterPhase::JumpBackEnd:
        UIManager::GetInstance().SetAttackPromptVisible(false);
        if (currentFrame >= PlayerConstants::JUMP_BACK_END_FRAMES)
        {
            // ジャンプバック終了アニメーション完了 アイドル状態へ
            player.GetStateManager().PopState();
            player.GetStateManager().PushState(&PlayerIdleState::Instance());
            return;
        }
        break;

    case CounterPhase::CounterAttack:
        UIManager::GetInstance().SetAttackPromptVisible(false);
        // 通常速度に戻す
        TimeManager::Instance().ResetTimeScale(PlayerConstants::COUNTER_ATTACK_TIME_RESET);

        // ヒットボックスを有効化
        player.SetWeaponHitActive(true);

        // ヒットを一度だけ処理
        if (!hitRegistered)
        {
            if (HitManager::Instance().ProcessWeaponHit(player, Enemy::Instance()))
            {
                hitRegistered = true;
                player.SetWeaponHitActive(false);
            }
        }

        if (currentFrame >= PlayerConstants::COUNTER_ATTACK_FRAMES)
        {
            player.invincibleTimer = 0.0f;
            player.GetStateManager().PopState();
            player.GetStateManager().PushState(&PlayerFallingState::Instance());
            return;
        }
        break;

    default:
        break;
    }
}

void PlayerCounterState::Exit(Player& player)
{
    // 状態変数をリセット
    wasComboCounter = false;
    phase = CounterPhase::None;
    queuedCounter = false;
    hitRegistered = false;
    player.invincibleTimer = 0.0f;

    // カウンター終了時にスローモーション終了 通常速度に戻す
    TimeManager::Instance().ResetTimeScale(PlayerConstants::COUNTER_EXIT_TIME_RESET);
    UIManager::GetInstance().SetAttackPromptVisible(false);

    // ヒットボックスを無効化
    player.SetWeaponHitActive(false);
    player.SetCurrentAttackStateID(PlayerStateID::NONE);
}

bool PlayerCounterState::CanCounterEnemyAttack(const Player& player) const
{
    const Enemy& enemy = Enemy::Instance();

    // 敵が攻撃状態またはコンボ攻撃状態でない場合はカウンター不可
    if (!enemy.GetStateManager().IsInState(EnemyStateID::ATTACK) &&
        !enemy.GetStateManager().IsInState(EnemyStateID::COMBO_ATTACK))
    {
        return false;
    }

    return IsEnemyInCounterWindow();
}

bool PlayerCounterState::IsEnemyInCounterWindow() const
{
    const Enemy& enemy = Enemy::Instance();

    // 敵の状態に応じてカウンターウィンドウをチェック
    if (enemy.GetStateManager().IsInState(EnemyStateID::ATTACK))
    {
        return enemy.IsInCounterWindow();
    }
    else if (enemy.GetStateManager().IsInState(EnemyStateID::COMBO_ATTACK))
    {
        return enemy.IsInCounterWindow(); // コンボ状態も同じフラグを使用
    }

    return false;
}

void PlayerCounterState::CalculateBackwardDirection(Player& player)
{
    const Enemy& enemy = Enemy::Instance();
    DirectX::XMFLOAT3 playerPos = player.GetPosition();
    DirectX::XMFLOAT3 enemyPos = enemy.GetPosition();

    // 敵からプレイヤーへの方向を計算 後退方向
    DirectX::XMVECTOR toPlayer = DirectX::XMVectorSubtract(
        DirectX::XMLoadFloat3(&playerPos),
        DirectX::XMLoadFloat3(&enemyPos)
    );

    // 正規化して後退方向を保存
    toPlayer = DirectX::XMVector3Normalize(toPlayer);
    DirectX::XMStoreFloat3(&backwardDirection, toPlayer);

    // 地上移動のためXZ平面に平坦化
    backwardDirection.y = 0.0f;

    // 平坦化後再度正規化
    DirectX::XMVECTOR flatBackward = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&backwardDirection));
    DirectX::XMStoreFloat3(&backwardDirection, flatBackward);
}