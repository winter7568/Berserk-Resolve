// 外部ライブラリ
#include <imgui.h>

// エンジンシステム
#include "GameLib/Input.h"
#include "GameLib/GpuResourceUtils.h"

// ゲームシステム
#include "GameSystems/Time/TimeManager.h"
#include "GameSystems/Collision/CollisionManager.h"
#include "GameSystems/HitManagement/HitManager.h"
#include "GameSystems/Projectile/ProjectileManager.h"
#include "GameSystems/Effect/EffectManager.h"
#include "GameSystems/Debug/DebugManager.h"
#include "GameSystems/Debug/DebugRenderer.h"
#include "GameSystems/UI/UIManager.h"
#include "GameSystems/Particle/ParticleManager.h"
#include "CameraFiles/LockOnController.h"

// ゲームオブジェクト
#include "GameObjects/Player/Player.h"
#include "GameObjects/Enemy/Enemy.h"
#include "GameObjects/Arena/Arena.h"

// ステートとシーン
#include "SceneGame.h"
#include "GameOver.h"
#include "GameClear.h"
#include "SceneLoading.h"
#include "SceneManager.h"

void SceneGame::Initialize()
{
	// 全体リセット
	ResetGameState();

	// グラフィック取得
	ID3D11Device* device = Graphics::Instance().GetDevice();
	Graphics& graphics = Graphics::Instance();

	// カメラ設定
	SetupCamera(graphics);

	// 環境設定
	SetupEnvironment(device);

	// エフェクト設定
	SetupVisualEffects(device, graphics);

	// キャラ生成
	SetupGameEntities(device, graphics);

	// Input設定
	SetupInput();

	// UI設定
	SetupUI(device);

	// 音設定
	SetupAudio();

	// フェード開始
	fadeManager.StartFade(true, SceneGameConst::FADE_IN_INTENSITY);
}

void SceneGame::Finalize()
{
	// 音を削除
	if (Bgm) {
		delete Bgm;
		Bgm = nullptr;
	}
	Enemy::Instance().CleanupAudio();
	Player::Instance().CleanupAudio();
	Input::Instance().GetMouse().ConfineToWindow(false);
	Input::Instance().GetMouse().SetVisible(false);
}

void SceneGame::Update(float elapsedTime)
{
	float scaledTime = TimeManager::Instance().GetScaledDeltaTime();

	// カットシーン処理
	if (HandleCinematic(elapsedTime))
		return;

	// 時間とライト更新
	UpdateTimeAndLighting(elapsedTime);

	// カメラ更新
	UpdateCamera(elapsedTime);

	// ロックオン更新
	UpdateLockOn();

	// キャラ更新
	UpdateWorldEntities(scaledTime, elapsedTime);

	// マネージャ更新
	UpdateManagers(elapsedTime);

	// シーン切り替え
	if (CheckSceneTransition())
		return;

	// エフェクト更新
	UpdateVisualEffects(elapsedTime);
}

void SceneGame::Render()
{
	Graphics& graphics = Graphics::Instance();

	// 描画用データ作成
	RenderContext rc = PrepareRenderContext(graphics);

	// シーン描画
	RenderScenePass(rc, graphics);

	// 輝度描画
	RenderLuminancePass(rc, graphics);

	// ポスト処理
	RenderPostProcessPass(rc, graphics);

	// 軌跡描画
	RenderTrailPass(rc, graphics);

	// UI描画
	RenderUIPass(rc, graphics);
}

void SceneGame::DrawGUI()
{
	//UIManager::GetInstance().DrawControlsDebugUI();
	//UIManager::GetInstance().DrawPromptsDebugUI();
}

// ヘルパー関数

