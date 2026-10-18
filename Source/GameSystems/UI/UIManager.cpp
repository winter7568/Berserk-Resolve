#include <assert.h>
#include <algorithm>
#include <imgui.h>

#include "UIManager.h"

UIManager& UIManager::GetInstance()
{
    static UIManager instance;
    return instance;
}

void UIManager::Initialize(ID3D11Device* device)
{
    mainDevice = device;
}

void UIManager::LoadTexture(const char* filename)
{
    std::string path(filename);
    if (mainTextures.find(path) != mainTextures.end())
        return;

    assert(mainDevice && "UIManager not initialized!");

    TextureInfo info;
    info.sprite = std::make_unique<Sprite>(mainDevice, filename);
    info.width = info.sprite->GetTextureWidth();
    info.height = info.sprite->GetTextureHeight();

    mainTextures[path] = std::move(info);
}

void UIManager::UnloadTexture(const char* filename)
{
    std::string path(filename);
    mainTextures.erase(path);

    for (auto it = mainRegions.begin(); it != mainRegions.end(); )
    {
        if (it->second.texturePath == path)
            it = mainRegions.erase(it);
        else
            ++it;
    }
}

void UIManager::UnloadAllTextures()
{
    mainTextures.clear();
    mainRegions.clear();
}

void UIManager::AddUIRegion(const std::string& regionName,
    const char* filename,
    float u, float v,
    float width, float height,
    bool normalizedCoords)
{
    std::string path(filename);
    assert(mainTextures.find(path) != mainTextures.end() && "Texture not loaded!");
    mainRegions[regionName] = { path, u, v, width, height, normalizedCoords };
}

void UIManager::RenderRegion(ID3D11DeviceContext* dc,
    const std::string& regionName,
    float screenX, float screenY,
    float screenWidth,
    float screenHeight,
    float angle,
    float r, float g, float b, float a)
{
    auto regionIt = mainRegions.find(regionName);
    if (regionIt == mainRegions.end()) return;

    const UIRegion& region = regionIt->second;
    auto textureIt = mainTextures.find(region.texturePath);
    if (textureIt == mainTextures.end()) return;

    const TextureInfo& texInfo = textureIt->second;

    float texX = region.normalized ? region.u * texInfo.width : region.u;
    float texY = region.normalized ? region.v * texInfo.height : region.v;
    float texWidth = region.normalized ? region.width * texInfo.width : region.width;
    float texHeight = region.normalized ? region.height * texInfo.height : region.height;

    float renderWidth = screenWidth > 0 ? screenWidth : texWidth;
    float renderHeight = screenHeight > 0 ? screenHeight : texHeight;

    texInfo.sprite->Render(dc,
        screenX, screenY, 0,
        renderWidth, renderHeight,
        texX, texY,
        texWidth, texHeight,
        angle,
        r, g, b, a);
}

void UIManager::GetUIRegionSize(const std::string& regionName, float& outWidth, float& outHeight)
{
    auto regionIt = mainRegions.find(regionName);
    if (regionIt == mainRegions.end())
    {
        outWidth = outHeight = 0.0f;
        return;
    }

    const UIRegion& region = regionIt->second;
    auto textureIt = mainTextures.find(region.texturePath);
    if (textureIt == mainTextures.end())
    {
        outWidth = outHeight = 0.0f;
        return;
    }

    if (region.normalized)
    {
        outWidth = region.width * textureIt->second.width;
        outHeight = region.height * textureIt->second.height;
    }
    else
    {
        outWidth = region.width;
        outHeight = region.height;
    }
}

ID3D11ShaderResourceView* UIManager::GetTextureSRV(const char* filename)
{
    std::string path(filename);
    auto it = mainTextures.find(path);
    return it != mainTextures.end() ? it->second.sprite->GetShaderResourceView() : nullptr;
}

