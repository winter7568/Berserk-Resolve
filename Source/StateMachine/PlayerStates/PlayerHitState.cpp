#include "PlayerHitState.h"
#include "MoveState.h"
#include "GameObjects/Enemy/Enemy.h"
#include "CameraFiles/CameraShake.h"

// ============================================================================
// PlayerRegularHitState
// ============================================================================
void PlayerRegularHitState::Enter(Player& player)
{
    // カメラシェイクを適用
    CameraShake::Instance().AddTrauma(PlayerConstants::REGULAR_HIT_TRAUMA);

    // ヒット時に空中にいたかどうかを保存
    wasInAir = !player.isGround;

    if (wasInAir)
    {
        // 空中ヒット シンプルな空中ヒットアニメーションのみ使用
        player.animationIndex = player.GetAnimIndex(PlayerAnim::HIT_AIR);
        timer = PlayerConstants::AIR_HIT_DURATION;
    }
    else
    {
        // 地上ヒット 方向別ヒットシステムを使用
        auto& hitData = player.GetLastHitData();
        std::string hitDirection = CalculateHitDirection(player, hitData);
        PlayDirectionalHitAnimation(player, hitDirection);
        timer = PlayerConstants::REGULAR_HIT_ANIM_DURATION;
    }

    // アニメーションを開始
    player.animationSeconds = 0.0f;
    player.oldAnimationSeconds = 0.0f;
    player.animationLoop = false;
    player.model->PlayAnimation(player.animationIndex, player.animationLoop, player.animationBlendSeconds);
}

void PlayerRegularHitState::Update(Player& player, float elapsedTime)
{
    // 死亡チェック
    if (player.GetHealth() <= 0 && !player.GetStateManager().IsInState(PlayerStateID::DEATH))
    {
        player.GetStateManager().PushState(&PlayerDeathState::Instance());
        return;
    }

    timer -= elapsedTime;

    if (timer <= 0.0f)
    {
        player.GetStateManager().PopState();

        // 移動状態に応じて適切な状態に戻る
        if (wasInAir || !player.isGround)
        {
            player.GetStateManager().PushState(&PlayerFallingState::Instance());
        }
        else
        {
            DirectX::XMVECTOR worldDir;
            bool isMoving = player.InputMove(worldDir);

            if (isMoving)
                player.GetStateManager().PushState(&PlayerRunState::Instance());
            else
                player.GetStateManager().PushState(&PlayerIdleState::Instance());
        }
    }
}

void PlayerRegularHitState::Exit(Player& player)
{
    // クリーンアップは不要
}

std::string PlayerRegularHitState::CalculateHitDirection(const Player& player, const Character::HitData& hitData)
{
    DirectX::XMFLOAT3 hitDir = hitData.hitDirection;

    // プレイヤーの前方方向を計算
    DirectX::XMFLOAT3 playerForward = {
        sin(player.GetAngle().y),
        0.0f,
        cos(player.GetAngle().y)
    };

    // ドット積とクロス積を計算
    float dotProduct = playerForward.x * hitDir.x + playerForward.z * hitDir.z;
    float crossY = playerForward.z * hitDir.x - playerForward.x * hitDir.z;

    // 投射物ヒットかどうかをチェック
    bool isProjectileHit = (hitData.hitType == Character::HitType::Regular &&
        hitData.comboStep == 0);

    // 投射物ヒットの場合はドット積のロジックを反転
    if (isProjectileHit)
        dotProduct = -dotProduct;

    std::string direction;

    // 明確な横方向ヒットの判定を優先
    if (fabs(crossY) > PlayerConstants::HIT_DIRECTION_SIDE_THRESHOLD)
    {
        // 強い横ヒット クロス積で方向を判定
        direction = (crossY > 0.0f) ? "R" : "L";
    }
    else if (dotProduct > PlayerConstants::HIT_DIRECTION_FRONT_THRESHOLD)
    {
        direction = "F";
    }
    else if (dotProduct < -PlayerConstants::HIT_DIRECTION_BACK_THRESHOLD)
    {
        direction = "B";
    }
    else if (crossY > 0.0f)
    {
        direction = "R";
    }
    else
    {
        direction = "L";
    }

    return direction;
}

void PlayerRegularHitState::PlayDirectionalHitAnimation(Player& player, const std::string& direction)
{
    std::string animName = "AS_Hit_Combat_" + direction + "_Seq_0";
    int hitAnim = player.model->GetAnimationIndex(animName.c_str());

    if (hitAnim == -1)
    {
        hitAnim = player.model->GetAnimationIndex("AS_Hit_Combat_F_Seq_0");
    }

    player.StartAnimation(hitAnim, false);
}

