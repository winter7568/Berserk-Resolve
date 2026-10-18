#include "imgui.h"
#include "GameLib/Graphics.h"
#include "GameLib/GamePad.h"
#include "GameLib/Input.h"
#include "SceneTitle.h"		 
#include "SceneLoading.h"
#include "SceneGame.h"
#include "SceneManager.h"
#include "GameSystems/UI/UIManager.h"
#include "SceneConstants.h"

void SceneTitle::Initialize()
{
    ID3D11Device* device = Graphics::Instance().GetDevice();

    Graphics& graphics = Graphics::Instance();
    float screenWidth = Graphics::Instance().GetScreenWidth();
    float screenHeight = Graphics::Instance().GetScreenHeight();

    // カメラ設定
    camera.SetPerspectiveFov(
        DirectX::XMConvertToRadians(SceneTitleConst::FOV_DEGREES),
        screenWidth / screenHeight,
        SceneTitleConst::NEAR_PLANE,
        SceneTitleConst::FAR_PLANE
    );

    camera.SetLookAt(
        { SceneTitleConst::CAMERA_POS_X, SceneTitleConst::CAMERA_POS_Y, SceneTitleConst::CAMERA_POS_Z },
        { SceneTitleConst::TARGET_X, SceneTitleConst::TARGET_Y, SceneTitleConst::TARGET_Z },
        { SceneTitleConst::UP_X, SceneTitleConst::UP_Y, SceneTitleConst::UP_Z }
    );
    cameraController.SyncCameraToController(camera);

    // スカイボックス初期化
    skybox.Initialize(device);

    // ライト設定
    DirectionalLight directionalLight{};
    directionalLight.direction = {
        SceneTitleConst::LIGHT_DIR_X,
        SceneTitleConst::LIGHT_DIR_Y,
        SceneTitleConst::LIGHT_DIR_Z
    };
    directionalLight.color = {
        SceneTitleConst::LIGHT_COLOR_R,
        SceneTitleConst::LIGHT_COLOR_G,
        SceneTitleConst::LIGHT_COLOR_B
    };
    lightManager.SetDirectionalLight(directionalLight);

    // UI初期化
    UIManager::GetInstance().Initialize(device);

    // テクスチャ読み込み
    UIManager::GetInstance().LoadTexture(SceneTitleConst::TITLE_TEXTURE);
    UIManager::GetInstance().LoadTexture(SceneTitleConst::START_TEXTURE);
    UIManager::GetInstance().LoadTexture(SceneTitleConst::PRESS_O_TEXTURE);

    // UI登録
    UIManager::GetInstance().AddUIRegion(
        SceneTitleConst::TITLE_REGION,
        SceneTitleConst::TITLE_TEXTURE,
        SceneTitleConst::Crop::TITLE_X,
        SceneTitleConst::Crop::TITLE_Y,
        SceneTitleConst::Crop::TITLE_W,
        SceneTitleConst::Crop::TITLE_H,
        false
    );

    UIManager::GetInstance().AddUIRegion(
        SceneTitleConst::START_REGION,
        SceneTitleConst::START_TEXTURE,
        SceneTitleConst::Crop::START_X,
        SceneTitleConst::Crop::START_Y,
        SceneTitleConst::Crop::START_W,
        SceneTitleConst::Crop::START_H,
        false
    );

    UIManager::GetInstance().AddUIRegion(
        SceneTitleConst::PRESS_O_REGION,
        SceneTitleConst::PRESS_O_TEXTURE,
        SceneTitleConst::Crop::PRESS_O_X,
        SceneTitleConst::Crop::PRESS_O_Y,
        SceneTitleConst::Crop::PRESS_O_W,
        SceneTitleConst::Crop::PRESS_O_H,
        false
    );

    // ホバー用の初期値
    startTextOriginalWidth = startTextWidth;
    startTextOriginalHeight = startTextHeight;
    startTextOriginalPosX = startTextPosX;
    startTextOriginalPosY = startTextPosY;

    // 状態リセット
    hoverAnimationTime = 0.0f;
    isMouseOverStartButton = false;

    // BGM再生
    bgm = Audio::Instance().LoadAudioSource(SceneTitleConst::BGM_PATH);
    if (bgm)
    {
        bgm->SetVolume(SceneTitleConst::BGM_VOLUME);
        bgm->Play(true);
    }

    // クリック音読み込み
    clickSound = Audio::Instance().LoadAudioSource(SceneTitleConst::CLICK_SOUND_PATH);

    Mouse& mouse = Input::Instance().GetMouse();
    mouse.SetVisible(false);  
    mouse.ConfineToWindow(false);  
}

void SceneTitle::Finalize()
{
    delete clickSound;
    clickSound = nullptr;

    delete bgm;
    bgm = nullptr;
}

