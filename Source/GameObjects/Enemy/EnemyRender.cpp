#include "Enemy.h"
#include "GameSystems/Debug/DebugManager.h"
#include "GameSystems/Collision/CollisionManager.h"
#include "GameObjects/Player/Player.h"
#include "GameLib/Graphics.h"
#include "ShaderSystems/DistortionShader.h"
#include "StateMachine/EnemyStates/PositioningState.h"
#include "StateMachine/EnemyStates/AttackStates/SlashAttackState.h"
#include "StateMachine/EnemyStates/TeleportState.h"
#include <imgui.h>

// ============================================================================
// PUBLIC METHODS - Render
// ============================================================================

void Enemy::RenderDebugPrimitive(const RenderContext& rc, ShapeRenderer* renderer)
{
    auto& debugManager = DebugManager::Instance();
    auto& debugRenderer = DebugRenderer::Instance();

    // 本体カプセルの描画
    if (debugManager.IsPlayerBodyVisible()) {

        // 通常はシアン色
        DirectX::XMFLOAT4 bodyColor = { 0.0f, 1.0f, 1.0f, 1.0f };

        auto& collisionManager = CollisionManager::Instance();
        auto& player = Player::Instance();

        // 衝突している場合は赤色に変更
        if (collisionManager.IsPlayerEnemyColliding() ||
            (collisionManager.IsWeaponEnemyColliding() && player.IsWeaponHitActive())) {
            bodyColor = { 1.0f, 0.0f, 0.0f, 1.0f };
        }

        // 本体カプセル描画
        debugRenderer.DrawBodyCapsule(rc, this, bodyColor);
    }

    // 武器カプセルの描画
    RenderWeaponCapsuleDebug(rc, debugRenderer);
}

void Enemy::RenderSwordTrail(const RenderContext& rc)
{
    // 深度ステンシル状態を取得（デバッグ用）
    ID3D11DepthStencilState* currentDepthState = nullptr;
    UINT stencilRef;
    rc.deviceContext->OMGetDepthStencilState(&currentDepthState, &stencilRef);

    if (currentDepthState) {
        D3D11_DEPTH_STENCIL_DESC desc;
        currentDepthState->GetDesc(&desc);
        currentDepthState->Release();
    }

    // トレイルが無効または頂点が無い場合は描画しない
    if (!swordTrail.IsActive() || swordTrail.GetVertexCount() == 0) {
        return;
    }

    // 歪みシェーダーを取得
    DistortionShader* distortionShader = static_cast<DistortionShader*>(Graphics::Instance().GetShader(ShaderId::Distortion));

    if (!distortionShader) {
        return;
    }

    // シーンバッファを取得（歪み用）
    Graphics& graphics = Graphics::Instance();
    ID3D11ShaderResourceView* sceneSRV = graphics.GetFrameBuffer(FrameBufferId::Scene)->GetColorMap();

    // 敵のトレイル用設定
    distortionShader->SetEnemyTrail();

    // シェーダー開始
    distortionShader->Begin(rc);

    // トレイルを描画
    distortionShader->DrawTrail(rc,
        swordTrail.GetVertexBuffer(),
        swordTrail.GetIndexBuffer(),
        swordTrail.GetVertexCount(),
        swordTrail.GetIndexCount(),
        sceneSRV,
        swordTrail.GetNormalMap(),
        swordTrail.GetMask()
    );

    // シェーダー終了
    distortionShader->End(rc);

    // シェーダーリソースをクリア
    ID3D11ShaderResourceView* nullSRVs[] = { nullptr, nullptr, nullptr };
    rc.deviceContext->PSSetShaderResources(
        CharacterConstants::TRAIL_TEXTURE_SLOT_START,
        CharacterConstants::TRAIL_TEXTURE_SLOT_COUNT,
        nullSRVs
    );
}

void Enemy::UpdateEmissiveFlash(float elapsedTime)
{
    if (emissiveFlashTimer > 0.0f)
    {
        emissiveFlashTimer -= elapsedTime;

        // フラッシュ終了時に元の色へ戻す
        if (emissiveFlashTimer <= 0.0f)
        {
            ResetEmissiveColor();
            emissiveFlashTimer = 0.0f;
        }
    }
}

