#include <imgui.h>
#include "GameLib/Graphics.h"
#include "GameLib/Input.h"
#include "GameSystems/UI/UIManager.h"
#include "SceneLoading.h"
#include "SceneManager.h"

void SceneLoading::Initialize()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();

	// スプライト作成
	sprite = std::make_unique<Sprite>(device, SceneLoadingConst::SPRITE_PATH);

	// ロード用スレッド開始
	thread = new std::thread(LoadingThread, this);

	// UI初期化
	UIManager::GetInstance().Initialize(device);

	// テクスチャ読み込み
	UIManager::GetInstance().LoadTexture(SceneLoadingConst::CONTROLLER_TEXTURE);
	UIManager::GetInstance().AddUIRegion(
		SceneLoadingConst::CONTROLLER_REGION,
		SceneLoadingConst::CONTROLLER_TEXTURE,
		SceneLoadingConst::CONTROLLER_CROP_X,
		SceneLoadingConst::CONTROLLER_CROP_Y,
		SceneLoadingConst::CONTROLLER_CROP_W,
		SceneLoadingConst::CONTROLLER_CROP_H,
		false);

	// BGM再生
	bgm = Audio::Instance().LoadAudioSource(SceneLoadingConst::BGM_PATH);
	if (bgm)
	{
		bgm->SetVolume(SceneLoadingConst::BGM_VOLUME);
		bgm->Play(false);
	}

	// マウス非表示
	Mouse& mouse = Input::Instance().GetMouse();
	mouse.SetVisible(false);
	mouse.ConfineToWindow(false);
}

void SceneLoading::Finalize()
{
	// 音を解放
	delete bgm;

	// スレッド終了
	if (thread != nullptr)
	{
		thread->join();
		delete thread;
		thread = nullptr;
	}
}

void SceneLoading::Update(float elapsedTime)
{
	// 回転更新
	angle += SceneLoadingConst::ROTATION_SPEED * elapsedTime;

	// 読み込み完了でシーン切り替え
	if (nextScene->IsReady())
	{
		SceneManager::Instance().ChangeScene(nextScene);
		nextScene = nullptr;
	}
}

void SceneLoading::Render()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();

	RenderContext rc{};
	rc.deviceContext = dc;
	rc.renderState = graphics.GetRenderState();

	// ローディングアイコン描画
	{
		float screenWidth = static_cast<float>(graphics.GetScreenWidth());
		float screenHeight = static_cast<float>(graphics.GetScreenHeight());

		float positionX = screenWidth - SceneLoadingConst::SPRITE_SIZE;
		float positionY = screenHeight - SceneLoadingConst::SPRITE_SIZE;

		const float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		dc->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Additive), blendFactor, 0xFFFFFFFF);

		sprite->Render(
			dc,
			positionX,
			positionY,
			0,
			SceneLoadingConst::SPRITE_SIZE,
			SceneLoadingConst::SPRITE_SIZE,
			angle,
			1.0f, 1.0f, 1.0f, 1.0f);
	}

	// UI描画
	const float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	dc->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Additive), blendFactor, 0xFFFFFFFF);

	UIManager::GetInstance().RenderRegion(
		dc,
		SceneLoadingConst::CONTROLLER_REGION,
		SceneLoadingConst::CONTROLLER_POS_X,   
		SceneLoadingConst::CONTROLLER_POS_Y,
		SceneLoadingConst::CONTROLLER_WIDTH,
		SceneLoadingConst::CONTROLLER_HEIGHT);
}

void SceneLoading::DrawGUI()
{
	// デバッグUIなし
}

void SceneLoading::LoadingThread(SceneLoading* scene)
{
	// 初期化を別スレッドで実行
	CoInitialize(nullptr);

	scene->nextScene->Initialize();

	CoUninitialize();

	// 準備完了
	scene->nextScene->SetReady();
}