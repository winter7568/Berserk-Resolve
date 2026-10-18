// SceneConstants.h
#pragma once

// 共通定数（全シーンで共有）
constexpr float AUTO_CHANGE_TIME = 3.0f;

// Pressテクスチャ（GameClearとGameOverで共有）
constexpr const char* PRESS_TEXTURE = "Data/Sprite/PressZ.png";
constexpr const char* PRESS_REGION = "Press";
constexpr float PRESS_CROP_X = 24.0f;
constexpr float PRESS_CROP_Y = 15.0f;
constexpr float PRESS_CROP_W = 636.0f;
constexpr float PRESS_CROP_H = 80.0f;
constexpr float PRESS_BASE_W = 700.0f;
constexpr float PRESS_BASE_H = 100.0f;
constexpr float PRESS_Y = 826.0f;

// 点滅アニメーション
constexpr float PULSE_PERIOD = 3.14159f;
constexpr float PULSE_AMPLITUDE = 0.2f;

// 深度ステート
constexpr int DEPTH_NO_TEST_NO_WRITE = 0;

// GameClearシーン定数
namespace GameClearConst
{
    constexpr const char* BACKGROUND = "Data/Sprite/GameClear.png";
    constexpr float SCREEN_WIDTH = 1400.0f;
    constexpr float SCREEN_HEIGHT = 798.0f;
    constexpr float BG_X = 300.0f;
    constexpr float BG_Y = 80.0f;
    constexpr float PRESS_X = 920.0f;
}

// GameOverシーン定数
namespace GameOverConst
{
    constexpr const char* BACKGROUND = "Data/Sprite/Over.png";
    constexpr float BG_X = 0.0f;
    constexpr float BG_Y = 0.0f;
    constexpr float PRESS_X = 720.0f;
}

// ローディングシーン定数
namespace SceneLoadingConst
{
    constexpr const char* SPRITE_PATH = "Data/Sprite/Loading.png";
    constexpr float SPRITE_SIZE = 256.0f;
    constexpr float ROTATION_SPEED = 180.0f;

    constexpr const char* CONTROLLER_TEXTURE = "Data/Sprite/cont_ps5.png";
    constexpr const char* CONTROLLER_REGION = "Controller";
    constexpr float CONTROLLER_CROP_X = 40.0f;
    constexpr float CONTROLLER_CROP_Y = 60.0f;
    constexpr float CONTROLLER_CROP_W = 5206.0f;
    constexpr float CONTROLLER_CROP_H = 2927.0f;

    constexpr float CONTROLLER_POS_X = 336.0f;
    constexpr float CONTROLLER_POS_Y = 73.0f;
    constexpr float CONTROLLER_WIDTH = 1275.0f;
    constexpr float CONTROLLER_HEIGHT = 750.0f;

    constexpr const char* BGM_PATH = "Data/Audio/Loading.wav";
    constexpr float BGM_VOLUME = 0.5f;
}

// タイトルシーン定数
namespace SceneTitleConst
{
    // カメラ設定
    constexpr float FOV_DEGREES = 45.0f;
    constexpr float NEAR_PLANE = 0.1f;
    constexpr float FAR_PLANE = 1000.0f;

    constexpr float CAMERA_POS_X = 0.0f;
    constexpr float CAMERA_POS_Y = 5.0f;
    constexpr float CAMERA_POS_Z = -10.0f;

    constexpr float TARGET_X = 0.0f;
    constexpr float TARGET_Y = 0.0f;
    constexpr float TARGET_Z = 0.0f;

    constexpr float UP_X = 0.0f;
    constexpr float UP_Y = 1.0f;
    constexpr float UP_Z = 0.0f;

    // ライト設定
    constexpr float LIGHT_DIR_X = -1.0f;
    constexpr float LIGHT_DIR_Y = -1.0f;
    constexpr float LIGHT_DIR_Z = 0.0f;
    constexpr float LIGHT_COLOR_R = 1.0f;
    constexpr float LIGHT_COLOR_G = 1.0f;
    constexpr float LIGHT_COLOR_B = 1.0f;

    // テクスチャパス
    constexpr const char* TITLE_TEXTURE = "Data/Sprite/Title.png";
    constexpr const char* START_TEXTURE = "Data/Sprite/Start.png";
    constexpr const char* PRESS_O_TEXTURE = "Data/Sprite/PressO.png";

    // 音声パス
    constexpr const char* BGM_PATH = "Data/Audio/TitleBGM.wav";
    constexpr const char* CLICK_SOUND_PATH = "Data/Audio/ClickStart.wav";
    constexpr float BGM_VOLUME = 0.5f;