void Enemy::RenderSwordTrailDebugUI()
{
    if (ImGui::CollapsingHeader("Sword Trail Debug"))
    {
        ImGui::DragFloat3("##TipOffset", &swordTrailTipOffset.x, EnemyConstants::DEBUG_UI_DRAG_STEP, EnemyConstants::DEBUG_UI_MIN_RANGE, EnemyConstants::DEBUG_UI_MAX_RANGE);
        ImGui::DragFloat3("##BaseOffset", &swordTrailBaseOffset.x, EnemyConstants::DEBUG_UI_DRAG_STEP, EnemyConstants::DEBUG_UI_MIN_RANGE, EnemyConstants::DEBUG_UI_MAX_RANGE);
        // 現在のワールド位置を参考表示
        DirectX::XMMATRIX swordWorld = DirectX::XMLoadFloat4x4(&sword.transform);

        DirectX::XMVECTOR tipWorld = DirectX::XMVector3TransformCoord(
            DirectX::XMLoadFloat3(&swordTrailTipOffset), swordWorld);
        DirectX::XMVECTOR baseWorld = DirectX::XMVector3TransformCoord(
            DirectX::XMLoadFloat3(&swordTrailBaseOffset), swordWorld);

        DirectX::XMFLOAT3 tipWorldPos, baseWorldPos;
        DirectX::XMStoreFloat3(&tipWorldPos, tipWorld);
        DirectX::XMStoreFloat3(&baseWorldPos, baseWorld);

        ImGui::Text("World Positions:");
        ImGui::Text("Tip: (%.3f, %.3f, %.3f)", tipWorldPos.x, tipWorldPos.y, tipWorldPos.z);
        ImGui::Text("Base: (%.3f, %.3f, %.3f)", baseWorldPos.x, baseWorldPos.y, baseWorldPos.z);

        // オフセットリセット
        if (ImGui::Button("Reset Offsets"))
        {
            swordTrailTipOffset = EnemyConstants::TRAIL_RESET_TIP;
            swordTrailBaseOffset = EnemyConstants::TRAIL_RESET_BASE;
        }
    }
}

void Enemy::RenderSwordPositioningDebugUI()
{
    if (ImGui::CollapsingHeader("Sword Positioning & Capsule"))
    {
        ImGui::Text("Sword Model Positioning (Local to Bone):");
        ImGui::Separator();

        // 剣のボーンに対する位置調整
        ImGui::DragFloat3("Sword Position", &sword.position.x, 0.001f, -1.0f, 1.0f, "%.3f");

        // 回転（ラジアン → 度で表示）
        float degX = DirectX::XMConvertToDegrees(sword.angle.x);
        float degY = DirectX::XMConvertToDegrees(sword.angle.y);
        float degZ = DirectX::XMConvertToDegrees(sword.angle.z);

        if (ImGui::DragFloat("Sword Rot X (deg)", &degX, 0.1f, -180.0f, 180.0f)) {
            sword.angle.x = DirectX::XMConvertToRadians(degX);
        }
        if (ImGui::DragFloat("Sword Rot Y (deg)", &degY, 0.1f, -180.0f, 180.0f)) {
            sword.angle.y = DirectX::XMConvertToRadians(degY);
        }
        if (ImGui::DragFloat("Sword Rot Z (deg)", &degZ, 0.1f, -180.0f, 180.0f)) {
            sword.angle.z = DirectX::XMConvertToRadians(degZ);
        }

        // スケール調整
        ImGui::DragFloat3("Sword Scale", &sword.scale.x, 0.01f, 0.1f, 5.0f, "%.2f");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Weapon Capsule Parameters:");
        ImGui::Separator();

        // カプセル調整
        ImGui::DragFloat3("Capsule Offset", &weaponCapsule.offset.x, 0.001f, -1.0f, 1.0f, "%.3f");
        ImGui::DragFloat3("Capsule Rotation (deg)", &weaponCapsule.rotation.x, 0.1f, -180.0f, 180.0f, "%.1f");
        ImGui::DragFloat("Capsule Radius", &weaponCapsule.radius, 0.01f, 0.01f, 1.0f, "%.2f");
        ImGui::DragFloat("Capsule Height", &weaponCapsule.height, 0.01f, 0.1f, 3.0f, "%.2f");

        ImGui::ColorEdit4("Capsule Color", &weaponCapsule.color.x);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Current Values & Reset:");
        ImGui::Separator();

        ImGui::Text("Current Sword Pos: (%.3f, %.3f, %.3f)",
            sword.position.x, sword.position.y, sword.position.z);

        ImGui::Text("Current Sword Rot: (%.3f, %.3f, %.3f) rad",
            sword.angle.x, sword.angle.y, sword.angle.z);

        ImGui::Text("Current Capsule Offset: (%.3f, %.3f, %.3f)",
            weaponCapsule.offset.x, weaponCapsule.offset.y, weaponCapsule.offset.z);

        if (ImGui::Button("Reset Sword Only"))
        {
            sword.position = { -0.120f, 0.300f, -0.060f };
            sword.angle = { 0.0f, 3.8f, 0.0f };
            sword.scale = { 1.0f, 1.0f, 1.0f };
        }

        ImGui::SameLine();

        if (ImGui::Button("Reset Capsule Only"))
        {
            weaponCapsule.offset = { 0.0f, 0.130f, 0.257f };
            weaponCapsule.rotation = { -2.0f, 1.0f, 0.0f };
            weaponCapsule.radius = 0.1f;
            weaponCapsule.height = 1.0f;
            weaponCapsule.color = { 1.0f, 0.0f, 0.0f, 1.0f };
        }

        ImGui::SameLine();

        if (ImGui::Button("Reset All"))
        {
            sword.position = { -0.120f, 0.300f, -0.060f };
            sword.angle = { 0.0f, 3.8f, 0.0f };
            sword.scale = { 1.0f, 1.0f, 1.0f };

            weaponCapsule.offset = { 0.0f, 0.130f, 0.257f };
            weaponCapsule.rotation = { -2.0f, 1.0f, 0.0f };
            weaponCapsule.radius = 0.1f;
            weaponCapsule.height = 1.0f;
            weaponCapsule.color = { 1.0f, 0.0f, 0.0f, 1.0f };
        }

        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Changes apply in real time!");
    }
}