void SceneGame::StartCinematicSequence() 
{
	// シネマ開始
	cinematicActive = true;

	// 弾を消す
	ParticleManager::Instance().StopStaffProjectileRendering();
	ProjectileManager::Instance().ClearAllProjectiles();

	// カメラ開始
	cinematicCamera.StartSequence(
		Player::Instance().GetPosition(),
		Enemy::Instance().GetPosition()
	);

	// プレイヤー非表示
	Player::Instance().SetVisibility(false);
	UIManager::GetInstance().SetPlayerUIVisible(false);

	// BGM再生
	Bgm->SetVolume(SceneGameConst::CINEMATIC_BGM_VOLUME);
	Bgm->Play(true);

	// 敵スポーン
	Enemy::Instance().StartCinematicSpawn();

	// エフェクト再生
	DirectX::XMFLOAT3 enemyPos = Enemy::Instance().GetPosition();
	EffectManager::Instance().PlayEffect("enemySpawn", enemyPos, SceneGameConst::EFFECT_SCALE);
	Enemy::Instance().PlayEnemySound(EnemySound::SPAWN, SceneGameConst::ENEMY_SPAWN_SOUND_VOLUME);

	// バリア無効
	glassBarrier = false;
}

void SceneGame::EndCinematicSequence()
{
	// シネマ終了
	cinematicActive = false;

	// 弾再開
	ParticleManager::Instance().StartStaffProjectileRendering();
	ProjectileManager::Instance().ClearAllProjectiles();

	// プレイヤー表示
	Player::Instance().SetVisibility(true);
	UIManager::GetInstance().SetPlayerUIVisible(true);
	UIManager::GetInstance().SetEnemyUIVisible(true);

	// マナ回復
	Player::Instance().ResetMana();
}

bool SceneGame::HandleCinematic(float elapsedTime)
{
	// シネマ開始チェック
	if (!cinematicActive && !hasCinematicPlayed && Arena::Instance().PlayerHasEntered()) {
		StartCinematicSequence();
		hasCinematicPlayed = true;
	}

	if (cinematicActive) {
		// カメラ更新
		cinematicCamera.Update(elapsedTime);
		cinematicCamera.ApplyToCamera(Camera::Instance());

		// 敵更新
		Enemy::Instance().UpdateCinematicSpawn(elapsedTime);

		// エフェクト更新
		EffectManager::Instance().Update(elapsedTime);

		// 終了チェック
		if (cinematicCamera.IsSequenceComplete()) {
			EndCinematicSequence();
		}
		return true;
	}
	return false;
}

void SceneGame::ResetGameState()
{
	// 時間リセット
	TimeManager::Instance().ResetTimeScale();
	isFadingOut = false;
	fadeManager.Reset();

	// キャラリセット
	Player::Instance().Reset();
	Enemy::Instance().Reset();

	// 弾と当たり判定リセット
	ProjectileManager::Instance().ClearAllProjectiles();
	HitManager::Instance().Reset();

	// ステージリセット
	Arena::Instance().Reset();

	// UIと状態リセット
	hasCinematicPlayed = false;
	UIManager::GetInstance().SetEnemyUIVisible(false);
	UIManager::GetInstance().SetPlayerUIVisible(true);
}

void SceneGame::SetupCamera(Graphics& graphics)
{
	float screenWidth = graphics.GetScreenWidth();
	float screenHeight = graphics.GetScreenHeight();

	// カメラ設定
	Camera::Instance().SetPerspectiveFov(
		DirectX::XMConvertToRadians(SceneGameConst::FOV_DEGREES),
		screenWidth / screenHeight,
		SceneGameConst::NEAR_PLANE,
		SceneGameConst::FAR_PLANE
	);

	// カメラ位置
	Camera::Instance().SetLookAt(
		{
			SceneGameConst::CAMERA_POS_X,
			SceneGameConst::CAMERA_POS_Y,
			SceneGameConst::CAMERA_POS_Z
		},
		{
			SceneGameConst::TARGET_X,
			SceneGameConst::TARGET_Y,
			SceneGameConst::TARGET_Z
		},
		{
			SceneGameConst::UP_X,
			SceneGameConst::UP_Y,
			SceneGameConst::UP_Z
		}
	);

	// コントローラ初期化
	cameraControllerFree.SyncCameraToController(Camera::Instance());
	cameraControllerFollow = std::make_unique<CameraController>();
}


