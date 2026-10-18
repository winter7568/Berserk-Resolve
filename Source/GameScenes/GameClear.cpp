#include "GameLib/Graphics.h"
#include "GameLib/GamePad.h"
#include "GameLib/Input.h"
#include "GameSystems/UI/UIManager.h"
#include "GameClear.h"
#include "SceneLoading.h"
#include "SceneTitle.h"
#include "SceneManager.h"

void GameClear::Initialize()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();

	// UI初期化
	UIManager::GetInstance().Initialize(device);

	// 背景スプライト読み込み
	sprite = std::make_unique<Sprite>(device, GameClearConst::BACKGROUND);

	// テクスチャ読み込み
	UIManager::GetInstance().LoadTexture(PRESS_TEXTURE);
	UIManager::GetInstance().AddUIRegion(
		PRESS_REGION,
		PRESS_TEXTURE,
		PRESS_CROP_X,
		PRESS_CROP_Y,
		PRESS_CROP_W,
		PRESS_CROP_H,
		false);

	// タイマー初期化
	timer = 0.0f;
}

void GameClear::Finalize()
{
}

void GameClear::Update(float elapsedTime)
{
	GamePad& gamepad = Input::Instance().GetGamePad();

	// ボタン入力チェック
	const GamePadButton anyButton =
		GamePad::BTN_A
		| GamePad::BTN_B
		| GamePad::BTN_X
		| GamePad::BTN_Y
		;

	if (gamepad.GetButtonDown() & anyButton)
	{
		SceneManager::Instance().ChangeScene(new SceneLoading(new SceneTitle));
	}

	// タイマー更新
	timer += elapsedTime;

	// 一定時間後にシーン変更
	if (timer >= AUTO_CHANGE_TIME)
	{
		SceneManager::Instance().ChangeScene(new SceneLoading(new SceneTitle));
		return;
	}
}

void GameClear::Render()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();

	// サンプラー設定
	ID3D11SamplerState* samplers[] =
	{
		renderState->GetSamplerState(SamplerState::PointClamp)
	};
	dc->PSSetSamplers(0, _countof(samplers), samplers);

	// 描画設定
	RenderContext rc;
	rc.deviceContext = dc;
	rc.renderState = graphics.GetRenderState();

	// ブレンド設定
	FLOAT blendFactor[4] = { 1,1,1,1 };
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), blendFactor, 0xFFFFFFFF);

	// 深度設定
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::NoTestNoWrite), DEPTH_NO_TEST_NO_WRITE);

	// ラスタライザ設定
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	// 背景描画
	sprite->Render(dc, GameClearConst::BG_X, GameClearConst::BG_Y, 0, GameClearConst::SCREEN_WIDTH, GameClearConst::SCREEN_HEIGHT, 0, 1, 1, 1, 1);

	{
		// 加算ブレンド
		dc->OMSetBlendState(renderState->GetBlendState(BlendState::Additive), blendFactor, 0xFFFFFFFF);

		// 点滅サイズ計算
		float pulse = sinf(timer * PULSE_PERIOD) * PULSE_AMPLITUDE + 1.0f;
		float pulseWidth = PRESS_BASE_W * pulse;
		float pulseHeight = PRESS_BASE_H * pulse;

		// UI描画
		UIManager::GetInstance().RenderRegion(
			dc,
			PRESS_REGION,
			GameClearConst::PRESS_X,
			PRESS_Y,
			pulseWidth,
			pulseHeight);
	}
}

void GameClear::DrawGUI()
{
}