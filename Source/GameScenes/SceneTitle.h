#pragma once
#include <memory>

#include "Scene.h"
#include "ShaderSystems/Sprite.h"
#include "ShaderSystems/SkyBox.h"
#include "CameraFiles/FreeCameraController.h" 
#include "GameSystems/UI/UIManager.h"
#include "Audio/Audio.h"
#include "SceneConstants.h"

class SceneTitle : public GameScene
{
public:
    SceneTitle() {}
    ~SceneTitle() override {}

    void Initialize() override;
    void Finalize() override;
    void Update(float elapsedTime) override;
    void Render() override;
    void DrawGUI() override;

    std::string GetName() const override { return "SceneTitle"; }

private:
    Skybox skybox{};
    FreeCameraController cameraController{};
    Camera camera{};
    LightManager lightManager{};

    // Start button - these CHANGE during hover animation
    float startTextPosX = SceneTitleConst::Display::START_X;
    float startTextPosY = SceneTitleConst::Display::START_Y;
    float startTextWidth = SceneTitleConst::Display::START_W;
    float startTextHeight = SceneTitleConst::Display::START_H;
    float startTextOriginalWidth = SceneTitleConst::Display::START_W;
    float startTextOriginalHeight = SceneTitleConst::Display::START_H;
    float startTextOriginalPosX = SceneTitleConst::Display::START_X;
    float startTextOriginalPosY = SceneTitleConst::Display::START_Y;

    // Mouse click area - these are CONFIGURABLE at runtime via debug UI
    int mouseClickX = SceneTitleConst::MouseArea::X;
    int mouseClickY = SceneTitleConst::MouseArea::Y;
    int mouseClickWidth = SceneTitleConst::MouseArea::W;
    int mouseClickHeight = SceneTitleConst::MouseArea::H;

    // Hover state
    bool isMouseOverStartButton = false;
    float hoverAnimationTime = 0.0f;

    float EaseOutQuad(float t)
    {
        return 1.0f - (1.0f - t) * (1.0f - t);
    }

    AudioSource* bgm = nullptr;
    AudioSource* clickSound = nullptr;

    bool shouldChangeScene = false;
    float sceneChangeTimer = 0.0f;
};