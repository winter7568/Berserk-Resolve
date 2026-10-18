#pragma once
#include <memory>
#include <unordered_map>
#include <string>

#include "ShaderSystems/Sprite.h"

class UIManager
{
public:
    static UIManager& GetInstance();

    UIManager(const UIManager&) = delete;
    void operator=(const UIManager&) = delete;

    void Initialize(ID3D11Device* device);

    // テクスチャ読み込み
    void LoadTexture(const char* filename);
    void UnloadTexture(const char* filename);
    void UnloadAllTextures();

    // UI登録
    void AddUIRegion(const std::string& regionName,
        const char* filename,
        float u, float v,
        float width, float height,
        bool normalizedCoords = true);

    // UI描画
    void RenderRegion(ID3D11DeviceContext* dc,
        const std::string& regionName,
        float screenX, float screenY,
        float screenWidth = 0.0f,
        float screenHeight = 0.0f,
        float angle = 0.0f,
        float r = 1.0f, float g = 1.0f,
        float b = 1.0f, float a = 1.0f);

    void GetUIRegionSize(const std::string& regionName, float& outWidth, float& outHeight);
    ID3D11ShaderResourceView* GetTextureSRV(const char* filename);

    // 敵HPバー描画
    void RenderEnemyHealthBar(ID3D11DeviceContext* dc,
        float currentHealth, float maxHealth);

    void DrawEnemyHealthBarDebugUI();
    void LoadEnemyUi();

    void LoadPlayerUi();

    // プレイヤーUI描画
    void RenderPlayerUI(ID3D11DeviceContext* dc,
        float currentHealth, float maxHealth,
        float currentStamina, float maxStamina);

    void DrawPlayerUIDebugUI();

    void SetPlayerUIVisible(bool visible) { playerUIVisible = visible; }
    void SetEnemyUIVisible(bool visible) { enemyUIVisible = visible; }

    void LoadControlsUI();
    void RenderControlsUI(ID3D11DeviceContext* dc);
    void DrawControlsDebugUI();
    void RenderAttackPrompt(ID3D11DeviceContext* dc);
    void RenderCounterPrompt(ID3D11DeviceContext* dc);
    void DrawPromptsDebugUI();

    void SetAttackPromptVisible(bool visible) { showAttackPrompt = visible; }
    void SetCounterPromptVisible(bool visible) { showCounterPrompt = visible; }

private:
    UIManager() = default;
    ~UIManager() = default;

    // UI定数
    struct UIConstants
    {
        // 敵UI
        static constexpr float ENEMY_HEALTH_BAR_X = 146.0f;
        static constexpr float ENEMY_HEALTH_BAR_Y = 898.0f;
        static constexpr float ENEMY_HEALTH_BAR_WIDTH = 1640.0f;
        static constexpr float ENEMY_HEALTH_BAR_HEIGHT = 71.0f;
        static constexpr float ENEMY_FILL_OFFSET_X = 37.0f;
        static constexpr float ENEMY_FILL_OFFSET_Y = 10.0f;
        static constexpr float ENEMY_MAX_FILL_WIDTH = 1521.0f;
        static constexpr float ENEMY_FILL_HEIGHT = 55.0f;

        // プレイヤーUI
        static constexpr float PLAYER_FRAME_X = 8.0f;
        static constexpr float PLAYER_FRAME_Y = 4.0f;
        static constexpr float PLAYER_FRAME_WIDTH = 420.0f;
        static constexpr float PLAYER_FRAME_HEIGHT = 164.0f;
        static constexpr float PLAYER_HEALTH_OFFSET_X = 144.0f;
        static constexpr float PLAYER_HEALTH_OFFSET_Y = 66.0f;
        static constexpr float PLAYER_HEALTH_WIDTH = 261.0f;
        static constexpr float PLAYER_HEALTH_HEIGHT = 29.0f;
        static constexpr float PLAYER_STAMINA_OFFSET_X = 143.0f;
        static constexpr float PLAYER_STAMINA_OFFSET_Y = 95.0f;
        static constexpr float PLAYER_STAMINA_WIDTH = 227.0f;
        static constexpr float PLAYER_STAMINA_HEIGHT = 15.0f;

        // テクスチャパス
        static constexpr const char* HEALTH_TEXTURE_PATH = "Data/Sprite/Health.png";
        static constexpr const char* PLAYER_UI_TEXTURE_PATH = "Data/Sprite/PlayerUI.png";

        // コントロール UI
        static constexpr const char* CONTROLS_TEXTURE_PATH = "Data/Sprite/ControllerUI.png"; 
        static constexpr float CONTROLS_DEFAULT_X = 1300.0f;
        static constexpr float CONTROLS_DEFAULT_Y = 515.0f;
        static constexpr float CONTROLS_DEFAULT_WIDTH = 750.0f;
        static constexpr float CONTROLS_DEFAULT_HEIGHT = 380.0f;
        // Attack prompt region (pixel coordinates)
        static constexpr float ATTACK_PROMPT_U = 459.0f;
        static constexpr float ATTACK_PROMPT_V = 334.0f;
        static constexpr float ATTACK_PROMPT_W = 239.0f;
        static constexpr float ATTACK_PROMPT_H = 232.0f;