void SceneTitle::Update(float elapsedTime)
{
    // カメラ更新
    cameraController.AutoRotate(elapsedTime);
    cameraController.SyncControllerToCamera(camera);

    GamePad& gamepad = Input::Instance().GetGamePad();
    Mouse& mouse = Input::Instance().GetMouse();

    // マウス位置取得
    int mouseX = mouse.GetPositionX();
    int mouseY = mouse.GetPositionY();

    // ボタン上にあるかチェック
    isMouseOverStartButton =
        (mouseX >= mouseClickX && mouseX <= mouseClickX + mouseClickWidth &&
            mouseY >= mouseClickY && mouseY <= mouseClickY + mouseClickHeight);

    // コントローラー入力
    if (Input::Instance().GetGamePad().GetButton() & GamePad::BTN_B)
    {
        if (clickSound)
        {
            clickSound->Play(false);
            shouldChangeScene = true;
            sceneChangeTimer = SceneTitleConst::SCENE_CHANGE_DELAY;
        }
    }

    // ホバー処理
    if (isMouseOverStartButton)
    {
        hoverAnimationTime += elapsedTime * SceneTitleConst::HOVER_ANIMATION_SPEED;
        if (hoverAnimationTime > 1.0f) hoverAnimationTime = 1.0f;

        // マウスクリック
        if (mouse.GetButtonDown() & Mouse::BTN_LEFT && !shouldChangeScene)
        {
            if (clickSound)
            {
                clickSound->Play(false);
                shouldChangeScene = true;
                sceneChangeTimer = SceneTitleConst::SCENE_CHANGE_DELAY;
            }
        }
    }
    else
    {
        hoverAnimationTime -= elapsedTime * SceneTitleConst::HOVER_ANIMATION_SPEED;
        if (hoverAnimationTime < 0.0f) hoverAnimationTime = 0.0f;
    }

    // シーン切り替え
    if (shouldChangeScene)
    {
        sceneChangeTimer -= elapsedTime;
        if (sceneChangeTimer <= 0.0f)
        {
            SceneManager::Instance().ChangeScene(new SceneLoading(new SceneGame()));
            ShowCursor(true);
        }
    }

    // スケール計算
    float easeT = EaseOutQuad(hoverAnimationTime);
    float currentScale = 1.0f + (SceneTitleConst::HOVER_SCALE - 1.0f) * easeT;

    startTextWidth = startTextOriginalWidth * currentScale;
    startTextHeight = startTextOriginalHeight * currentScale;

    // 中心基準で位置調整
    float widthDiff = startTextWidth - startTextOriginalWidth;
    float heightDiff = startTextHeight - startTextOriginalHeight;
    startTextPosX = startTextOriginalPosX - widthDiff / 2.0f;
    startTextPosY = startTextOriginalPosY - heightDiff / 2.0f;
}

void SceneTitle::Render()
{
    Graphics& graphics = Graphics::Instance();
    ID3D11DeviceContext* dc = graphics.GetDeviceContext();

    // 描画設定
    RenderContext rc{};
    rc.deviceContext = dc;
    rc.renderState = graphics.GetRenderState();
    rc.view = camera.GetView();
    rc.projection = camera.GetProjection();
    rc.camera = &camera;
    rc.lightManager = &lightManager;

    // スカイボックス用行列
    DirectX::XMMATRIX view = XMLoadFloat4x4(&rc.view);
    DirectX::XMMATRIX proj = XMLoadFloat4x4(&rc.projection);
    view.r[3] = DirectX::XMVectorSet(0, 0, 0, 1);
    DirectX::XMMATRIX viewProj = view * proj;

    // スカイボックス描画
    skybox.Render(rc, viewProj);

    ID3D11SamplerState* samplers[] =
    {
        rc.renderState->GetSamplerState(SamplerState::PointClamp)
    };
    dc->PSSetSamplers(0, _countof(samplers), samplers);

    // 加算ブレンド設定
    const float blendFactor[4] = { 0, 0, 0, 0 };
    dc->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Additive), blendFactor, 0xffffffff);

    // UI描画
    UIManager::GetInstance().RenderRegion(
        dc,
        SceneTitleConst::TITLE_REGION,
        SceneTitleConst::Display::TITLE_X,
        SceneTitleConst::Display::TITLE_Y,
        SceneTitleConst::Display::TITLE_W,
        SceneTitleConst::Display::TITLE_H
    );

    UIManager::GetInstance().RenderRegion(
        dc,
        SceneTitleConst::START_REGION,
        startTextPosX,
        startTextPosY,
        startTextWidth,
        startTextHeight
    );

    UIManager::GetInstance().RenderRegion(
        dc,
        SceneTitleConst::PRESS_O_REGION,
        SceneTitleConst::Display::PRESS_O_X,
        SceneTitleConst::Display::PRESS_O_Y,
        SceneTitleConst::Display::PRESS_O_W,
        SceneTitleConst::Display::PRESS_O_H
    );
}

void SceneTitle::DrawGUI()
{
}