void SceneGame::SetupEnvironment(ID3D11Device* device)
{
	// スカイボックス
	skybox.Initialize(device);

	// ステージ
	Arena::Instance().Initialize(device);

	// 環境マップ
	EnvironmentMap = std::make_unique<HDRTexture>();
	EnvironmentMap->Load(SceneGameConst::ENVIRONMENT_MAP);
	EnvironmentMap->Set(SceneGameConst::ENVIRONMENT_MAP_SLOT);
}

void SceneGame::SetupVisualEffects(ID3D11Device* device, Graphics& graphics)
{
	// ポストエフェクト
	postEffect = std::make_unique<PostEffect>(graphics.GetDevice());

	// パーティクル
	ParticleManager::Instance().Initialize(device);
	// デバッグ初期化
	DebugManager::Instance().Initialize();

	// エフェクト管理
	EffectManager::Instance().Initialize();

	// ビネットテクスチャ
	HRESULT hr = GpuResourceUtils::LoadTexture(
		device,
		SceneGameConst::VIGNETTE_TEXTURE,
		vignetteTextureSRV.GetAddressOf()
	);

	if (FAILED(hr))
	{
		// 読み込み失敗
		OutputDebugStringA("vignette load fail\n");
	}
}

void SceneGame::SetupGameEntities(ID3D11Device* device, Graphics& graphics)
{
	// プレイヤー
	Player::Instance().Initialize();

	// 敵
	Enemy::Instance().Initialize();

	// 弾管理
	ProjectileManager::Instance().Initialize(device);

	// デバッグ描画
	DebugRenderer::Instance().Initialize(graphics.GetShapeRenderer());
}

void SceneGame::SetupUI(ID3D11Device* device)
{
	// UI初期化
	UIManager::GetInstance().Initialize(device);

	// UI読み込み
	UIManager::GetInstance().LoadEnemyUi();
	UIManager::GetInstance().LoadPlayerUi();
	UIManager::GetInstance().LoadControlsUI();
}

void SceneGame::SetupAudio()
{
	// BGM読み込み
	Bgm = Audio::Instance().LoadAudioSource(SceneGameConst::BGM_PATH);
	bgmStopped = false;
}

void SceneGame::SetupInput()
{
	Mouse& mouse = Input::Instance().GetMouse();

	// マウスを非表示
	mouse.SetVisible(false);

	// ウィンドウ内に制限
	mouse.ConfineToWindow(true);

	// 中央に配置（最初の1回だけ）
	mouse.CenterCursor();
}

void SceneGame::UpdateTimeAndLighting(float elapsedTime)
{
	// ライト更新
	DirectionalLight directionalLight{};
	directionalLight.direction = lightDirection;
	directionalLight.color = {
		lightColor.x * lightIntensity,
		lightColor.y * lightIntensity,
		lightColor.z * lightIntensity
	};
	lightManager.SetDirectionalLight(directionalLight);

	// 時間更新
	TimeManager::Instance().Update(elapsedTime);
}

void SceneGame::UpdateCamera(float elapsedTime)
{
	// カメラ更新
	if (currentCameraMode == CameraMode::Free) {
		cameraControllerFree.Update();
		cameraControllerFree.SyncControllerToCamera(Camera::Instance());
	}
	else if (LockOnController::Instance().IsLockedOn()) {
		LockOnController::Instance().Update(elapsedTime);
		currentCameraMode = CameraMode::LockOn;
	}
	else {
		if (cameraControllerFollow) {
			DirectX::XMFLOAT3 target = Player::Instance().GetPosition();
			target.y += SceneGameConst::CAMERA_HEIGHT;
			cameraControllerFollow->SetTarget(target);
			cameraControllerFollow->Update(elapsedTime);
		}
		currentCameraMode = CameraMode::Follow;
	}
}

