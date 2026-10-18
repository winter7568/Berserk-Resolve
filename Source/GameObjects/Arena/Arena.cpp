#include <imgui.h>

#include "Arena.h"
#include "GameLib/Graphics.h"
#include "GameLib/GpuResourceUtils.h"
#include "GameLib/ShapeRenderer.h"
#include "GameObjects/Player/Player.h"   
#include "GameObjects/Enemy/Enemy.h"    
#include "ShaderSystems/DistortionShader.h"

void Arena::Initialize(ID3D11Device* device)
{
    // ステージモデルを読み込む
    stage = std::make_unique<Model>(device, ArenaConstants::STAGE_MODEL_PATH, ArenaConstants::STAGE_MODEL_SCALE);
    stage->emissiveStrengthMultiplier = ArenaConstants::EMISSIVE_STRENGTH; // 発光の強さ
    stage->environmentStrength = ArenaConstants::ENVIRONMENT_STRENGTH; // 環境反射の強さ

    // デフォルトの発光色を保存する
    if (stage && !stage->GetMaterial().empty()) {
        defaultEmissiveColor = stage->GetMaterial()[0].emissiveColor;
        currentEmissiveColor = defaultEmissiveColor;
    }

    // ゲートトレイルを初期化する
    InitializeGateTrail(device);
}

void Arena::Update(float elapsedTime)
{
    // ステージのワールド変換を計算する
    DirectX::XMMATRIX S = DirectX::XMMatrixScaling(stageScale.x, stageScale.y, stageScale.z);
    DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(
        DirectX::XMConvertToRadians(stageRotation.x),
        DirectX::XMConvertToRadians(stageRotation.y),
        DirectX::XMConvertToRadians(stageRotation.z)
    );
    DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(stagePosition.x, stagePosition.y, stagePosition.z);
    DirectX::XMMATRIX world = S * R * T;

    DirectX::XMFLOAT4X4 worldTransform;
    DirectX::XMStoreFloat4x4(&worldTransform, world);

    // モデルの変換を更新する
    stage->UpdateTransform(worldTransform);

    // 境界制御を行う
    if (showBoundary)
    {
        ConstrainAllCharacters();
    }

    // ゲートキューブの制御
    ConstrainGateCube();
}

void Arena::Render(RenderContext& rc)
{
    ID3D11DeviceContext* dc = rc.deviceContext;

    // ステージを描画する
    Shader* shader = Graphics::Instance().GetShader(ShaderId::PBR);
    shader->Begin(rc);

    // カリングを無効にする
    dc->RSSetState(rc.renderState->GetRasterizerState(RasterizerState::SolidCullNone));
    shader->Draw(rc, stage.get());

    // カリングを元に戻す
    dc->RSSetState(rc.renderState->GetRasterizerState(RasterizerState::SolidCullBack));
    shader->End(rc);

    ShapeRenderer* shapeRenderer = Graphics::Instance().GetShapeRenderer();

    // 境界の描画
    if (showBoundary)
    {
        if (shapeRenderer)
        {
            DirectX::XMFLOAT3 cylinderPos = boundaryCenter;
            cylinderPos.y = 0.1f; // 少し上にずらす
            //shapeRenderer->RenderCylinder(rc, cylinderPos, boundaryRadius, boundaryHeight, boundaryColor, boundaryRotation);
        }
    }

    // パス境界の描画
    if (showBoundary && usePathBoundary)
    {
        if (shapeRenderer)
        {
            //Render the path boundary as a wireframe box
            //shapeRenderer->RenderBox(rc,
            //    pathCenter,
            //    { 0.0f, 0.0f, 0.0f },  // No rotation
            //    pathSize,
            //    DirectX::XMFLOAT4{ 1.0f, 0.5f, 0.0f, 0.5f });  // Orange
        }
    }

    // 中央キューブの描画
    if (showCentralCube)
    {
        if (shapeRenderer)
        {
            // Render a cube at the center
            //shapeRenderer->RenderBox(rc,
            //    centralCubeCenter,
            //    { 0.0f, 0.0f, 0.0f },  // No rotation
            //    centralCubeSize,
            //    centralCubeColor);
        }
    }

    // ゲートキューブの描画
    if (showGateCube)
    {
        if (shapeRenderer)
        {
            //shapeRenderer->RenderBox(rc,
            //    gateCubeCenter,
            //    { 0.0f, 0.0f, 0.0f },  // No rotation
            //    gateCubeSize,
            //    gateCubeColor);
        }
    }
}

