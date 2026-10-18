#include "PlayerAttackState.h"
#include "PlayerProjectileAttackState.h"
#include "StateMachine/PlayerStates/MoveState.h"
#include "StateMachine/PlayerStates/AttackStates/PlayerCounterState.h"
#include "GameLib/Input.h"
#include "GameSystems/HitManagement/HitManager.h"
#include "GameObjects/Player/Player.h"
#include "GameObjects/Enemy/Enemy.h"

// ============================================================================
// PlayerAttackState - 近接攻撃
// ============================================================================

void PlayerAttackState::Enter(Player& player)
{
    currentAttack = CharacterConstants::FIRST_ATTACK;
    queuedInputs = 0;
    inputBufferTime = 0.0f;
    hasHitThisAttack = false;
    effectPlayedForSecondAttack = false;
    // プレイヤーの攻撃追跡を更新
    player.SetCurrentAttackIndex(currentAttack);
    player.SetCurrentAttackStateID(PlayerStateID::ATTACK);
    // 最初の攻撃を再生
    player.StartAnimation(player.GetAnimIndex(attackAnimations[currentAttack]), false);
}

void PlayerAttackState::Update(Player& player, float elapsedTime)
{
    // 近接攻撃範囲内の場合のみ敵を自動追尾 カメラモードに関係なく
    player.AutoFaceEnemy(elapsedTime, PlayerConstants::MELEE_MAX_AUTO_FACE_DISTANCE);

    const AttackData& atk = attackData[currentAttack];
    int currentFrame = static_cast<int>(player.animationSeconds * CharacterConstants::FRAME_RATE);
    // ヒットウィンドウ状態を更新
    UpdateHitWindow(currentFrame);

    // 80% 移動オーバーライドチェック
    if (CheckMoveOverride(player)) {
        return;
    }

    // 入力オーバーライドチェック（遠距離、ジャンプ、ダッシュ）
    if (CheckInputOverrides(player)) {
        return;
    }

    // ヒット処理
    ProcessHit(player);

    // 2回目攻撃のエフェクトチェック
    CheckSecondAttackEffect(currentFrame);

    // 入力バッファ更新
    UpdateInputBuffer(elapsedTime);

    // 早期入力 / レスポンシブ短縮
    if (CheckEarlyCancel(player, currentFrame)) {
        return;
    }

    // キャンセルウィンドウ通常チェーン
    if (CheckCancelWindow(player, currentFrame)) {
        return;
    }

    // アニメーション終了
    if (currentFrame >= atk.totalFrames) {
        FinishAttack(player);
    }
}

void PlayerAttackState::Exit(Player& player)
{
    // 攻撃サウンドを停止
    player.StopPlayerSound(PlayerSound::SWING);
    // 状態変数をリセット
    queuedInputs = 0;
    currentAttack = CharacterConstants::FIRST_ATTACK;
    hasHitThisAttack = false;
    isInHitWindow = false;
    effectPlayedForSecondAttack = false;
    swingSoundPlayedForThisAttack = false;
    // 武器ヒット検出を無効化
    player.SetWeaponHitActive(false);
    player.DisableHitbox(currentAttack);
    // プレイヤーの攻撃追跡をリセット
    player.SetCurrentAttackIndex(CharacterConstants::FIRST_ATTACK);
    player.SetCurrentAttackStateID(PlayerStateID::NONE);
}

// ============================================================================
// PlayerAttackState - ヘルパー関数
// ============================================================================

bool PlayerAttackState::CheckMoveOverride(Player& player)
{
    const Model::Animation& currentAnim = player.model->GetAnimations().at(player.animationIndex);
    float animationProgress = player.animationSeconds / currentAnim.secondsLength;
    // 80% 移動オーバーライドチェック
    if (animationProgress >= PlayerConstants::ANIMATION_PROGRESS_THRESHOLD) {
        DirectX::XMVECTOR worldDir;
        bool isMoving = player.InputMove(worldDir);
        if (isMoving) {
            player.GetStateManager().PopState();
            player.GetStateManager().PushState(&PlayerRunState::Instance());
            return true;
        }
    }
    return false;
}

bool PlayerAttackState::CheckInputOverrides(Player& player)
{
    // カウンター状態に入れるかチェック
    if (player.CanEnterCounterState() && player.InputDash()) {
        player.GetStateManager().PushState(&PlayerCounterState::Instance());
        return true;
    }
    // 遠距離攻撃入力チェック
    if (player.InputProjectileAttack()) {
        player.GetStateManager().PushState(&PlayerProjectileAttackState::Instance());
        return true;
    }
    // ジャンプ入力チェック
    if (player.InputJump() && player.isGround) {
        player.GetStateManager().PopState();
        player.GetStateManager().PushState(&PlayerJumpState::Instance());
        return true;
    }
    // ダッシュ入力チェック
    if (player.InputDash()) {
        player.GetStateManager().PushState(&PlayerDashState::Instance());
        return true;
    }
    return false;
}