void SceneGame::UpdateLockOn()
{
	bool lockOnHeld = Player::Instance().InputLockOn();
	Enemy& enemy = Enemy::Instance();

	// ロックオン開始
	if (lockOnHeld && !LockOnController::Instance().IsLockedOn() &&
		Arena::Instance().PlayerHasEntered()) {

		LockOnController::Instance().SetTarget(&enemy);
		LockOnController::Instance().SetLockOnEnabled(true);
	}
	// ロックオン解除
	else if (!lockOnHeld && LockOnController::Instance().IsLockedOn()) {

		if (cameraControllerFollow) {
			DirectX::XMFLOAT3 playerPos = Player::Instance().GetPosition();
			float playerAngleY = Player::Instance().GetAngle().y;
			cameraControllerFollow->ResetToBehindPlayer(playerPos, playerAngleY);
		}

		LockOnController::Instance().SetLockOnEnabled(false);
	}
}

void SceneGame::UpdateWorldEntities(float scaledTime, float elapsedTime) const
{
	// ステージ更新
	Arena::Instance().Update(elapsedTime);

	if (glassBarrier)
		Arena::Instance().UpdateGateTrail(elapsedTime);

	// 当たり判定リセット
	CollisionManager::Instance().ResetFrameState();

	// プレイヤー更新
	Player::Instance().Update(scaledTime);

	// 敵更新
	Enemy& enemy = Enemy::Instance();

	if (enemy.GetStateManager().IsInState(EnemyStateID::POWER_CHARGE) ||
		enemy.GetStateManager().IsInState(EnemyStateID::BERSERK)) {

		enemy.Update(elapsedTime);
	}
	else {
		enemy.Update(scaledTime);
	}
}


void SceneGame::UpdateManagers(float elapsedTime)
{
	// BGM音量調整
	if (!bgmStopped && (Enemy::Instance().GetHealth() <= 0 || Player::Instance().GetHealth() <= 0)) {
		Bgm->SetVolume(SceneGameConst::DEATH_BGM_VOLUME);
		bgmStopped = true;
	}

	// 各システム更新
	ProjectileManager::Instance().Update(elapsedTime);
	CollisionManager::Instance().CheckPlayerEnemyCollision(Player::Instance(), Enemy::Instance());
	HitManager::Instance().Update(elapsedTime);
	EffectManager::Instance().Update(elapsedTime);
	fadeManager.Update(elapsedTime);
}

bool SceneGame::CheckSceneTransition()
{
	// フェード開始
	if (!isFadingOut) {
		if (Player::Instance().IsDead() || Enemy::Instance().IsDead()) {
			isFadingOut = true;
			fadeManager.StartFade(false, SceneGameConst::FADE_OUT_INTENSITY);
		}
	}

	// シーン変更
	if (isFadingOut && !fadeManager.IsFading()) {
		if (Player::Instance().IsDead())
			SceneManager::Instance().ChangeScene(new GameOver());
		else if (Enemy::Instance().IsDead())
			SceneManager::Instance().ChangeScene(new GameClear());
		return true;
	}
	return false;
}

void SceneGame::UpdateVisualEffects(float elapsedTime)
{
	// パーティクル更新
	ParticleManager::Instance().Update(elapsedTime);
	ParticleManager::Instance().SpawnBackgroundEnergyField(elapsedTime);

	// ビネット更新
	if (postEffect) {
		UpdateVignetteEffect(elapsedTime);
	}
}

void SceneGame::UpdateVignetteEffect(float elapsedTime)
{
	bool isCountering = Player::Instance().GetStateManager().IsInState(PlayerStateID::COUNTER);
	float targetIntensity = isCountering ? SceneGameConst::VIGNETTE_COUNTER_INTENSITY : 0.0f;

	// 補間
	if (vignetteIntensity < targetIntensity) {
		vignetteIntensity += SceneGameConst::VIGNETTE_FADE_SPEED * elapsedTime;
		if (vignetteIntensity > targetIntensity)
			vignetteIntensity = targetIntensity;
	}
	else if (vignetteIntensity > targetIntensity) {
		vignetteIntensity -= SceneGameConst::VIGNETTE_FADE_SPEED * elapsedTime;
		if (vignetteIntensity < targetIntensity)
			vignetteIntensity = targetIntensity;
	}

	// 設定
	postEffect->SetVignetteEnabled(vignetteIntensity > SceneGameConst::VIGNETTE_THRESHOLD);
	postEffect->SetVignetteIntensity(vignetteIntensity);
	postEffect->SetVignetteScale(SceneGameConst::VIGNETTE_SCALE);
	postEffect->SetScreenFadeAlpha(fadeManager.GetAlpha());
}

