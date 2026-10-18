#pragma once
#include <DirectXMath.h>

// アリーナ定数
struct ArenaConstants
{
    // モデル設定
    static constexpr const char* STAGE_MODEL_PATH = "Data/Model/LightArenaFinal/Arena.gltf";
    static constexpr float STAGE_MODEL_SCALE = 4.0f;
    static constexpr float EMISSIVE_STRENGTH = 0.8f;
    static constexpr float ENVIRONMENT_STRENGTH = 0.005f;

    // デフォルト位置
    static constexpr float DEFAULT_POSITION_X = 0.0f;
    static constexpr float DEFAULT_POSITION_Y = 0.4f;
    static constexpr float DEFAULT_POSITION_Z = 0.0f;
    static constexpr float DEFAULT_SCALE = 6.0f;
    static constexpr float DEFAULT_ROTATION_Z = 0.0f;

    // 境界設定
    static constexpr float BOUNDARY_CENTER_X = -14.0f;
    static constexpr float BOUNDARY_CENTER_Y = 0.0f;
    static constexpr float BOUNDARY_CENTER_Z = -4.5f;
    static constexpr float BOUNDARY_RADIUS = 350.0f;
    static constexpr float BOUNDARY_HEIGHT = 50.0f;
    static constexpr float BOUNDARY_ROTATION_Y = -11.0f;

    // パス境界
    static constexpr float PATH_CENTER_X = -470.0f;
    static constexpr float PATH_CENTER_Y = 0.0f;
    static constexpr float PATH_CENTER_Z = -6.0f;
    static constexpr float PATH_SIZE_X = 110.0f;
    static constexpr float PATH_SIZE_Y = 50.0f;
    static constexpr float PATH_SIZE_Z = 40.0f;

    // ゲートトレイル
    static constexpr float GATE_ENTRANCE_X = -366.0f;
    static constexpr float GATE_ENTRANCE_Y = 16.0f;
    static constexpr float GATE_ENTRANCE_Z = -57.0f;
    static constexpr float GATE_EXIT_X = -366.0f;
    static constexpr float GATE_EXIT_Y = 16.0f;
    static constexpr float GATE_EXIT_Z = 48.0f;
    static constexpr float OSCILLATION_SPEED = 60.0f;
    static constexpr float OSCILLATION_RANGE = 18.0f;
    static constexpr const char* NORMAL_MAP_PATH = "Data/Textures/NoiseTexture.png";
    static constexpr const char* MASK_PATH = "Data/Textures/TrailGate.png";

    // ゲートキューブ
    static constexpr float GATE_TRAIL_LIFETIME = 1.8f;
    static constexpr int GATE_TRAIL_SEGMENTS = 20;
    static constexpr float GATE_CUBE_CENTER_X = -379.0f;
    static constexpr float GATE_CUBE_CENTER_Y = 36.0f;
    static constexpr float GATE_CUBE_CENTER_Z = -4.0f;
    static constexpr float GATE_CUBE_SIZE_X = 19.0f;
    static constexpr float GATE_CUBE_SIZE_Y = 7.0f;
    static constexpr float GATE_CUBE_SIZE_Z = 64.0f;

    // キューブ
    static constexpr float CENTER_CUBE_CENTER_X = -5.5f;
    static constexpr float CENTER_CUBE_CENTER_Y = 0.0f;
    static constexpr float CENTER_CUBE_CENTER_Z = 0.5f;
    static constexpr float CENTER_CUBE_SIZE_X = 70.0f;
    static constexpr float CENTER_CUBE_SIZE_Y = 60.0f;
    static constexpr float CENTER_CUBE_SIZE_Z = 70.0f;

    // 発光
    static constexpr float EMISSIVE_COLOR_R = 1.0f;
    static constexpr float EMISSIVE_COLOR_G = 0.6f;
    static constexpr float EMISSIVE_COLOR_B = 0.2f;
    static constexpr float FLASH_COLOR_R = 1.0f;
    static constexpr float FLASH_COLOR_G = 0.0f;
    static constexpr float FLASH_COLOR_B = 0.0f;
};
