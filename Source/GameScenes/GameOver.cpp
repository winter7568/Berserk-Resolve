#include "GameLib/Graphics.h"
#include "GameLib/GamePad.h"
#include "GameLib/Input.h"
#include "GameSystems/UI/UIManager.h"
#include "GameOver.h"
#include "SceneLoading.h"
#include "SceneTitle.h"
#include "SceneManager.h"


void GameOver::Initialize()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	UIManager::GetInstance().Initialize(device);

	sprite = std::make_unique<Sprite>(device, GameOverConst::BACKGROUND);
	UIManager::GetInstance().LoadTexture(PRESS_TEXTURE);
	UIManager::GetInstance().AddUIRegion(
		PRESS_REGION,
		PRESS_TEXTURE,
		PRESS_CROP_X,
		PRESS_CROP_Y,
		PRESS_CROP_W,
		PRESS_CROP_H,
		false);

	timer = 0.0f;
}

void GameOver::Finalize()
{
}

void GameOver::Update(float elapsedTime)
{
	GamePad& gamepad = Input::Instance().GetGamePad();

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

	// Update timer
	timer += elapsedTime;

	// Check if 3 seconds have passed
	if (timer >= AUTO_CHANGE_TIME)
	{
		SceneManager::Instance().ChangeScene(new SceneLoading(new SceneTitle));
		return; // Exit early since we're changing scenes
	}
}

void GameOver::Render()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();

	ID3D11SamplerState* samplers[] =
	{
		renderState->GetSamplerState(SamplerState::PointClamp)
	};
	dc->PSSetSamplers(0, _countof(samplers), samplers);


	// 描画準備
	RenderContext rc;
	rc.deviceContext = dc;
	rc.renderState = graphics.GetRenderState();

	float screenWidth = Graphics::Instance().GetScreenWidth();
	float screenHeight = Graphics::Instance().GetScreenHeight();

	// ブレンドステート設定 
	FLOAT blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), blendFactor, 0xFFFFFFFF);

	// 深度ステンシルステート設定 
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::NoTestNoWrite), DEPTH_NO_TEST_NO_WRITE);

	// ラスタライザーステート設定 
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	sprite->Render(
		dc,
		GameOverConst::BG_X,
		GameOverConst::BG_Y,
		0,
		screenWidth,
		screenHeight,
		0, 1, 1, 1, 1);

	{
		dc->OMSetBlendState(renderState->GetBlendState(BlendState::Additive), blendFactor, 0xFFFFFFFF);

		// Calculate pulsing scale based on time (0.8 to 1.2 range with 2 second period)
		float pulse = sinf(timer * PULSE_PERIOD) * PULSE_AMPLITUDE + 1.0f;
		float pulseWidth = PRESS_BASE_W * pulse;
		float pulseHeight = PRESS_BASE_H * pulse;

		// Render Start Text using ImGui-controlled values
		UIManager::GetInstance().RenderRegion(
			dc,
			PRESS_REGION,
			GameOverConst::PRESS_X,
			PRESS_Y,
			pulseWidth,
			pulseHeight);
	}
}

void GameOver::DrawGUI()
{
}