void Enemy::RenderPositioningBrainDebugUI()
{
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 0.3f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 0.5f));

    ImGui::Begin("Enemy AI - Positioning Brain", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoBackground);

    // 現在のステート情報
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "現在のステート");
    ImGui::Separator();

    std::string currentState = GetCurrentStateName();
    ImGui::Text("アクティブステート: %s", currentState.c_str());
    ImGui::Text("スタックサイズ: %d", GetStateManager().GetStackSize());

    ImGui::Spacing();

    // ポジショニングAI情報
    ImGui::TextColored(ImVec4(0, 1, 1, 1), "ポジショニングAI");
    ImGui::Separator();

    Player& player = Player::Instance();
    DirectX::XMFLOAT3 playerPos = player.GetPosition();
    DirectX::XMFLOAT3 enemyPos = GetPosition();

    float dx = playerPos.x - enemyPos.x;
    float dz = playerPos.z - enemyPos.z;
    float distance = std::sqrt(dx * dx + dz * dz);

    ImGui::Text("プレイヤーとの距離: %.2f", distance);
    ImGui::Text("プレイヤーを向いている: %s", IsFacingPlayer() ? "YES" : "NO");

    std::string playerState = player.GetCurrentStateName();
    ImGui::Text("プレイヤー状態: %s", playerState.c_str());
    ImGui::Text("攻撃中: %s",
        (playerState.find("Attack") != std::string::npos) ? "YES" : "NO");

    ImGui::Spacing();

    // 判断ログ
    ImGui::TextColored(ImVec4(0, 1, 0, 1), "判断ログ");
    ImGui::Separator();

    ImGui::Text("最後の判断: %s", positioningDebug.currentDecision.c_str());
    ImGui::Text("最後の行動: %s", positioningDebug.lastAction.c_str());

    ImGui::Text("最近の判断:");
    ImGui::BeginChild("DecisionHistory", ImVec2(0, 100), true);
    for (int i = max(0, (int)positioningDebug.decisionHistory.size() - 10);
        i < positioningDebug.decisionHistory.size(); ++i) {
        ImGui::Text("%d: %s", i + 1, positioningDebug.decisionHistory[i].c_str());
    }
    ImGui::EndChild();

    ImGui::Spacing();

    // 性格パラメータ
    ImGui::TextColored(ImVec4(0, 1, 0, 1), "性格パラメータ");
    ImGui::Separator();

    auto& positioning = EnemyPositioningState::Instance();

    static int currentAggression = 1;
    const char* aggressionNames = "Passive\0Balanced\0Aggressive\0Unpredictable\0";

    if (ImGui::Combo("攻撃性レベル", &currentAggression, aggressionNames)) {
        EnemyPositioningState::EnemyAggression aggression;
        switch (currentAggression) {
        case 0: aggression = EnemyPositioningState::EnemyAggression::PASSIVE; break;
        case 1: aggression = EnemyPositioningState::EnemyAggression::BALANCED; break;
        case 2: aggression = EnemyPositioningState::EnemyAggression::AGGRESSIVE; break;
        case 3: aggression = EnemyPositioningState::EnemyAggression::UNPREDICTABLE; break;
        }
        positioning.SetAggression(aggression);
    }

    static int currentStyle = 1;
    const char* styleNames = "Defensive\0Standard\0Offensive\0Mixed\0";

    if (ImGui::Combo("戦術スタイル", &currentStyle, styleNames)) {
        EnemyPositioningState::TacticalStyle style = EnemyPositioningState::TacticalStyle::STANDARD;;
        switch (currentStyle) {
        case 0: style = EnemyPositioningState::TacticalStyle::DEFENSIVE; break;
        case 1: style = EnemyPositioningState::TacticalStyle::STANDARD; break;
        case 2: style = EnemyPositioningState::TacticalStyle::OFFENSIVE; break;
        case 3: style = EnemyPositioningState::TacticalStyle::MIXED; break;
        }
        positioning.SetTacticalStyle(style);
    }

    ImGui::Spacing();

    // 現在の確率
    ImGui::TextColored(ImVec4(0, 1, 1, 1), "現在の行動確率");
    ImGui::Separator();

    float blockChance = GetPersonalityBlockChance();
    float dashChance = GetPersonalityDashChance();
    float counterChance = GetPersonalityCounterChance();
    float specialChance = GetPersonalitySpecialAttackChance();
    float teleportChance = GetPersonalityTeleportChance();

    ImGui::Text("ブロック確率: %.0f%%", blockChance * 100);
    ImGui::Text("ダッシュ確率: %.0f%%", dashChance * 100);
    ImGui::Text("カウンター確率: %.0f%%", counterChance * 100);
    ImGui::Text("特殊攻撃確率: %.0f%%", specialChance * 100);
    ImGui::Text("テレポート確率: %.0f%%", teleportChance * 100);

    ImGui::Spacing();

    // 距離設定
    ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "距離設定");
    ImGui::Separator();

    ImGui::Text("攻撃距離: %.1f", positioning.GetAdjustedAttackRange());
    ImGui::Text("移動開始距離: %.1f", positioning.GetAdjustedWalkRange());
    ImGui::Text("停止距離: 20.5");

    ImGui::Spacing();

    // 手動操作
    ImGui::TextColored(ImVec4(1, 0, 0, 1), "手動操作");
    ImGui::Separator();

    if (ImGui::Button("攻撃強制")) {
        GetStateManager().PushState(&EnemyAttackState::Instance());
    }

    if (ImGui::Button("ダッシュ強制")) {
        GetStateManager().PushState(&EnemyDashState::Instance());
    }

    if (ImGui::Button("テレポート")) {
        GetStateManager().PushState(&EnemyTeleportState::Instance());
    }

    if (ImGui::Button("ステートクリア")) {
        GetStateManager().ClearStates();
        GetStateManager().PushState(&EnemyPositioningState::Instance());
    }

    ImGui::End();

    ImGui::PopStyleColor(3);
}

// ============================================================================
// PRIVATE METHODS - Render Debug Helpers
// ============================================================================

void Enemy::RenderWeaponCapsuleDebug(const RenderContext& rc, DebugRenderer& debugRenderer)
{
    auto& weaponParams = GetWeaponCapsuleParams();

    if (weaponParams.enabled)
    {
        DirectX::XMFLOAT3 base, tip;
        float radius = 0.0f;

        // カプセルの端点取得
        if (GetWeaponCapsule(base, tip, radius))
        {
            // 攻撃中は赤、それ以外は緑
            DirectX::XMFLOAT4 color = IsWeaponHitActive()
                ? DirectX::XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f }
            : DirectX::XMFLOAT4{ 0.0f, 1.0f, 0.0f, 1.0f };

            // カプセル描画
            debugRenderer.DrawCapsuleFromEndpoints(rc, base, tip, radius, color);
        }
    }
}