RenderContext SceneGame::PrepareRenderContext(Graphics& graphics)
{
	// 描画データ作成
	RenderContext rc{};
	rc.deviceContext = graphics.GetDeviceContext();
	rc.renderState = graphics.GetRenderState();
	rc.view = Camera::Instance().GetView();
	rc.projection = Camera::Instance().GetProjection();
	rc.lightManager = &lightManager;
	rc.shadowMap = graphics.GetShadowMap();
	rc.shadowColor = {
			SceneGameConst::SHADOW_COLOR,
			SceneGameConst::SHADOW_COLOR,
			SceneGameConst::SHADOW_COLOR
	};
	return rc;
}

void SceneGame::RenderScenePass(RenderContext& rc, Graphics& graphics)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	// バッファ設定
	graphics.GetFrameBuffer(FrameBufferId::Scene)->SetRenderTargets(dc);
	graphics.GetFrameBuffer(FrameBufferId::Scene)->Clear(dc, 0, 0, SceneGameConst::CLEAR_COLOR_BLUE, 1.0f);

	// シャドウ
	RenderShadowMap(rc);

	// スカイボックス
	RenderSkybox(rc);

	// オブジェクト描画
	Arena::Instance().Render(rc);
	Player::Instance().Render(rc);
	Enemy::Instance().Render(rc);
	ProjectileManager::Instance().Render(rc);

	// デバッグ描画
	RenderDebugPrimitives(rc, graphics.GetShapeRenderer());
}

void SceneGame::RenderShadowMap(RenderContext& rc)
{
	ShadowMap* shadowMap = rc.shadowMap;
	if (!shadowMap) return;

	Model* modelPlayer = Player::Instance().GetModel();
	Model* modelStaff = Player::Instance().GetWeaponModel();
	Model* modelEnemy = Enemy::Instance().GetModel();
	Model* modelSword = Enemy::Instance().GetWeaponModel();

	shadowMap->Begin(rc, Camera::Instance().GetFocus());
	shadowMap->Draw(rc, modelPlayer);
	shadowMap->Draw(rc, modelEnemy);
	shadowMap->Draw(rc, modelStaff);
	shadowMap->Draw(rc, modelSword);
	shadowMap->End(rc);
}

void SceneGame::RenderSkybox(RenderContext& rc)
{
	DirectX::XMMATRIX view = XMLoadFloat4x4(&rc.view);
	view.r[3] = DirectX::XMVectorSet(0, 0, 0, 1);

	DirectX::XMMATRIX proj = XMLoadFloat4x4(&rc.projection);
	DirectX::XMMATRIX viewProj = view * proj;

	skybox.Render(rc, viewProj);
}

void SceneGame::RenderDebugPrimitives(RenderContext& rc, ShapeRenderer* shapeRenderer)
{
	if (DebugManager::Instance().ShouldShowDebugPrimitives()) {
		Player::Instance().RenderDebugPrimitive(rc, shapeRenderer);
		Enemy::Instance().RenderDebugPrimitive(rc, shapeRenderer);
	}
}

void SceneGame::RenderLuminancePass(RenderContext& rc, Graphics& graphics)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	// バッファ設定
	ID3D11RenderTargetView* luminanceRTV =
		graphics.GetFrameBuffer(FrameBufferId::Luminance)->GetRenderTargetView();

	ID3D11DepthStencilView* sceneDepth =
		graphics.GetFrameBuffer(FrameBufferId::Scene)->GetDepthStencilView();

	dc->OMSetRenderTargets(1, &luminanceRTV, sceneDepth);

	// クリア
	float clearColor[4] = { 0,0,0,1 };
	dc->ClearRenderTargetView(luminanceRTV, clearColor);

	dc->RSSetViewports(1,
		&graphics.GetFrameBuffer(FrameBufferId::Scene)->GetViewport());

	// パーティクル描画
	ParticleManager::Instance().Render(rc);
}