void PlayerAttackState::ProcessHit(Player& player)
{
    // クリーンなヒット管理
    bool shouldBeActive = isInHitWindow && !hasHitThisAttack;
    player.SetWeaponHitActive(shouldBeActive);

    if (shouldBeActive) {
        player.EnableHitbox(currentAttack);
        if (!swingSoundPlayedForThisAttack) {
            player.PlayPlayerSound(PlayerSound::SWING, PlayerConstants::SWING_SOUND_VOLUME);
            swingSoundPlayedForThisAttack = true;
        }
        auto& hitManager = HitManager::Instance();
        bool hitOccurred = hitManager.ProcessWeaponHit(player, Enemy::Instance());
        if (hitOccurred) {
            hasHitThisAttack = true;
        }
    }
}

void PlayerAttackState::CheckSecondAttackEffect(int currentFrame)
{
    const AttackData& atk = attackData[currentAttack];
    // 2回目攻撃のエフェクトチェック
    if (currentAttack == CharacterConstants::SECOND_ATTACK) {
        // キャンセルウィンドウを過ぎていてエフェクト未再生の場合
        if (currentFrame > atk.cancelEnd && !effectPlayedForSecondAttack) {
            // この攻撃のエフェクトを再生済みとしてマーク
            effectPlayedForSecondAttack = true;
        }
    }
}

void PlayerAttackState::UpdateInputBuffer(float elapsedTime)
{
    bool mouseAttack = Input::Instance().GetMouse().GetButtonDown() == Mouse::BTN_LEFT;
    bool controllerAttack = (Input::Instance().GetGamePad().GetButtonDown() & GamePad::BTN_X) != 0;
    if (mouseAttack || controllerAttack) {
        queuedInputs++;
        inputBufferTime = 0.0f;
    }
    inputBufferTime += elapsedTime;
    // 古すぎるバッファは期限切れ
    if (queuedInputs == 0 && inputBufferTime >= PlayerConstants::MAX_BUFFER_DURATION) {
        inputBufferTime = 0.0f;
    }
}

bool PlayerAttackState::CheckEarlyCancel(Player& player, int currentFrame)
{
    const AttackData& atk = attackData[currentAttack];
    // 早期入力 / レスポンシブ短縮
    if (queuedInputs > 0 && currentAttack < CharacterConstants::FOURTH_ATTACK) {
        // キャンセルウィンドウ内で入力があった場合
        if (currentFrame >= atk.cancelStart && currentFrame < atk.cancelEnd) {
            // 現在の攻撃をキャンセル終了フレームに移動
            player.animationSeconds = (float)atk.cancelEnd / CharacterConstants::FRAME_RATE;
            // 1つの入力のみ消費 全てはクリアしない
            queuedInputs = 0;
            currentAttack++;
            // 重要 プレイヤーと即座に同期
            player.SetCurrentAttackIndex(currentAttack);
            hasHitThisAttack = false;
            swingSoundPlayedForThisAttack = false;
            UpdateHitWindow(0);
            player.StartAnimation(player.GetAnimIndex(attackAnimations[currentAttack]), false);
            return true;
        }
    }
    return false;
}

bool PlayerAttackState::CheckCancelWindow(Player& player, int currentFrame)
{
    const AttackData& atk = attackData[currentAttack];
    // キャンセルウィンドウ通常チェーン
    if (atk.hasCancel && queuedInputs > 0 && currentFrame >= atk.cancelStart && currentFrame <= atk.cancelEnd) {
        // 1つの入力のみ消費 全てはクリアしない
        queuedInputs = 0;
        currentAttack++;
        // 重要 プレイヤーと即座に同期
        player.SetCurrentAttackIndex(currentAttack);
        hasHitThisAttack = false;
        swingSoundPlayedForThisAttack = false;
        UpdateHitWindow(0);
        player.StartAnimation(player.GetAnimIndex(attackAnimations[currentAttack]), false);
        return true;
    }
    return false;
}

void PlayerAttackState::FinishAttack(Player& player)
{
    const AttackData& atk = attackData[currentAttack];
    // これが攻撃4の場合 コンボをリセット
    if (currentAttack == CharacterConstants::FOURTH_ATTACK) {
        HitManager::Instance().ResetCombo();
    }
    // コンボ終了
    player.GetStateManager().PopState();
    // 攻撃終了後の移動入力をチェック
    DirectX::XMVECTOR worldDir;
    bool isMoving = player.InputMove(worldDir);
    if (isMoving) {
        // プレイヤーが移動中ならラン状態に遷移
        player.model->PlayAnimation(player.GetAnimIndex(PlayerAnim::RUN_START), true, player.animationBlendSeconds);
        player.GetStateManager().PushState(&PlayerRunState::Instance());
    }
    else {
        // 移動入力がなければアイドル状態に遷移
        player.model->PlayAnimation(player.GetAnimIndex(PlayerAnim::IDLE_COMBAT), true, player.animationBlendSeconds);
        player.GetStateManager().PushState(&PlayerIdleState::Instance());
    }
}