#include "GameLib/Graphics.h"
#include "Player.h"
#include "CameraFiles/CameraShake.h"
#include "GameSystems/Debug/DebugManager.h"
#include "GameSystems/Collision/CollisionManager.h"
#include "GameSystems/Effect/EffectManager.h"
#include "ShaderSystems/DistortionShader.h"
#include <imgui.h>

// ============================================================================
// PUBLIC METHODS - Render
// ============================================================================
bool Player::UpdateSpawn(float elapsedTime)
{
    // スポーン完了済みなら早期リターン
    if (spawnCompleted)
        return true;
    // スポーンタイマーを更新
    spawnTimer += elapsedTime;
    // EFFECT_START_TIME（4秒）になったらスポーンエフェクトとカメラシェイクを再生
    if (!spawnEffectPlayed && spawnTimer >= PlayerConstants::EFFECT_START_TIME) {
        DirectX::XMFLOAT3 spawnPosition = position;
        spawnPosition.y += PlayerConstants::EFFECT_Y_OFFSET;
        EffectManager::Instance().PlayEffect("playerSpwan", spawnPosition, PlayerConstants::EFFECT_SCALE);
        PlayPlayerSound(PlayerSound::SPAWN, PlayerConstants::SPAWN_SOUND_VOLUME);
        spawnEffectPlayed = true;
        CameraShake::Instance().AddTrauma(PlayerConstants::SPAWN_CAMERA_SHAKE);
    }

    // SPAWN_DURATIONに達するまでディゾルブを継続
    if (spawnTimer < PlayerConstants::SPAWN_DURATION)
        return false;
    // SPAWN_DURATION経過後のディゾルブ進行度を計算
    const float dissolveElapsed = spawnTimer - PlayerConstants::SPAWN_DURATION;
    float dissolveThreshold = 1.0f - (dissolveElapsed * PlayerConstants::DISSOLVE_SPEED);
    // 完全にディゾルブしたら完了（しきい値が0以下になる）
    if (dissolveThreshold <= PlayerConstants::DISSOLVE_COMPLETE)
    {
        dissolveThreshold = PlayerConstants::DISSOLVE_COMPLETE;
        spawnCompleted = true;
    }
    // ディゾルブしきい値をモデルに適用
    model->SetDissolveThreshold(dissolveThreshold);
    staff.model->SetDissolveThreshold(dissolveThreshold);
    // 点滅するエッジ発光効果を追加
    float pulse = sinf(spawnTimer * PlayerConstants::PULSE_FREQUENCY) * PlayerConstants::PULSE_STRENGTH + PlayerConstants::PULSE_BASE;
    model->SetDissolveEdgeIntensity(PlayerConstants::EDGE_INTENSITY_BASE * pulse);
    staff.model->SetDissolveEdgeIntensity(PlayerConstants::EDGE_INTENSITY_BASE * pulse);

    return spawnCompleted;
}


void Player::RenderDebugPrimitive(const RenderContext& rc, ShapeRenderer* renderer)
{
    auto& debugManager = DebugManager::Instance();
    auto& debugRenderer = DebugRenderer::Instance();
    // プレイヤーの当たり判定（カプセル）を表示
    if (debugManager.IsPlayerBodyVisible())
    {
        // 通常はシアン色
        DirectX::XMFLOAT4 bodyColor = { 0.0f, 1.0f, 1.0f, 1.0f };
        auto& collisionManager = CollisionManager::Instance();
        // 当たっている時は赤色
        if (collisionManager.IsPlayerEnemyColliding())
            bodyColor = { 1.0f, 0.0f, 0.0f, 1.0f };
        // カプセル描画
        DebugRenderer::Instance().DrawBodyCapsule(rc, this, bodyColor);
    }
    // 武器カプセルの表示
    RenderWeaponCapsuleDebug(rc, debugRenderer);
    // 武器スフィアの表示
    RenderWeaponSphereDebug(rc, debugRenderer);
}