void UIManager::RenderEnemyHealthBar(ID3D11DeviceContext* dc,
    float currentHealth, float maxHealth)
{
    if (!enemyUIVisible) return;

    RenderRegion(dc, "HealthBarBackground",
        healthBarX, healthBarY,
        healthBarWidth, healthBarHeight);

    float healthPercent = std::clamp(currentHealth / maxHealth, 0.0f, 1.0f);

    if (healthPercent > 0.0f)
    {
        auto regionIt = mainRegions.find("HealthBarFill");
        if (regionIt != mainRegions.end())
        {
            const UIRegion& region = regionIt->second;
            auto textureIt = mainTextures.find(region.texturePath);
            if (textureIt != mainTextures.end())
            {
                const TextureInfo& texInfo = textureIt->second;

                float fillX = healthBarX + fillOffsetX;
                float fillY = healthBarY + fillOffsetY;

                float texX = region.u;
                float texY = region.v;
                float texWidth = region.width;
                float texHeight = region.height;

                float visibleTexWidth = texWidth * healthPercent;
                float destWidth = maxFillWidth * healthPercent;

                texInfo.sprite->Render(dc,
                    fillX, fillY, 0,
                    destWidth, fillHeight,
                    texX, texY,
                    visibleTexWidth, texHeight,
                    0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
            }
        }
    }
}

void UIManager::DrawEnemyHealthBarDebugUI()
{
    ImGui::Begin("Enemy Health Bar Settings");

    ImGui::Text("HEALTH BAR BACKGROUND:");
    ImGui::DragFloat("Bar X", &healthBarX, 1.0f, -1000.0f, 2000.0f);
    ImGui::DragFloat("Bar Y", &healthBarY, 1.0f, -1000.0f, 2000.0f);
    ImGui::DragFloat("Bar Width", &healthBarWidth, 1.0f, 1.0f, 2000.0f);
    ImGui::DragFloat("Bar Height", &healthBarHeight, 1.0f, 1.0f, 2000.0f);

    ImGui::Separator();

    ImGui::Text("HEALTH FILL:");
    ImGui::DragFloat("Fill Offset X", &fillOffsetX, 1.0f, -500.0f, 500.0f);
    ImGui::DragFloat("Fill Offset Y", &fillOffsetY, 1.0f, -500.0f, 500.0f);
    ImGui::DragFloat("Max Fill Width", &maxFillWidth, 1.0f, 1.0f, 2000.0f);
    ImGui::DragFloat("Fill Height", &fillHeight, 1.0f, 1.0f, 2000.0f);

    ImGui::Separator();

    ImGui::Text("Current Fill Position:");
    ImGui::Text("X=%.1f, Y=%.1f",
        healthBarX + fillOffsetX,
        healthBarY + fillOffsetY);

    ImGui::Separator();

    if (ImGui::Button("Reset to Default"))
    {
        healthBarX = UIConstants::DEBUG_BAR_X;
        healthBarY = UIConstants::DEBUG_BAR_Y;
        healthBarWidth = UIConstants::DEBUG_BAR_WIDTH;
        healthBarHeight = UIConstants::DEBUG_BAR_HEIGHT;
        fillOffsetX = UIConstants::DEBUG_FILL_OFFSET_X;
        fillOffsetY = UIConstants::DEBUG_FILL_OFFSET_Y;
        maxFillWidth = UIConstants::DEBUG_MAX_FILL_WIDTH;
        fillHeight = UIConstants::DEBUG_FILL_HEIGHT;
    }

    ImGui::SameLine();

    if (ImGui::Button("Top Center"))
    {
        healthBarX = UIConstants::DEBUG_TOP_CENTER_X;
        healthBarY = UIConstants::DEBUG_TOP_CENTER_Y;
    }

    ImGui::SameLine();

    if (ImGui::Button("Top Left"))
    {
        healthBarX = UIConstants::DEBUG_TOP_LEFT_X;
        healthBarY = UIConstants::DEBUG_TOP_LEFT_Y;
    }

    ImGui::SameLine();

    if (ImGui::Button("Top Right"))
    {
        healthBarX = UIConstants::DEBUG_TOP_RIGHT_X;
        healthBarY = UIConstants::DEBUG_TOP_RIGHT_Y;
    }

    ImGui::End();
}

void UIManager::LoadEnemyUi()
{
    UIManager::GetInstance().LoadTexture(UIConstants::HEALTH_TEXTURE_PATH);

    UIManager::GetInstance().AddUIRegion("HealthBarBackground",
        UIConstants::HEALTH_TEXTURE_PATH,
        130.0f, 365.0f, 1177.0f, 63.0f, false);

    UIManager::GetInstance().AddUIRegion("HealthBarFill",
        UIConstants::HEALTH_TEXTURE_PATH,
        135.0f, 470.0f, 1092.0f, 50.0f, false);
}

void UIManager::LoadPlayerUi()
{
    LoadTexture(UIConstants::PLAYER_UI_TEXTURE_PATH);

    AddUIRegion("PlayerUIFrame",
        UIConstants::PLAYER_UI_TEXTURE_PATH,
        35.0f, 7.0f, 530.0f, 250.0f, false);

    AddUIRegion("PlayerHealthFill",
        UIConstants::PLAYER_UI_TEXTURE_PATH,
        220.0f, 290.0f, 328.0f, 41.0f, false);

    AddUIRegion("PlayerStaminaFill",
        UIConstants::PLAYER_UI_TEXTURE_PATH,
        219.0f, 333.0f, 289.0f, 20.0f, false);
}

void UIManager::RenderPlayerUI(ID3D11DeviceContext* dc,
    float currentHealth, float maxHealth,
    float currentStamina, float maxStamina)
{
    if (!playerUIVisible) return;
    if (maxHealth <= 0 || maxStamina <= 0) return;

    RenderRegion(dc, "PlayerUIFrame",
        playerUIFrameX, playerUIFrameY,
        playerUIFrameWidth, playerUIFrameHeight,
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    float healthPercent = std::clamp(currentHealth / maxHealth, 0.0f, 1.0f);
    if (healthPercent > 0.0f)
    {
        auto regionIt = mainRegions.find("PlayerHealthFill");
        if (regionIt != mainRegions.end())
        {
            const UIRegion& region = regionIt->second;
            auto textureIt = mainTextures.find(region.texturePath);
            if (textureIt != mainTextures.end())
            {
                const TextureInfo& texInfo = textureIt->second;

                float fillX = playerUIFrameX + playerHealthFillOffsetX;
                float fillY = playerUIFrameY + playerHealthFillOffsetY;

                float texX = region.u;
                float texY = region.v;
                float texWidth = region.width;
                float texHeight = region.height;

                float visibleTexWidth = texWidth * healthPercent;
                float destWidth = playerHealthFillWidth * healthPercent;

                texInfo.sprite->Render(dc,
                    fillX, fillY, 0,
                    destWidth, playerHealthFillHeight,
                    texX, texY,
                    visibleTexWidth, texHeight,
                    0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
            }
        }
    }

    float staminaPercent = std::clamp(currentStamina / maxStamina, 0.0f, 1.0f);
    if (staminaPercent > 0.0f)
    {
        auto regionIt = mainRegions.find("PlayerStaminaFill");
        if (regionIt != mainRegions.end())
        {
            const UIRegion& region = regionIt->second;
            auto textureIt = mainTextures.find(region.texturePath);
            if (textureIt != mainTextures.end())
            {
                const TextureInfo& texInfo = textureIt->second;

                float fillX = playerUIFrameX + playerStaminaFillOffsetX;
                float fillY = playerUIFrameY + playerStaminaFillOffsetY;

                float texX = region.u;
                float texY = region.v;
                float texWidth = region.width;
                float texHeight = region.height;

                float visibleTexWidth = texWidth * staminaPercent;
                float destWidth = playerStaminaFillWidth * staminaPercent;

                texInfo.sprite->Render(dc,
                    fillX, fillY, 0,
                    destWidth, playerStaminaFillHeight,
                    texX, texY,
                    visibleTexWidth, texHeight,
                    0.0f, 0.0f, 0.0f, 1.0f, 1.0f);
            }
        }
    }
}

void UIManager::DrawPlayerUIDebugUI()
{
    ImGui::Begin("Player UI Settings");

    ImGui::Text("PLAYER UI FRAME:");
    ImGui::DragFloat("Frame X", &playerUIFrameX, 1.0f, -1000.0f, 2000.0f);
    ImGui::DragFloat("Frame Y", &playerUIFrameY, 1.0f, -1000.0f, 2000.0f);
    ImGui::DragFloat("Frame Width", &playerUIFrameWidth, 1.0f, 1.0f, 2000.0f);
    ImGui::DragFloat("Frame Height", &playerUIFrameHeight, 1.0f, 1.0f, 2000.0f);

    ImGui::Separator();

    ImGui::Text("HEALTH FILL:");
    ImGui::DragFloat("Health Offset X", &playerHealthFillOffsetX, 1.0f, -100.0f, 200.0f);
    ImGui::DragFloat("Health Offset Y", &playerHealthFillOffsetY, 1.0f, -100.0f, 200.0f);
    ImGui::DragFloat("Health Max Width", &playerHealthFillWidth, 1.0f, 1.0f, 500.0f);
    ImGui::DragFloat("Health Height", &playerHealthFillHeight, 1.0f, 1.0f, 100.0f);

    ImGui::Separator();

    ImGui::Text("STAMINA FILL:");
    ImGui::DragFloat("Stamina Offset X", &playerStaminaFillOffsetX, 1.0f, -100.0f, 200.0f);
    ImGui::DragFloat("Stamina Offset Y", &playerStaminaFillOffsetY, 1.0f, -100.0f, 200.0f);
    ImGui::DragFloat("Stamina Max Width", &playerStaminaFillWidth, 1.0f, 1.0f, 500.0f);
    ImGui::DragFloat("Stamina Height", &playerStaminaFillHeight, 1.0f, 1.0f, 100.0f);

    ImGui::Separator();

    ImGui::Text("Current Positions:");
    ImGui::Text("Health: (%.1f, %.1f)",
        playerUIFrameX + playerHealthFillOffsetX,
        playerUIFrameY + playerHealthFillOffsetY);
    ImGui::Text("Stamina: (%.1f, %.1f)",
        playerUIFrameX + playerStaminaFillOffsetX,
        playerUIFrameY + playerStaminaFillOffsetY);

    ImGui::End();
}

void UIManager::LoadControlsUI()
{
    LoadTexture(UIConstants::CONTROLS_TEXTURE_PATH);
    AddUIRegion("ControlsOverlay",
        UIConstants::CONTROLS_TEXTURE_PATH,
        0.0f, 0.0f,  // UV u,v - full texture
        1.0f, 1.0f,  // width,height - normalized
        true);       // normalizedCoords

    // Attack prompt - crop specific area from texture
    AddUIRegion("AttackPrompt",
        UIConstants::CONTROLS_TEXTURE_PATH,
        UIConstants::ATTACK_PROMPT_U,
        UIConstants::ATTACK_PROMPT_V,
        UIConstants::ATTACK_PROMPT_W,
        UIConstants::ATTACK_PROMPT_H,
        false);

    // Counter prompt - crop specific area from texture
    AddUIRegion("CounterPrompt",
        UIConstants::CONTROLS_TEXTURE_PATH,
        UIConstants::COUNTER_PROMPT_U,
        UIConstants::COUNTER_PROMPT_V,
        UIConstants::COUNTER_PROMPT_W,
        UIConstants::COUNTER_PROMPT_H,
        false);
}

void UIManager::RenderControlsUI(ID3D11DeviceContext* dc)
{
    RenderRegion(dc, "ControlsOverlay",
        controlsX, controlsY,
        controlsWidth, controlsHeight,
        0.0f,        
        1.0f, 1.0f, 1.0f, 0.85f); 
}

void UIManager::DrawControlsDebugUI()
{
    ImGui::Begin("Controls UI Settings");

    ImGui::Text("CONTROLS OVERLAY:");
    ImGui::DragFloat("Position X", &controlsX, 1.0f, -500.0f, 1920.0f);
    ImGui::DragFloat("Position Y", &controlsY, 1.0f, -500.0f, 1080.0f);
    ImGui::DragFloat("Width", &controlsWidth, 1.0f, 50.0f, 800.0f);
    ImGui::DragFloat("Height", &controlsHeight, 1.0f, 50.0f, 600.0f);

    ImGui::Separator();

    if (ImGui::Button("Reset to Default"))
    {
        controlsX = UIConstants::CONTROLS_DEFAULT_X;
        controlsY = UIConstants::CONTROLS_DEFAULT_Y;
        controlsWidth = UIConstants::CONTROLS_DEFAULT_WIDTH;
        controlsHeight = UIConstants::CONTROLS_DEFAULT_HEIGHT;
    }

    ImGui::SameLine();

    if (ImGui::Button("Bottom-Right"))
    {
        controlsX = 1920.0f - controlsWidth - 20.0f;
        controlsY = 1080.0f - controlsHeight - 20.0f;
    }

    ImGui::SameLine();

    if (ImGui::Button("Bottom-Left"))
    {
        controlsX = 20.0f;
        controlsY = 1080.0f - controlsHeight - 20.0f;
    }

    ImGui::End();
}

void UIManager::RenderAttackPrompt(ID3D11DeviceContext* dc)
{
    if (!showAttackPrompt) return;

    RenderRegion(dc, "AttackPrompt",
        attackPromptX, attackPromptY,
        attackPromptWidth, attackPromptHeight,
        0.0f,
        1.0f, 1.0f, 1.0f, 0.7f);
}

void UIManager::RenderCounterPrompt(ID3D11DeviceContext* dc)
{
    if (!showCounterPrompt) return;

    RenderRegion(dc, "CounterPrompt",
        counterPromptX, counterPromptY,
        counterPromptWidth, counterPromptHeight,
        0.0f,
        1.0f, 1.0f, 1.0f, 0.7f);
}

void UIManager::DrawPromptsDebugUI()
{
    ImGui::Begin("Prompt Settings");

    ImGui::Checkbox("Show Attack Prompt", &showAttackPrompt);
    ImGui::Checkbox("Show Counter Prompt", &showCounterPrompt);

    ImGui::Separator();

    ImGui::Text("ATTACK PROMPT:");
    ImGui::DragFloat("Attack X", &attackPromptX, 1.0f, -500.0f, 1920.0f);
    ImGui::DragFloat("Attack Y", &attackPromptY, 1.0f, -500.0f, 1080.0f);
    ImGui::DragFloat("Attack Width", &attackPromptWidth, 1.0f, 50.0f, 800.0f);
    ImGui::DragFloat("Attack Height", &attackPromptHeight, 1.0f, 50.0f, 600.0f);

    ImGui::Separator();

    ImGui::Text("COUNTER PROMPT:");
    ImGui::DragFloat("Counter X", &counterPromptX, 1.0f, -500.0f, 1920.0f);
    ImGui::DragFloat("Counter Y", &counterPromptY, 1.0f, -500.0f, 1080.0f);
    ImGui::DragFloat("Counter Width", &counterPromptWidth, 1.0f, 50.0f, 800.0f);
    ImGui::DragFloat("Counter Height", &counterPromptHeight, 1.0f, 50.0f, 600.0f);

    ImGui::Separator();

    if (ImGui::Button("Reset Prompts to Default"))
    {
        attackPromptX = 100.0f;
        attackPromptY = 100.0f;
        attackPromptWidth = 200.0f;
        attackPromptHeight = 200.0f;
        counterPromptX = 100.0f;
        counterPromptY = 300.0f;
        counterPromptWidth = 200.0f;
        counterPromptHeight = 200.0f;
    }

    ImGui::End();
}
