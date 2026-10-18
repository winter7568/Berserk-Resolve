#pragma once
#include <wrl/client.h>

#include "Scene.h"
#include "GameLib/Graphics.h"
#include "GameLib/HDRTexture.h"
#include "CameraFiles/FreeCameraController.h" 
#include "CameraFiles/CameraController.h"
#include "CameraFiles/CinematicCameraController.h"
#include "GameSystems/Effect/Effect.h"
#include "ShaderSystems/PostEffect.h"
#include "ShaderSystems/FadeManager.h" 
#include "ShaderSystems/SkyBox.h"
#include "Audio/Audio.h"

// ゲームシーン
class SceneGame : public GameScene
{
public:
    SceneGame() {};
    ~SceneGame() override {};

    // シーンライフサイクル
    void Initialize() override;
    void Finalize() override;
    void Update(float elapsedTime) override;
    void Render() override;
    void DrawGUI() override;

    std::string GetName() const override { return "SceneGame"; }

    // パブリックアクセサ
    enum class CameraMode { Follow, Free, LockOn };
    CameraMode GetCurrentCameraMode() const { return currentCameraMode; }
    void SetCameraMode(CameraMode mode) { currentCameraMode = mode; }
    CameraController* GetFollowController() const { return cameraControllerFollow.get(); }
    void SetFollowController(std::unique_ptr<CameraController> controller) {
        cameraControllerFollow = std::move(controller);
    }

    // ライト設定のゲッター/セッター（デバッグ用）
    DirectX::XMFLOAT3 GetLightDirection() const { return lightDirection; }
    void SetLightDirection(const DirectX::XMFLOAT3& direction) { lightDirection = direction; }

    DirectX::XMFLOAT3 GetLightColor() const { return lightColor; }
    void SetLightColor(const DirectX::XMFLOAT3& color) { lightColor = color; }

    float GetLightIntensity() const { return lightIntensity; }
    void SetLightIntensity(float intensity) { lightIntensity = intensity; }

private:
    // カメラシステム
    CinematicCameraController  cinematicCamera{};
    FreeCameraController       cameraControllerFree{};
    CameraMode currentCameraMode = CameraMode::Follow;

    // カメラコントローラー
    std::unique_ptr<CameraController> cameraControllerFollow;

    // 環境
    Skybox   skybox{};
    std::unique_ptr<HDRTexture> EnvironmentMap = nullptr;
    bool glassBarrier = true;  // アリーナのゲート視覚効果

    // ライティング
    LightManager    lightManager{};
    // ライティング（デバッグで変更可能）
    DirectX::XMFLOAT3 lightDirection = {
        SceneGameConst::LIGHT_DIR_X,
        SceneGameConst::LIGHT_DIR_Y,
        SceneGameConst::LIGHT_DIR_Z
    };
    DirectX::XMFLOAT3 lightColor = {
        SceneGameConst::LIGHT_COLOR_R,
        SceneGameConst::LIGHT_COLOR_G,
        SceneGameConst::LIGHT_COLOR_B
    };
    float lightIntensity = SceneGameConst::LIGHT_INTENSITY;

    // 視覚効果
    std::unique_ptr<PostEffect> postEffect = nullptr;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> vignetteTextureSRV = nullptr;
    float vignetteIntensity = 0.0f;           // 現在の強さ（0.0から1.0）

    // シーン状態
    FadeManager     fadeManager{};
    bool isFadingOut = false;

    // シネマティックシーケンス
    bool hasCinematicPlayed = false;
    bool cinematicActive = false;

    // オーディオ
    AudioSource* Bgm = nullptr;
    bool bgmStopped = false;

    // シネマティックヘルパー
    void StartCinematicSequence();
    void EndCinematicSequence();
    bool HandleCinematic(float elapsedTime);

    // 初期化ヘルパー
    void ResetGameState();
    void SetupCamera(Graphics& graphics);
    void SetupEnvironment(ID3D11Device* device);
    void SetupVisualEffects(ID3D11Device* device, Graphics& graphics);
    void SetupGameEntities(ID3D11Device* device, Graphics& graphics);
    void SetupUI(ID3D11Device* device);
    void SetupAudio();
    void SetupInput();

    // 更新ヘルパー
    // コアシステム
    void UpdateTimeAndLighting(float elapsedTime);
    void UpdateCamera(float elapsedTime);
    void UpdateLockOn();
    void UpdateWorldEntities(float scaledTime, float elapsedTime)const;
    void UpdateManagers(float elapsedTime);

    // シーンフロー
    bool CheckSceneTransition();

    // ビジュアル
    void UpdateVisualEffects(float elapsedTime);
    void UpdateVignetteEffect(float elapsedTime);

    // 描画ヘルパー
    // セットアップ
    RenderContext PrepareRenderContext(Graphics& graphics);

    // パス1: シーン描画
    void RenderScenePass(RenderContext& rc, Graphics& graphics);
    void RenderShadowMap(RenderContext& rc);
    void RenderSkybox(RenderContext& rc);
    void RenderDebugPrimitives(RenderContext& rc, ShapeRenderer* shapeRenderer);

    // パス2: 輝度抽出（ブルーム用）
    void RenderLuminancePass(RenderContext& rc, Graphics& graphics);

    // パス3: ポストプロセス
    void RenderPostProcessPass(RenderContext& rc, Graphics& graphics);

    // パス4: トレイル描画
    void RenderTrailPass(RenderContext& rc, Graphics& graphics)const;

    // パス5: UI描画
    void RenderUIPass(RenderContext& rc, Graphics& graphics);
};