// ============================================================================
// PlayerLaunchHitState
// ============================================================================
void PlayerLaunchHitState::Enter(Player& player)
{
    // カメラシェイクを適用
    CameraShake::Instance().AddTrauma(PlayerConstants::LAUNCH_HIT_TRAUMA);

    // ヒット時に空中にいたかどうかを保存
    wasInAir = !player.isGround;

    if (wasInAir)
    {
        // 空中ヒット 落下フェーズを使用
        currentPhase = LaunchPhase::Falling;
    }
    else
    {
        // 地上ヒット 元の動作 打ち上げアニメーションを使用
        currentPhase = LaunchPhase::Launch;
    }

    phaseTimer = 0.0f;

    // 死亡アニメーションを再生 打ち上げ用
    int deathAnimIndex = player.GetAnimIndex(PlayerAnim::DEATH);
    player.StartAnimation(deathAnimIndex, false);

    // アニメーション時間を取得
    const auto& anim = player.model->GetAnimations().at(deathAnimIndex);
    phaseDuration = anim.secondsLength;
}

void PlayerLaunchHitState::Update(Player& player, float elapsedTime)
{
    // 死亡チェック
    if (player.GetHealth() <= 0 && !player.GetStateManager().IsInState(PlayerStateID::DEATH))
    {
        player.GetStateManager().PushState(&PlayerDeathState::Instance());
        return;
    }

    phaseTimer += elapsedTime;

    // 起き上がりフェーズ中はプレイヤー入力をチェック
    if (currentPhase == LaunchPhase::GetUp)
    {
        DirectX::XMVECTOR worldDir;
        bool isMoving = player.InputMove(worldDir);
        bool wantsToDash = player.InputDash();
        bool wantsToJump = player.InputJump() && player.isGround;

        // プレイヤーが何か入力したら早期に状態を終了
        if (isMoving || wantsToDash || wantsToJump)
        {
            player.GetStateManager().PopState();

            // 入力に応じて適切な状態に遷移
            if (wantsToDash)
            {
                player.GetStateManager().PushState(&PlayerDashState::Instance());
            }
            else if (wantsToJump && player.isGround)
            {
                player.GetStateManager().PushState(&PlayerJumpState::Instance());
            }
            else if (isMoving)
            {
                player.GetStateManager().PushState(&PlayerRunState::Instance());
            }
            else
            {
                player.GetStateManager().PushState(&PlayerIdleState::Instance());
            }
            return;
        }
    }

    switch (currentPhase)
    {
    case LaunchPhase::Falling:  // 空中ヒット 最初に地面に落下
    {
        // 重力を適用
        if (!player.isGround && player.position.y > 0.0f)
        {
            // 強い重力を適用
            player.velocity.y += player.gravity * elapsedTime * PlayerConstants::LAUNCH_FALL_GRAVITY_MULTIPLIER;
            player.position.y += player.velocity.y * elapsedTime;

            // 地面衝突チェック
            if (player.position.y <= 0.0f)
            {
                player.position.y = 0.0f;
                player.velocity.y = 0.0f;
                player.isGround = true;
            }
        }

        if (phaseTimer >= phaseDuration)
        {
            // 起き上がりアニメーションに遷移
            int getupIndex = player.GetAnimIndex(PlayerAnim::GET_UP);
            player.StartAnimation(getupIndex, false);

            currentPhase = LaunchPhase::GetUp;
            phaseTimer = 0.0f;

            // 起き上がりアニメーション時間を取得
            const auto& getupAnim = player.model->GetAnimations().at(getupIndex);
            phaseDuration = getupAnim.secondsLength;
        }
        break;
    }

    case LaunchPhase::Launch:  // 地上ヒット 元の動作
        if (phaseTimer >= phaseDuration)
        {
            // 地上ヒット 直接起き上がりに遷移 落下は不要
            int getupIndex = player.GetAnimIndex(PlayerAnim::GET_UP);
            player.StartAnimation(getupIndex, false);

            currentPhase = LaunchPhase::GetUp;
            phaseTimer = 0.0f;

            // 起き上がりアニメーション時間を取得
            const auto& getupAnim = player.model->GetAnimations().at(getupIndex);
            phaseDuration = getupAnim.secondsLength;
        }
        break;

    case LaunchPhase::GetUp:
        if (phaseTimer >= phaseDuration)
        {
            currentPhase = LaunchPhase::Finished;
            player.GetStateManager().PopState();

            // アイドル状態に戻る
            player.GetStateManager().PushState(&PlayerIdleState::Instance());
        }
        break;

    case LaunchPhase::Finished:
        break;
    }
}