void SceneGame::RenderPostProcessPass(RenderContext& rc, Graphics& graphics)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	// バッファ設定
	graphics.GetFrameBuffer(FrameBufferId::Display)->SetRenderTargets(dc);
	graphics.GetFrameBuffer(FrameBufferId::Display)->Clear(dc, 0, 0, 0, 1);

	// ポスト処理開始
	postEffect->Begin(rc);

	// ブルーム
	postEffect->Bloom(rc,
		graphics.GetFrameBuffer(FrameBufferId::Scene)->GetColorMap(),
		graphics.GetFrameBuffer(FrameBufferId::Luminance)->GetColorMap());

	// ビネット
	if (Player::Instance().GetStateManager().IsInState(PlayerStateID::COUNTER))
	{
		postEffect->ApplyVignette(rc,
			graphics.GetFrameBuffer(FrameBufferId::Scene)->GetColorMap(),
			vignetteTextureSRV.Get());
	}

	postEffect->End(rc);
}

void SceneGame::RenderTrailPass(RenderContext& rc, Graphics& graphics) const
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	// 描画チェック
	bool renderEnemyTrail = Enemy::Instance().GetStateManager().IsInAnyState({
		EnemyStateID::COMBO_ATTACK,
		EnemyStateID::POWER_CHARGE,
		EnemyStateID::STARFE_ATTACK,
		EnemyStateID::TELEPORT,
		EnemyStateID::ATTACK
		});

	bool renderPlayerTrail = Player::Instance().GetStateManager().IsInAnyState({
		PlayerStateID::IDLE,
		PlayerStateID::RUN,
		PlayerStateID::JUMP,
		PlayerStateID::FALLING
		});

	// バッファ設定
	ID3D11DepthStencilView* sceneDepth =
		graphics.GetFrameBuffer(FrameBufferId::Scene)->GetDepthStencilView();

	ID3D11RenderTargetView* displayRTV =
		graphics.GetFrameBuffer(FrameBufferId::Display)->GetRenderTargetView();

	dc->OMSetRenderTargets(1, &displayRTV, sceneDepth);

	// 軌跡描画
	if (renderEnemyTrail)
		Enemy::Instance().RenderSwordTrail(rc);

	if (!renderPlayerTrail)
		Player::Instance().RenderSwordTrail(rc);

	if (glassBarrier)
		Arena::Instance().RenderGateTrail(rc);
}

void SceneGame::RenderUIPass(RenderContext& rc, Graphics& graphics)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	// ブレンド設定
	const float blendFactor[4] = { 1,1,1,1 };
	dc->OMSetBlendState(
		rc.renderState->GetBlendState(BlendState::Transparency),
		blendFactor, 0xFFFFFFFF);

	// 敵HP
	int enemyHealth = Enemy::Instance().GetHealth();
	UIManager::GetInstance().RenderEnemyHealthBar(dc, static_cast<float>(enemyHealth), EnemyConstants::MAX_HEALTH);

	// プレイヤー情報
	int playerHealth = Player::Instance().GetHealth();
	float playerMaxHealth = PlayerConstants::MAX_HEALTH;
	float playerStamina = Player::Instance().GetCurrentMana();
	float playerMaxStamina = Player::Instance().GetMaxMana();

	UIManager::GetInstance().RenderPlayerUI(dc,
		static_cast<float>(playerHealth), playerMaxHealth,
		playerStamina, playerMaxStamina);

	// コントロール UI
	UIManager::GetInstance().RenderControlsUI(dc);
	UIManager::GetInstance().RenderAttackPrompt(dc);
	UIManager::GetInstance().RenderCounterPrompt(dc);
	// UIエフェクト
	EffectManager::Instance().Render(rc.view, rc.projection);

	// 元に戻す
	dc->OMSetBlendState(
		rc.renderState->GetBlendState(BlendState::Opaque),
		blendFactor, 0xFFFFFFFF);
}