void Arena::DrawGUI()
{
    // ステージ操作用ウィンドウ
    ImGui::Begin("Stage Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    // 位置
    ImGui::Text("Position");
    ImGui::Separator();
    ImGui::PushItemWidth(200);
    ImGui::DragFloat3("X, Y, Z", &stagePosition.x, 0.1f, -100.0f, 100.0f, "%.2f");

    // 回転
    ImGui::Text("Rotation (Degrees)");
    ImGui::Separator();
    ImGui::DragFloat3("Pitch, Yaw, Roll", &stageRotation.x, 0.5f, -360.0f, 360.0f, "%.1f°");

    // スケール
    ImGui::Text("Scale");
    ImGui::Separator();
    ImGui::DragFloat3("X, Y, Z", &stageScale.x, 0.05f, 0.01f, 50.0f, "%.2f");

    ImGui::SameLine();
    if (ImGui::Button("Uniform"))
        stageScale = { stageScale.x, stageScale.x, stageScale.x };

    ImGui::PopItemWidth();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 初期値に戻す
    if (ImGui::Button("Reset to Default"))
    {
        stagePosition = { ArenaConstants::DEFAULT_POSITION_X, ArenaConstants::DEFAULT_POSITION_Y, ArenaConstants::DEFAULT_POSITION_Z };
        stageScale = { ArenaConstants::DEFAULT_SCALE, ArenaConstants::DEFAULT_SCALE, ArenaConstants::DEFAULT_SCALE };
        stageRotation = { 0.0f, 0.0f, ArenaConstants::DEFAULT_ROTATION_Z };
    }

    ImGui::SameLine();

    // 値をコピーする
    if (ImGui::Button("Copy Values"))
    {
        char buffer[256];
        sprintf_s(buffer, "Position: (%.2f, %.2f, %.2f)\nScale: (%.2f, %.2f, %.2f)\nRotation: (%.1f, %.1f, %.1f)",
            stagePosition.x, stagePosition.y, stagePosition.z,
            stageScale.x, stageScale.y, stageScale.z,
            stageRotation.x, stageRotation.y, stageRotation.z);
        ImGui::SetClipboardText(buffer);
    }

    ImGui::End();

    // 境界設定ウィンドウ
    ImGui::Begin("Boundary Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    // 境界表示の切り替え
    ImGui::Checkbox("Show Boundary", &showBoundary);

    // 中心位置
    ImGui::Text("Boundary Center (X, Y, Z)");
    ImGui::Separator();
    ImGui::DragFloat3("Center", &boundaryCenter.x, 0.5f, -500.0f, 500.0f, "%.1f");

    // 半径
    ImGui::Text("Boundary Radius");
    ImGui::Separator();
    ImGui::DragFloat("Radius", &boundaryRadius, 0.5f, 1.0f, 500.0f, "%.1f");

    // 高さ
    ImGui::Text("Boundary Height");
    ImGui::Separator();
    ImGui::DragFloat("Height", &boundaryHeight, 0.5f, 1.0f, 500.0f, "%.1f");

    // 回転
    ImGui::Text("Boundary Rotation (Degrees)");
    ImGui::Separator();
    ImGui::DragFloat3("Rotation X, Y, Z", &boundaryRotation.x, 0.5f, -360.0f, 360.0f, "%.1f");

    // 色
    ImGui::Text("Boundary Color");
    ImGui::Separator();
    ImGui::ColorEdit4("Color", &boundaryColor.x, ImGuiColorEditFlags_AlphaBar);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 境界をリセットする
    if (ImGui::Button("Reset Boundary"))
    {
        boundaryCenter = { ArenaConstants::BOUNDARY_CENTER_X, ArenaConstants::BOUNDARY_CENTER_Y, ArenaConstants::BOUNDARY_CENTER_Z };
        boundaryRadius = ArenaConstants::BOUNDARY_RADIUS;
        boundaryColor = { 0.0f, 1.0f, 0.0f, 0.3f };
    }

    ImGui::SameLine();

    // ステージに合わせる
    if (ImGui::Button("Fit to Stage"))
    {
        boundaryCenter = { stagePosition.x, 0.1f, stagePosition.z };
        boundaryRadius = max(stageScale.x, stageScale.z) * 5.0f;
    }

    ImGui::End();

    // パス設定
    ImGui::Begin("Path Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Checkbox("Use Path Boundary", &usePathBoundary);

    // 中心
    ImGui::Text("Path Center (X, Y, Z)");
    ImGui::DragFloat3("Path Center", &pathCenter.x, 1.0f, -1000.0f, 1000.0f);

    // サイズ
    ImGui::Text("Path Size (W, H, D)");
    ImGui::DragFloat3("Path Size", &pathSize.x, 1.0f, 10.0f, 500.0f);

    ImGui::End();

    // 中央キューブ設定
    ImGui::Begin("Center Cube", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Checkbox("Show Cube", &showCentralCube);

    // 位置
    ImGui::Text("Position");
    ImGui::DragFloat3("Center", &centralCubeCenter.x, 0.5f, -100.0f, 100.0f, "%.1f");

    // サイズ
    ImGui::Text("Size (W, H, D)");
    ImGui::DragFloat3("Dimensions", &centralCubeSize.x, 0.5f, 1.0f, 100.0f, "%.1f");

    // 色
    ImGui::Text("Color");
    ImGui::ColorEdit4("Cube Color", &centralCubeColor.x, ImGuiColorEditFlags_AlphaBar);

    // リセット
    if (ImGui::Button("Reset"))
    {
        centralCubeCenter = { ArenaConstants::CENTER_CUBE_CENTER_X, ArenaConstants::CENTER_CUBE_CENTER_Y, ArenaConstants::CENTER_CUBE_CENTER_Z };
        centralCubeSize = { ArenaConstants::CENTER_CUBE_SIZE_X, ArenaConstants::CENTER_CUBE_SIZE_Y, ArenaConstants::CENTER_CUBE_SIZE_Z };
        centralCubeColor = { 1.0f, 0.0f, 0.0f, 0.3f };
    }

    ImGui::End();

    // ゲートトレイル設定
    ImGui::Begin("Gate Trail Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    // 有効切り替え
    ImGui::Checkbox("Gate Trail Active", &gateTrailActive);

    ImGui::Separator();

    // 位置設定
    ImGui::Text("Gate Positions");
    ImGui::Separator();
    ImGui::DragFloat3("Entrance Point", &gateEntrancePoint.x, 1.0f, -1000.0f, 1000.0f, "%.1f");
    ImGui::DragFloat3("Exit Point", &gateExitPoint.x, 1.0f, -1000.0f, 1000.0f, "%.1f");

    ImGui::Separator();

    // 振動設定
    ImGui::Text("Oscillation Settings");
    ImGui::Separator();

    TrailSystem::TrailConfig& config = gateTrail.GetConfig();

    // 速度
    ImGui::DragFloat("Oscillation Speed", &oscillationSpeed, 0.5f, 1.0f, 100.0f, "%.1f");
    ImGui::SameLine(); ImGui::Text("(%.1f Hz)", oscillationSpeed / (2.0f * 3.14159f));

    // 振れ幅
    ImGui::DragFloat("Oscillation Range", &oscillationRange, 0.5f, 1.0f, 500.0f, "%.1f");

    // 現在の値
    float currentOffset = sinf(gateTrailTimer * oscillationSpeed) * oscillationRange;
    ImGui::Text("Current Offset: %.2f", currentOffset);

    ImGui::Separator();

    // トレイル設定
    ImGui::Text("Trail Configuration");
    ImGui::Separator();

    ImGui::DragFloat("Trail Lifetime", &config.maxLifetime, 0.1f, 0.1f, 5.0f, "%.2f s");
    ImGui::DragInt("Max Segments", &config.maxSegments, 1, 10, 200);

    // テクスチャ表示
    ImGui::Text("Normal Map: %s", config.normalMapPath.empty() ? "None" : config.normalMapPath.c_str());
    ImGui::Text("Mask Texture: %s", config.maskPath.empty() ? "None" : config.maskPath.c_str());

    ImGui::Separator();

    // デバッグ情報
    ImGui::Text("Debug & Reset");
    ImGui::Separator();

    ImGui::Text("Trail Vertices: %d", gateTrail.GetVertexCount());
    ImGui::Text("Trail Indices: %d", gateTrail.GetIndexCount());
    ImGui::Text("Timer: %.2f", gateTrailTimer);

    // リセット
    if (ImGui::Button("Reset Oscillation")) {
        oscillationSpeed = ArenaConstants::OSCILLATION_SPEED;
        oscillationRange = ArenaConstants::OSCILLATION_RANGE;
    }

    ImGui::SameLine();

    if (ImGui::Button("Reset Trail Settings")) {
        config.maxLifetime = ArenaConstants::GATE_TRAIL_LIFETIME;
        config.maxSegments = ArenaConstants::GATE_TRAIL_SEGMENTS;
    }

    ImGui::SameLine();

    if (ImGui::Button("Reset Positions")) {
        gateEntrancePoint = { ArenaConstants::GATE_ENTRANCE_X, ArenaConstants::GATE_ENTRANCE_Y, ArenaConstants::GATE_ENTRANCE_Z };
        gateExitPoint = { ArenaConstants::GATE_EXIT_X, ArenaConstants::GATE_EXIT_Y, ArenaConstants::GATE_EXIT_Z };
    }

    ImGui::End();

    // ゲートキューブ設定
    ImGui::Begin("Gate Cube Collider", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Checkbox("Show Gate Cube", &showGateCube);
    ImGui::Checkbox("Solid Collision", &gateCubeSolid);

    // 位置
    ImGui::Text("Position");
    ImGui::DragFloat3("Gate Center", &gateCubeCenter.x, 1.0f, -1000.0f, 1000.0f, "%.1f");

    // サイズ
    ImGui::Text("Size (W, H, D)");
    ImGui::DragFloat3("Gate Size", &gateCubeSize.x, 1.0f, 1.0f, 500.0f, "%.1f");

    // 色
    ImGui::Text("Color");
    ImGui::ColorEdit4("Gate Color", &gateCubeColor.x, ImGuiColorEditFlags_AlphaBar);

    // リセット
    if (ImGui::Button("Reset Gate Cube")) {
        gateCubeCenter = { ArenaConstants::GATE_CUBE_CENTER_X, ArenaConstants::GATE_CUBE_CENTER_Y, ArenaConstants::GATE_CUBE_CENTER_Z };
        gateCubeSize = { ArenaConstants::GATE_CUBE_SIZE_X, ArenaConstants::GATE_CUBE_SIZE_Y, ArenaConstants::GATE_CUBE_SIZE_Z };
    }

    ImGui::End();
}

void Arena::ConstrainGateCube()
{
    if (!gateCubeSolid) return; // 無効なら処理しない

    // キューブの範囲を計算する
    float minX = gateCubeCenter.x - gateCubeSize.x;
    float maxX = gateCubeCenter.x + gateCubeSize.x;
    float minY = gateCubeCenter.y - gateCubeSize.y;
    float maxY = gateCubeCenter.y + gateCubeSize.y;
    float minZ = gateCubeCenter.z - gateCubeSize.z;
    float maxZ = gateCubeCenter.z + gateCubeSize.z;

    // プレイヤー情報を取得する
    Player& player = Player::Instance();

    DirectX::XMFLOAT3 playerBase, playerTip;
    player.GetBodyCapsuleEndpoints(playerBase, playerTip);
    float playerRadius = player.GetScaledBodyRadius();

    // キューブ内にいるか判定する
    bool baseInside = (playerBase.x >= minX && playerBase.x <= maxX &&
        playerBase.y >= minY && playerBase.y <= maxY &&
        playerBase.z >= minZ && playerBase.z <= maxZ);

    bool tipInside = (playerTip.x >= minX && playerTip.x <= maxX &&
        playerTip.y >= minY && playerTip.y <= maxY &&
        playerTip.z >= minZ && playerTip.z <= maxZ);

    if (baseInside || tipInside) {
        // 位置を元に戻す
        DirectX::XMFLOAT3 newPos = lastValidPlayerPos;

        float buffer = 1.0f;
        if (newPos.x <= minX) newPos.x = minX - buffer;
        else if (newPos.x >= maxX) newPos.x = maxX + buffer;
        player.SetPosition(newPos);

        // 水平方向の速度を止める
        DirectX::XMFLOAT3 vel = player.GetVelocity();
        vel.x = 0.0f;
        vel.z = 0.0f;
        player.SetVelocity(vel);
    }
    else {
        // 正常な位置を保存する
        lastValidPlayerPos = player.GetPosition();
    }
}

void Arena::ConstrainAllCharacters()
{
    if (!showBoundary)
        return;

    // プレイヤーの位置を取得する
    Player& player = Player::Instance();
    DirectX::XMFLOAT3 playerPos = player.GetPosition();

    if (usePathBoundary)
    {
        // パス内に制限する
        float minX = pathCenter.x - pathSize.x;
        float maxX = pathCenter.x + pathSize.x;
        float minZ = pathCenter.z - pathSize.z;
        float maxZ = pathCenter.z + pathSize.z;

        if (playerPos.x < minX) playerPos.x = minX;
        else if (playerPos.x > maxX) playerPos.x = maxX;
        if (playerPos.z < minZ) playerPos.z = minZ;
        else if (playerPos.z > maxZ) playerPos.z = maxZ;
        player.SetPosition(playerPos);

        // アリーナに入ったか判定する
        float distToArena = sqrtf(
            (playerPos.x - boundaryCenter.x) * (playerPos.x - boundaryCenter.x) +
            (playerPos.z - boundaryCenter.z) * (playerPos.z - boundaryCenter.z)
        );

        if (distToArena < boundaryRadius)
        {
            usePathBoundary = false;
            playerHasEntered = true;
        }
    }
    else
    {
        // 円形の境界内に制限する
        DirectX::XMFLOAT3 cylinderPos = boundaryCenter;
        cylinderPos.y += boundaryHeight * 0.5f;

        float dx = playerPos.x - cylinderPos.x;
        float dz = playerPos.z - cylinderPos.z;
        float distance = sqrtf(dx * dx + dz * dz);
        if (distance > boundaryRadius)
        {
            float pushRatio = boundaryRadius / distance;
            DirectX::XMFLOAT3 newPos = {
                cylinderPos.x + dx * pushRatio,
                playerPos.y,
                cylinderPos.z + dz * pushRatio
            };
            player.SetPosition(newPos);
        }
    }

    // 敵も同様に制限する
    {
        Enemy& enemy = Enemy::Instance();
        DirectX::XMFLOAT3 enemyPos = enemy.GetPosition();
        float dx = enemyPos.x - boundaryCenter.x;
        float dz = enemyPos.z - boundaryCenter.z;
        float distance = sqrtf(dx * dx + dz * dz);
        if (distance > boundaryRadius)
        {
            float pushRatio = boundaryRadius / distance;
            DirectX::XMFLOAT3 newPos = {
                boundaryCenter.x + dx * pushRatio,
                enemyPos.y,
                boundaryCenter.z + dz * pushRatio
            };
            enemy.SetPosition(newPos);
        }
    }
}

void Arena::InitializeGateTrail(ID3D11Device* device)
{
    TrailSystem::TrailConfig trailConfig;

    // 初期設定を行う
    trailConfig.startOffset = { 0.0f, 0.0f, 0.0f };
    trailConfig.endOffset = { 0.0f, 0.0f, 0.0f };
    trailConfig.maxLifetime = ArenaConstants::GATE_TRAIL_LIFETIME;
    trailConfig.maxSegments = ArenaConstants::GATE_TRAIL_SEGMENTS;

    // 使用するテクスチャを設定する
    trailConfig.normalMapPath = ArenaConstants::NORMAL_MAP_PATH;
    trailConfig.maskPath = ArenaConstants::MASK_PATH;

    // トレイルを初期化する
    gateTrail.Initialize(device, trailConfig);
    gateTrail.SetActive(true);
}

void Arena::UpdateGateTrail(float elapsedTime)
{
    if (!gateTrailActive) return; // 無効なら処理しない

    // タイマーを更新する
    gateTrailTimer += elapsedTime;

    // 上下に揺らす値を計算する
    float verticalOffset = sinf(gateTrailTimer * oscillationSpeed) * oscillationRange;

    // 現在の位置を計算する
    float entranceY = gateEntrancePoint.y + verticalOffset;
    float exitY = gateExitPoint.y + verticalOffset;

    // ゲートトレイルは武器にアタッチされていない独立したエフェクトのため、
    // トレイルシステムが要求する変換行列を手動で作成する。
    // 回転・拡大は行わず、位置のみを設定する（単位行列 + 平行移動）。
    // これにより、トレイルの始点がゲートの入り口位置に設定される。
    DirectX::XMFLOAT4X4 fakeTransform = {
        1, 0, 0, 0,                                     // 右方向（回転なし）
        0, 1, 0, 0,                                     // 上方向（回転なし）
        0, 0, 1, 0,                                     // 前方向（回転なし）
        gateEntrancePoint.x, entranceY, gateEntrancePoint.z, 1  // ワールド位置
    };

    // トレイル設定を取得する
    TrailSystem::TrailConfig& config = gateTrail.GetConfig();

    // 始点: 変換行列の位置（入り口）を基準とするため (0,0,0)
    config.startOffset = { 0.0f, 0.0f, 0.0f };

    // 終点: 始点（入り口）からの相対位置で指定する
    config.endOffset = {
        gateExitPoint.x - gateEntrancePoint.x,  // X方向の距離
        exitY - entranceY,                      // Y方向の距離（上下の揺れを含む）
        gateExitPoint.z - gateEntrancePoint.z   // Z方向の距離
    };

    // トレイルを更新する
    gateTrail.Update(elapsedTime, fakeTransform);
}

void Arena::RenderGateTrail(const RenderContext& rc)
{
    if (!gateTrailActive) return;

    // 揺れの値を計算する
    float verticalOffset = sinf(gateTrailTimer * oscillationSpeed) * oscillationRange;

    // 現在の入り口位置
    DirectX::XMFLOAT3 currentEntrance = {
        gateEntrancePoint.x,
        gateEntrancePoint.y + verticalOffset,
        gateEntrancePoint.z
    };

    // 現在の出口位置
    DirectX::XMFLOAT3 currentExit = {
        gateExitPoint.x,
        gateExitPoint.y + verticalOffset,
        gateExitPoint.z
    };

    // 頂点がない場合は描画しない
    if (gateTrail.GetVertexCount() == 0) {
        return;
    }

    // シェーダーを取得する
    DistortionShader* distortionShader = static_cast<DistortionShader*>(
        Graphics::Instance().GetShader(ShaderId::Distortion));

    if (!distortionShader) {
        return;
    }

    // シーンテクスチャを取得する
    ID3D11ShaderResourceView* sceneSRV =
        Graphics::Instance().GetFrameBuffer(FrameBufferId::Scene)->GetColorMap();

    // シェーダーを開始する
    distortionShader->SetEnemyTrail();
    distortionShader->Begin(rc);

    // トレイルを描画する
    distortionShader->DrawTrail(rc,
        gateTrail.GetVertexBuffer(),
        gateTrail.GetIndexBuffer(),
        gateTrail.GetVertexCount(),
        gateTrail.GetIndexCount(),
        sceneSRV,
        gateTrail.GetNormalMap(),
        gateTrail.GetMask()
    );

    distortionShader->End(rc);

    // リソースをクリアする
    ID3D11ShaderResourceView* nullSRVs[] = { nullptr, nullptr, nullptr };
    rc.deviceContext->PSSetShaderResources(
        CharacterConstants::TRAIL_TEXTURE_SLOT_START,
        CharacterConstants::TRAIL_TEXTURE_SLOT_COUNT,
        nullSRVs
    );
}

void Arena::SetEmissiveColor(const DirectX::XMFLOAT3& color)
{
    // 発光色を設定する
    currentEmissiveColor = color;

    // 全マテリアルに適用する
    if (stage) {
        for (auto& material : stage->GetMaterial())
        {
            const_cast<Model::Material&>(material).emissiveColor = color;
        }
    }
}

void Arena::ResetEmissiveColor()
{
    // 発光色を元に戻す
    SetEmissiveColor(defaultEmissiveColor);
}

void Arena::FlashEmissive(const DirectX::XMFLOAT3& color, float duration)
{
    // 一時的に発光させる
    flashColor = color;
    emissiveFlashTimer = duration;
    SetEmissiveColor(color);
}

void Arena::Reset()
{
    // 境界状態をリセットする
    playerHasEntered = false;
    usePathBoundary = true;

    // 発光をリセットする
    ResetEmissiveColor();
    emissiveFlashTimer = 0.0f;

    // タイマーをリセットする
    gateTrailTimer = 0.0f;
}