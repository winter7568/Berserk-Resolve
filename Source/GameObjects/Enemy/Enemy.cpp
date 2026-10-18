#include <imgui.h>
#include <functional>
#include <algorithm>

// エンジンの基本システム
#include "GameLib/Graphics.h"
#include "GameLib/GamePad.h"
#include "GameLib/Input.h"

// ゲームの共通システム
#include "GameSystems/Collision/Collision.h"

// ゲーム内オブジェクト
#include "Enemy.h"

// 敵の行動ステート
#include "StateMachine/EnemyStates/HitStates.h"
#include "StateMachine/EnemyStates/PositioningState.h"

// 補助ユーティリティ
#include "GameLib/TransformUtils.h"

void Enemy::Initialize()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();

	// 敵モデルと剣モデルを読み込む
	LoadModels(device);

	// ディゾルブエフェクト設定
	SetupDissolveEffect(device);

	// 初期位置・回転・スケール
	SetupInitialTransform();

	// アニメーション開始（待機状態）
	StartAnimation(GetAnimIndex(EnemyAnim::IDLE_COMBAT), true);
	model->GetNodePoses(nodePoses);

	// 剣の初期姿勢を適用
	ApplyDefaultSwordPose();

	// コライダー設定
	SetupColliders();

	// ステート管理初期化
	SetupStateManager();

	// ステータス初期化
	SetupStats();

	// トレイルシステム初期化
	SetupTrailSystem(device);

	// サウンド読み込み
	LoadAudio();
}

void Enemy::Update(float elapsedTime)
{
	// シネマティック出現処理中かチェック
	if (cinematicSpawnActive) {
		UpdateCinematicSpawn(elapsedTime);
	}

	// 出現演出が完了している場合のみAIステートを更新
	if (cinematicSpawnComplete) {
		stateManager.Update(elapsedTime);
	}

	// アニメーションの更新（ブレンドや補間）
	model->UpdateAnimation(elapsedTime);

	// ルートモーションによる移動処理
	RootMotion(elapsedTime);

	// 発光フラッシュの更新処理
	UpdateEmissiveFlash(elapsedTime);

	// アニメーション時間の更新
	AdvanceAnimationTime(elapsedTime);

	// ワールド変換行列の更新
	UpdateTransform();

	// 武器（剣）を手のボーンに追従させる
	UpdateWeaponAttachment();

	// 剣のトレイルエフェクト更新
	swordTrail.Update(elapsedTime, sword.transform);
}

void Enemy::Reset()
{
	// 位置・速度・回転の初期化
	ResetTransform();

	// 体力や状態の初期化
	ResetStats();

	// ステートをすべてクリア
	ResetState();

	// 攻撃関連の変数を初期化
	ResetAttackState();

	// 戦闘フラグの初期化
	ResetCombatFlags();

	// パワーチャージなどのエフェクト停止
	StopPowerChargeEffect();

	// 出現状態・ダウン状態のリセット
	ResetSpawnAndDownState();
}

void Enemy::Render(const RenderContext& rc)
{
	if (!isVisible) return;

	ID3D11DeviceContext* dc = rc.deviceContext;

	// 使用するシェーダーを取得
	Shader* shader = Graphics::Instance().GetShader(ShaderId::PBR);

	// 描画開始
	shader->Begin(rc);

	// 本体モデル描画
	shader->Draw(rc, model.get());

	// 武器モデル描画
	shader->Draw(rc, sword.model.get());

	// 描画終了
	shader->End(rc);
}


// ============================================================================
// PUBLIC METHODS - Misc
// ============================================================================

EnemyPositioningState& Enemy::GetPositioningBrain() const
{
	return EnemyPositioningState::Instance();
}

// ============================================================================
// PROTECTED METHODS - Overrides
// ============================================================================

void Enemy::RootMotion(float elapsedTime)
{
	if (animationIndex < 0)
		return;

	// アニメーションからルートモーションを取得して移動に反映する
	if (animationIndex >= 0)
	{
		const Model::Animation& animation = model->GetAnimations().at(animationIndex);

		// 現在時間のポーズを計算
		model->ComputeAnimation(animationIndex, animationSeconds, nodePoses);

		// ルートモーションの移動量を計算
		CalculateRootMotion(animation);

		// ノードポーズをモデルに反映
		model->SetNodePoses(nodePoses);
	}
}
// ============================================================================
// PRIVATE METHODS - Update Helpers
// ============================================================================