void PlayerLaunchHitState::Exit(Player& player)
{
    // クリーンアップは不要
}

// ============================================================================
// PlayerStunState
// ============================================================================
void PlayerStunState::Enter(Player& player)
{
    // カメラシェイクを適用
    CameraShake::Instance().AddTrauma(PlayerConstants::STUN_HIT_TRAUMA);

    PlayerAnim hitAnim;
    if (!player.isGround)
    {
        hitAnim = PlayerAnim::HIT_AIR;
        stunTimer = PlayerConstants::AIR_STUN_DURATION;
    }
    else
    {
        hitAnim = PlayerAnim::HIT_LAUNCH;
        stunTimer = stunDuration;
    }

    // スタンアニメーションを再生
    player.StartAnimation(player.GetAnimIndex(hitAnim), false);
}

void PlayerStunState::Update(Player& player, float elapsedTime)
{
    // 死亡チェック
    if (player.GetHealth() <= 0 && !player.GetStateManager().IsInState(PlayerStateID::DEATH))
    {
        player.GetStateManager().PushState(&PlayerDeathState::Instance());
        return;
    }

    stunTimer -= elapsedTime;

    if (stunTimer <= 0.0f)
    {
        // スタン終了
        player.GetStateManager().PopState();
        if (!player.isGround)
            player.GetStateManager().PushState(&PlayerFallingState::Instance());
        else
            player.GetStateManager().PushState(&PlayerIdleState::Instance());
    }
}

void PlayerStunState::Exit(Player& player)
{
    // 速度をリセット
    player.velocity.x = 0.0f;
    player.velocity.z = 0.0f;
}

// ============================================================================
// PlayerDeathState
// ============================================================================

void PlayerDeathState::Enter(Player& player)
{
    // 死亡アニメーションを再生
    player.StartAnimation(player.GetAnimIndex(PlayerAnim::DEATH), false);

    // すべての移動を停止 水平移動のみ停止 
    player.velocity.x = 0.0f;
    player.velocity.z = 0.0f;

    deathTimer = CharacterConstants::DEATH_TIMER;
}

void PlayerDeathState::Update(Player& player, float elapsedTime)
{
    // プレイヤーが地上より上なら重力を適用
    if (!player.isGround && player.position.y > 0.0f)
    {
        // 強い重力を適用
        player.velocity.y += player.gravity * elapsedTime * PlayerConstants::DEATH_FALL_GRAVITY_MULTIPLIER;
        player.position.y += player.velocity.y * elapsedTime;

        // 地面衝突チェック
        if (player.position.y <= 0.0f)
        {
            player.position.y = 0.0f;
            player.velocity.y = 0.0f;
            player.isGround = true;
        }
    }

    deathTimer -= elapsedTime;

    // アニメーション終了時にディゾルブを開始
    if (deathTimer <= CharacterConstants::DEATH_DISSOLVE_START)
    {
        player.PlayPlayerSound(PlayerSound::DEATH, CharacterConstants::DEATH_SOUND_VOLUME);
        dissolveTimer += elapsedTime;

        // ディゾルブ値を正規化してスムーズステップで計算
        float t = dissolveTimer / CharacterConstants::DEATH_DISSOLVE_START;
        float dissolveValue = t * t * (CharacterConstants::SMOOTH_STEP_A - CharacterConstants::SMOOTH_STEP_B * t);

        // プレイヤーモデルと武器にディゾルブを適用
        player.model->SetDissolveThreshold(dissolveValue);
        player.staff.model->SetDissolveThreshold(dissolveValue);

        // 死亡時の赤いエッジグローを追加
        float glowIntensity = CharacterConstants::DEATH_GLOW_INTENSITY * (CharacterConstants::DEATH_GLOW_FADE_RATIO - dissolveValue);
        player.model->SetDissolveEdgeIntensity(glowIntensity);
        player.model->SetDissolveEdgeWidth(CharacterConstants::DEATH_EDGE_WIDTH);

        if (player.staff.model)
        {
            player.staff.model->SetDissolveEdgeIntensity(glowIntensity * CharacterConstants::DEATH_SWORD_EDGE_INTENSITY_RATIO);
            player.staff.model->SetDissolveEdgeWidth(CharacterConstants::DEATH_SWORD_EDGE_WIDTH);
        }
    }

    if (deathTimer <= 0.0f)
    {
        player.isDead = true;
    }
}

void PlayerDeathState::Exit(Player& player)
{
    player.StopPlayerSound(PlayerSound::DEATH);
    dissolveTimer = 0.0f;
}