void Player::RenderSwordTrail(const RenderContext& rc)
{
    // 深度バッファの状態をチェック
    ID3D11DepthStencilState* currentDepthState = nullptr;
    UINT stencilRef;
    rc.deviceContext->OMGetDepthStencilState(&currentDepthState, &stencilRef);
    if (currentDepthState) {
        D3D11_DEPTH_STENCIL_DESC desc;
        currentDepthState->GetDesc(&desc);
        currentDepthState->Release();
    }
    // TrailSystemのゲッターを使用（直接アクセスしない）
    if (!staffTrail.IsActive() || staffTrail.GetVertexCount() == 0) {
        return;
    }
    // ディストーションシェーダーを使用
    DistortionShader* distortionShader = static_cast<DistortionShader*>(
        Graphics::Instance().GetShader(ShaderId::Distortion));
    if (!distortionShader) {
        return;
    }
    // レンダーターゲットを設定
    Graphics& graphics = Graphics::Instance();
    // ディストーション用のシーンバッファを取得
    ID3D11ShaderResourceView* sceneSRV = graphics.GetFrameBuffer(FrameBufferId::Scene)->GetColorMap();
    distortionShader->SetPlayerTrail();
    // ディストーションシェーダーを開始
    distortionShader->Begin(rc);
    // TrailSystemのゲッターを使ってトレイルを描画
    distortionShader->DrawTrail(rc,
        staffTrail.GetVertexBuffer(),
        staffTrail.GetIndexBuffer(),
        staffTrail.GetVertexCount(),
        staffTrail.GetIndexCount(),
        sceneSRV,
        staffTrail.GetNormalMap(),
        staffTrail.GetMask()
    );
    distortionShader->End(rc);
    // シェーダーリソースをクリア
    ID3D11ShaderResourceView* nullSRVs[] = { nullptr, nullptr, nullptr };
    rc.deviceContext->PSSetShaderResources(
        CharacterConstants::TRAIL_TEXTURE_SLOT_START,
        CharacterConstants::TRAIL_TEXTURE_SLOT_COUNT,
        nullSRVs
    );
}

void Player::RenderSwordTrailDebugUI()
{
    // デバッグUIを表示
    if (ImGui::CollapsingHeader("Player Sword Trail Debug")) {
        // 先端のローカル位置
        ImGui::Text("Tip_Offset (Local Space):");
        ImGui::DragFloat3("##TipOffset", &staffTrailTipOffset.x, 0.01f, -10.0f, 10.0f);
        // 根元のローカル位置
        ImGui::Text("Base_Offset (Local Space):");
        ImGui::DragFloat3("##BaseOffset", &staffTrailBaseOffset.x, 0.01f, -10.0f, 10.0f);
        // ワールド座標を計算
        DirectX::XMMATRIX staffWorld = DirectX::XMLoadFloat4x4(&staff.transform);
        DirectX::XMVECTOR tipWorld = DirectX::XMVector3TransformCoord(
            DirectX::XMLoadFloat3(&staffTrailTipOffset), staffWorld);
        DirectX::XMVECTOR baseWorld = DirectX::XMVector3TransformCoord(
            DirectX::XMLoadFloat3(&staffTrailBaseOffset), staffWorld);
        // ベクトルをfloat3に変換
        DirectX::XMFLOAT3 tipWorldPos, baseWorldPos;
        DirectX::XMStoreFloat3(&tipWorldPos, tipWorld);
        DirectX::XMStoreFloat3(&baseWorldPos, baseWorld);
        // ワールド座標を表示
        ImGui::Text("World Positions:");
        ImGui::Text("Tip: (%.3f, %.3f, %.3f)", tipWorldPos.x, tipWorldPos.y, tipWorldPos.z);
        ImGui::Text("Base: (%.3f, %.3f, %.3f)", baseWorldPos.x, baseWorldPos.y, baseWorldPos.z);
        // オフセットをリセット
        if (ImGui::Button("Reset Offsets")) {
            staffTrailTipOffset = { 0.0f, 0.0f, 1.0f };
            staffTrailBaseOffset = { 0.0f, 0.0f, -0.5f };
        }
    }
}