        // Counter prompt region (pixel coordinates)
        static constexpr float COUNTER_PROMPT_U = 903.0f;
        static constexpr float COUNTER_PROMPT_V = 334.0f;
        static constexpr float COUNTER_PROMPT_W = 239.0f;
        static constexpr float COUNTER_PROMPT_H = 232.0f;

        // Prompt default position/size
        static constexpr float PROMPT_DEFAULT_X = 100.0f;
        static constexpr float PROMPT_DEFAULT_Y = 100.0f;
        static constexpr float PROMPT_DEFAULT_WIDTH = 200.0f;
        static constexpr float PROMPT_DEFAULT_HEIGHT = 200.0f;

        // デバッグ初期値
        static constexpr float DEBUG_BAR_X = 50.0f;
        static constexpr float DEBUG_BAR_Y = 50.0f;
        static constexpr float DEBUG_BAR_WIDTH = 300.0f;
        static constexpr float DEBUG_BAR_HEIGHT = 50.0f;
        static constexpr float DEBUG_FILL_OFFSET_X = 10.0f;
        static constexpr float DEBUG_FILL_OFFSET_Y = 10.0f;
        static constexpr float DEBUG_MAX_FILL_WIDTH = 280.0f;
        static constexpr float DEBUG_FILL_HEIGHT = 30.0f;
        static constexpr float DEBUG_TOP_CENTER_X = 640.0f;
        static constexpr float DEBUG_TOP_CENTER_Y = 20.0f;
        static constexpr float DEBUG_TOP_LEFT_X = 20.0f;
        static constexpr float DEBUG_TOP_LEFT_Y = 20.0f;
        static constexpr float DEBUG_TOP_RIGHT_X = 1280.0f - 320.0f;
        static constexpr float DEBUG_TOP_RIGHT_Y = 20.0f;
    };

    bool playerUIVisible = true;
    bool enemyUIVisible = true;

    struct TextureInfo {
        std::unique_ptr<Sprite> sprite = nullptr;
        float width = 0.0f;
        float height = 0.0f;
    };

    struct UIRegion {
        std::string texturePath = "";
        float u = 0.0f;
        float v = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
        bool normalized = true;
    };

    // 敵UI
    float healthBarX = UIConstants::ENEMY_HEALTH_BAR_X;
    float healthBarY = UIConstants::ENEMY_HEALTH_BAR_Y;
    float healthBarWidth = UIConstants::ENEMY_HEALTH_BAR_WIDTH;
    float healthBarHeight = UIConstants::ENEMY_HEALTH_BAR_HEIGHT;

    float fillOffsetX = UIConstants::ENEMY_FILL_OFFSET_X;
    float fillOffsetY = UIConstants::ENEMY_FILL_OFFSET_Y;
    float maxFillWidth = UIConstants::ENEMY_MAX_FILL_WIDTH;
    float fillHeight = UIConstants::ENEMY_FILL_HEIGHT;

    // プレイヤーUI
    float playerUIFrameX = UIConstants::PLAYER_FRAME_X;
    float playerUIFrameY = UIConstants::PLAYER_FRAME_Y;
    float playerUIFrameWidth = UIConstants::PLAYER_FRAME_WIDTH;
    float playerUIFrameHeight = UIConstants::PLAYER_FRAME_HEIGHT;

    float playerHealthFillOffsetX = UIConstants::PLAYER_HEALTH_OFFSET_X;
    float playerHealthFillOffsetY = UIConstants::PLAYER_HEALTH_OFFSET_Y;
    float playerHealthFillWidth = UIConstants::PLAYER_HEALTH_WIDTH;
    float playerHealthFillHeight = UIConstants::PLAYER_HEALTH_HEIGHT;

    float playerStaminaFillOffsetX = UIConstants::PLAYER_STAMINA_OFFSET_X;
    float playerStaminaFillOffsetY = UIConstants::PLAYER_STAMINA_OFFSET_Y;
    float playerStaminaFillWidth = UIConstants::PLAYER_STAMINA_WIDTH;
    float playerStaminaFillHeight = UIConstants::PLAYER_STAMINA_HEIGHT;

    // Controls UI position
    float controlsX = UIConstants::CONTROLS_DEFAULT_X;
    float controlsY = UIConstants::CONTROLS_DEFAULT_Y;
    float controlsWidth = UIConstants::CONTROLS_DEFAULT_WIDTH;
    float controlsHeight = UIConstants::CONTROLS_DEFAULT_HEIGHT;

    // Attack prompt position
    float attackPromptX = 858.0f;
    float attackPromptY = 92.0f;
    float attackPromptWidth = 164.0f;
    float attackPromptHeight = 150.0f;

    // Counter prompt position
    float counterPromptX = 858.0f;
    float counterPromptY = 92.0f;
    float counterPromptWidth = 164.0f;
    float counterPromptHeight = 150.0f;

    bool showAttackPrompt = false;
    bool showCounterPrompt = false;

    std::unordered_map<std::string, TextureInfo> mainTextures;
    std::unordered_map<std::string, UIRegion> mainRegions;
    ID3D11Device* mainDevice = nullptr;
};