void Enemy::CalculateRootMotion(const Model::Animation& animation)
{
	// ルートボーンのインデックス取得
	const int rootMotionNodeIndex = model->GetNodeIndex("root");

	// 開始・前フレーム・現在のポーズを取得
	Model::NodeCache beginPose, oldPose, newPose;
	model->ComputeAnimation(animationIndex, rootMotionNodeIndex, 0.0f, beginPose);
	model->ComputeAnimation(animationIndex, rootMotionNodeIndex, oldAnimationSeconds, oldPose);
	model->ComputeAnimation(animationIndex, rootMotionNodeIndex, animationSeconds, newPose);

	// ローカル空間での移動差分
	DirectX::XMFLOAT3 localTranslation{};

	// ループ時の補正処理
	if (oldAnimationSeconds > animationSeconds)
	{
		Model::NodeCache endPose;
		model->ComputeAnimation(animationIndex, rootMotionNodeIndex, animation.secondsLength, endPose);

		localTranslation.x = (newPose.position.x - beginPose.position.x) + (endPose.position.x - oldPose.position.x);
		localTranslation.y = (newPose.position.y - beginPose.position.y) + (endPose.position.y - oldPose.position.y);
		localTranslation.z = (newPose.position.z - beginPose.position.z) + (endPose.position.z - oldPose.position.z);
	}
	else
	{
		// 通常フレーム間の移動差分
		localTranslation = {
			newPose.position.x - oldPose.position.x,
			newPose.position.y - oldPose.position.y,
			newPose.position.z - oldPose.position.z,
		};
	}

	DirectX::XMVECTOR LocalTranslation = DirectX::XMLoadFloat3(&localTranslation);

	// ルートボーンを初期位置にリセット
	nodePoses[rootMotionNodeIndex].position = beginPose.position;

	// ワールド空間へ変換（座標反転込み）
	DirectX::XMMATRIX worldTransform = DirectX::XMLoadFloat4x4(&transform);
	DirectX::XMMATRIX coordinateFlip = DirectX::XMMatrixScaling(-1, 1, 1);
	DirectX::XMVECTOR worldTranslation = DirectX::XMVector3TransformNormal(LocalTranslation, coordinateFlip * worldTransform);

	// 移動量を格納
	DirectX::XMFLOAT3 motionDelta;
	DirectX::XMStoreFloat3(&motionDelta, worldTranslation);

	// 状態に応じて移動スケールを適用
	ApplyMovementScales(motionDelta);

	// 計算した移動を適用
	position.x += motionDelta.x;
	position.y += motionDelta.y;
	position.z += motionDelta.z;
}

void Enemy::ApplyMovementScales(DirectX::XMFLOAT3& motionDelta)
{
	// ダッシュ中は移動量を増加
	bool isDashing = stateManager.IsInAnyState({
		EnemyStateID::DASH,
		EnemyStateID::POWER_CHARGE
		});
	if (isDashing)
	{
		motionDelta.x *= EnemyConstants::DASH_DISTANCE_SCALE;
		motionDelta.z *= EnemyConstants::DASH_DISTANCE_SCALE;

		if (isGround)	motionDelta.y *= EnemyConstants::DASH_DISTANCE_SCALE;
		else			motionDelta.y = 0;
	}

	// コンボ攻撃中は移動量を減少
	bool isCombo = stateManager.IsInState(EnemyStateID::COMBO_ATTACK);
	if (isCombo)
	{
		motionDelta.x *= EnemyConstants::COMBO_DISTANCE_SCALE;
		motionDelta.z *= EnemyConstants::COMBO_DISTANCE_SCALE;
		motionDelta.y = 0;
	}

	// 通常攻撃中は少し移動を速くする
	bool isAttacking = stateManager.IsInState(EnemyStateID::ATTACK);
	if (isAttacking)
	{
		motionDelta.x *= EnemyConstants::ATTACK_RUN_SCALE;
		motionDelta.z *= EnemyConstants::ATTACK_RUN_SCALE;
		motionDelta.y = 0;
	}
}