void Player::RenderWeaponSpinDebugUI()
{
    if (!showWeaponSpinDebug) return;
    ImGui::Begin("Weapon Spin Debug", &showWeaponSpinDebug);
    ImGui::Text("杖をまっすぐ回転させるための調整値：");
    ImGui::Separator();
    // X軸の回転（UIは度数、内部はラジアン）
    float degreesX = DirectX::XMConvertToDegrees(debugSpinAngleX);
    if (ImGui::SliderFloat("X角度 (度数)", &degreesX, -180.0f, 180.0f)) {
        debugSpinAngleX = DirectX::XMConvertToRadians(degreesX);
    }
    // Y軸の回転
    float degreesY = DirectX::XMConvertToDegrees(debugSpinAngleY);
    if (ImGui::SliderFloat("Y角度 (度数)", &degreesY, -180.0f, 180.0f)) {
        debugSpinAngleY = DirectX::XMConvertToRadians(degreesY);
    }
    // Z軸の回転
    float degreesZ = DirectX::XMConvertToDegrees(debugSpinAngleZ);
    if (ImGui::SliderFloat("Z角度 (度数)", &degreesZ, -180.0f, 180.0f)) {
        debugSpinAngleZ = DirectX::XMConvertToRadians(degreesZ);
    }
    ImGui::Separator();
    ImGui::Text("現在の値（ラジアン）：");
    ImGui::Text("X: %.3f, Y: %.3f, Z: %.3f", debugSpinAngleX, debugSpinAngleY, debugSpinAngleZ);
    // デフォルト値にリセット
    if (ImGui::Button("デフォルトにリセット")) {
        debugSpinAngleX = -DirectX::XM_PIDIV4;
        debugSpinAngleY = 0.0f;
        debugSpinAngleZ = DirectX::XM_PIDIV2;
    }
    // 現在の値をクリップボードにコピー
    if (ImGui::Button("最終値をコピー")) {
        char buffer[256];
        sprintf_s(buffer, "X: %.3ff, Y: %.3ff, Z: %.3ff",
            debugSpinAngleX, debugSpinAngleY, debugSpinAngleZ);
        ImGui::SetClipboardText(buffer);
    }
    ImGui::End();
}

void Player::RenderManaDebug()
{
    // マナの情報を表示
    ImGui::Text("Mana: %.0f/%.0f (%.0f/sec)", currentMana, maxMana, manaRegenRate);
    // マナバーを表示
    ImGui::ProgressBar(currentMana / maxMana);
}

// ============================================================================
// PRIVATE METHODS - Render Debug Helpers
// ============================================================================

void Player::RenderWeaponCapsuleDebug(const RenderContext& rc, DebugRenderer& debugRenderer)
{
    auto& weaponParams = GetWeaponCapsuleParams();
    if (weaponParams.enabled)
    {
        DirectX::XMFLOAT3 base, tip;
        float radius = 0.0f;
        if (GetWeaponCapsule(base, tip, radius))
        {
            // 攻撃中は赤、それ以外は緑
            DirectX::XMFLOAT4 color = isWeaponHitActive
                ? DirectX::XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f }
            : DirectX::XMFLOAT4{ 0.0f, 1.0f, 0.0f, 1.0f };
            debugRenderer.DrawCapsuleFromEndpoints(rc, base, tip, radius, color);
        }
    }
}

void Player::RenderWeaponSphereDebug(const RenderContext& rc, DebugRenderer& debugRenderer)
{
    auto& sphereParams = GetWeaponSphereParams();
    if (sphereParams.enabled)
    {
        DirectX::XMFLOAT3 sphereCenter;
        float sphereRadius;
        if (GetWeaponSphere(sphereCenter, sphereRadius))
        {
            bool isColliding = isWeaponHitActive;
            // 攻撃中は赤、それ以外は緑
            DirectX::XMFLOAT4 sphereColor = isColliding
                ? DirectX::XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f }
            : DirectX::XMFLOAT4{ 0.0f, 1.0f, 0.0f, 1.0f };
            // スフィア描画
            debugRenderer.DrawSphere(rc, sphereCenter, sphereRadius, sphereColor);
        }
    }
}