    // UIリージョン名
    constexpr const char* TITLE_REGION = "Title";
    constexpr const char* START_REGION = "Start";
    constexpr const char* PRESS_O_REGION = "Press";

    // テクスチャ切り取り範囲
    namespace Crop
    {
        constexpr float TITLE_X = 1025.0f;
        constexpr float TITLE_Y = 760.0f;
        constexpr float TITLE_W = 3725.0f;
        constexpr float TITLE_H = 1610.0f;

        constexpr float START_X = 1305.0f;
        constexpr float START_Y = 115.0f;
        constexpr float START_W = 2835.0f;
        constexpr float START_H = 1165.0f;

        constexpr float PRESS_O_X = 170.0f;
        constexpr float PRESS_O_Y = 550.0f;
        constexpr float PRESS_O_W = 930.0f;
        constexpr float PRESS_O_H = 218.0f;
    }

    // 表示位置・サイズ
    namespace Display
    {
        constexpr float TITLE_X = 400.0f;
        constexpr float TITLE_Y = 72.0f;
        constexpr float TITLE_W = 1118.0f;
        constexpr float TITLE_H = 483.0f;

        constexpr float START_X = 740.0f;
        constexpr float START_Y = 576.0f;
        constexpr float START_W = 337.0f;
        constexpr float START_H = 141.0f;

        constexpr float PRESS_O_X = 549.0f;
        constexpr float PRESS_O_Y = 663.0f;
        constexpr float PRESS_O_W = 712.0f;
        constexpr float PRESS_O_H = 154.0f;
    }

    // マウスクリック範囲
    namespace MouseArea
    {
        constexpr int X = 765;
        constexpr int Y = 592;
        constexpr int W = 268;
        constexpr int H = 94;
    }

    // アニメーション
    constexpr float HOVER_SCALE = 1.1f;
    constexpr float HOVER_ANIMATION_SPEED = 5.0f;
    constexpr float SCENE_CHANGE_DELAY = 0.1f;
}

// ゲームシーン定数
namespace SceneGameConst
{
    // カメラ設定
    constexpr float FOV_DEGREES = 45.0f;
    constexpr float NEAR_PLANE = 0.1f;
    constexpr float FAR_PLANE = 1000.0f;

    constexpr float CAMERA_POS_X = 0.0f;
    constexpr float CAMERA_POS_Y = 10.0f;
    constexpr float CAMERA_POS_Z = -10.0f;

    constexpr float TARGET_X = 0.0f;
    constexpr float TARGET_Y = 0.0f;
    constexpr float TARGET_Z = 0.0f;

    constexpr float UP_X = 0.0f;
    constexpr float UP_Y = 1.0f;
    constexpr float UP_Z = 0.0f;

    constexpr float CAMERA_HEIGHT = 8.6f;

    // ライト設定
    constexpr float LIGHT_DIR_X = -1.0f;
    constexpr float LIGHT_DIR_Y = -1.0f;
    constexpr float LIGHT_DIR_Z = 0.0f;

    constexpr float LIGHT_COLOR_R = 1.0f;
    constexpr float LIGHT_COLOR_G = 1.0f;
    constexpr float LIGHT_COLOR_B = 1.0f;

    constexpr float LIGHT_INTENSITY = 1.0f;
    constexpr float SHADOW_COLOR = 0.5f;
    constexpr float CLEAR_COLOR_BLUE = 0.2f;

    // ビネット
    constexpr float VIGNETTE_FADE_SPEED = 3.0f;
    constexpr float VIGNETTE_COUNTER_INTENSITY = 0.75f;
    constexpr float VIGNETTE_THRESHOLD = 0.01f;
    constexpr float VIGNETTE_SCALE = 1.0f;

    // フェード
    constexpr float FADE_IN_INTENSITY = 6.0f;
    constexpr float FADE_OUT_INTENSITY = 2.0f;

    // 音声
    constexpr float CINEMATIC_BGM_VOLUME = 0.1f;
    constexpr float DEATH_BGM_VOLUME = 0.05f;

    // エフェクト
    constexpr float EFFECT_SCALE = 1.0f;
    constexpr float ENEMY_SPAWN_SOUND_VOLUME = 0.5f;

    // テクスチャスロット
    constexpr int ENVIRONMENT_MAP_SLOT = 7;

    // ファイルパス
    constexpr const char* ENVIRONMENT_MAP = "Data/Environment/Material.001_baseColor.hdr";
    constexpr const char* VIGNETTE_TEXTURE = "Data/Textures/vignette_mask.png";
    constexpr const char* BGM_PATH = "Data/Audio/GameBgm.wav";
}