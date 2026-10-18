#pragma once
#include <memory>
#include <wrl/client.h>

#include "ArenaConstants.h"
#include "GameObjects/Character/Character.h"
#include "GameLib/Model.h"
#include "GameLib/RenderContext.h"
#include "ShaderSystems/TrailSystem.h"

class Arena
{
public:
    // シングルトン取得
    static Arena& Instance()
    {
        static Arena instance;
        return instance;
    }

    // コピー禁止
    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;

    // 初期化
    void Initialize(ID3D11Device* device);

    // 更新
    void Update(float elapsedTime);

    // 描画
    void Render(RenderContext& rc);

    // GUI描画
    void DrawGUI();

    // モデル取得
    Model* GetModel() const { return stage.get(); }

    // ステージ情報取得
    const DirectX::XMFLOAT3& GetPosition() const { return stagePosition; }
    const DirectX::XMFLOAT3& GetScale() const { return stageScale; }
    const DirectX::XMFLOAT3& GetRotation() const { return stageRotation; }

    // 境界情報取得
    const DirectX::XMFLOAT3& GetBoundaryCenter() const { return boundaryCenter; }
    float GetBoundaryRadius() const { return boundaryRadius; }
    bool IsBoundaryVisible() const { return showBoundary; }
    const DirectX::XMFLOAT3& GetBoundaryRotation() const { return boundaryRotation; }

    // パス境界情報取得
    const DirectX::XMFLOAT3& GetPathCenter() const { return pathCenter; }
    const DirectX::XMFLOAT3& GetPathSize() const { return pathSize; }
    bool UsingPathBoundary() const { return usePathBoundary; }

    // 境界制限処理
    void ConstrainAllCharacters();
    void ConstrainGateCube();

    // プレイヤー進入状態取得
    bool PlayerHasEntered() const { return playerHasEntered; }

    // トレイル描画と更新
    void RenderGateTrail(const RenderContext& rc);
    void UpdateGateTrail(float elapsedTime);

    // 発光制御
    void SetEmissiveColor(const DirectX::XMFLOAT3& color);
    void ResetEmissiveColor();
    void FlashEmissive(const DirectX::XMFLOAT3& color, float duration);

    // リセット
    void Reset();
    // ゲートキューブ関連 
    const DirectX::XMFLOAT3& GetGateCubeCenter() const { return gateCubeCenter; }
    void SetGateCubeCenter(const DirectX::XMFLOAT3& center) { gateCubeCenter = center; }

    const DirectX::XMFLOAT3& GetGateCubeSize() const { return gateCubeSize; }
    void SetGateCubeSize(const DirectX::XMFLOAT3& size) { gateCubeSize = size; }

    bool IsGateCubeVisible() const { return showGateCube; }
    void SetGateCubeVisible(bool visible) { showGateCube = visible; }

    bool IsGateCubeSolid() const { return gateCubeSolid; }
    void SetGateCubeSolid(bool solid) { gateCubeSolid = solid; }

private:
    Arena() = default;
    ~Arena() = default;

    std::unique_ptr<Model> stage;

    // ステージ変換情報
    DirectX::XMFLOAT3 stagePosition = { ArenaConstants::DEFAULT_POSITION_X, ArenaConstants::DEFAULT_POSITION_Y, ArenaConstants::DEFAULT_POSITION_Z };
    DirectX::XMFLOAT3 stageScale = { ArenaConstants::DEFAULT_SCALE, ArenaConstants::DEFAULT_SCALE, ArenaConstants::DEFAULT_SCALE };
    DirectX::XMFLOAT3 stageRotation = { 0.0f, 0.0f, ArenaConstants::DEFAULT_ROTATION_Z };

    // 境界設定
    DirectX::XMFLOAT3 boundaryCenter = { ArenaConstants::BOUNDARY_CENTER_X, ArenaConstants::BOUNDARY_CENTER_Y, ArenaConstants::BOUNDARY_CENTER_Z };
    float boundaryRadius = ArenaConstants::BOUNDARY_RADIUS;
    float boundaryHeight = ArenaConstants::BOUNDARY_HEIGHT;
    DirectX::XMFLOAT3 boundaryRotation = { 0.0f, ArenaConstants::BOUNDARY_ROTATION_Y, 0.0f };
    bool showBoundary = true;
    DirectX::XMFLOAT4 boundaryColor = { 0.0f, 1.0f, 0.0f, 0.3f };
    bool playerHasEntered = false;

    // パス境界
    DirectX::XMFLOAT3 pathCenter = { ArenaConstants::PATH_CENTER_X, ArenaConstants::PATH_CENTER_Y, ArenaConstants::PATH_CENTER_Z };
    DirectX::XMFLOAT3 pathSize = { ArenaConstants::PATH_SIZE_X, ArenaConstants::PATH_SIZE_Y, ArenaConstants::PATH_SIZE_Z };
    bool usePathBoundary = true;

    // ゲートトレイル
    TrailSystem gateTrail;
    DirectX::XMFLOAT3 gateEntrancePoint = { ArenaConstants::GATE_ENTRANCE_X, ArenaConstants::GATE_ENTRANCE_Y, ArenaConstants::GATE_ENTRANCE_Z };
    DirectX::XMFLOAT3 gateExitPoint = { ArenaConstants::GATE_EXIT_X, ArenaConstants::GATE_EXIT_Y, ArenaConstants::GATE_EXIT_Z };
    bool gateTrailActive = true;
    float gateTrailTimer = 0.0f;
    // ゲート用コライダー
    DirectX::XMFLOAT3 gateCubeCenter = { ArenaConstants::GATE_CUBE_CENTER_X, ArenaConstants::GATE_CUBE_CENTER_Y, ArenaConstants::GATE_CUBE_CENTER_Z };
    DirectX::XMFLOAT3 gateCubeSize = { ArenaConstants::GATE_CUBE_SIZE_X, ArenaConstants::GATE_CUBE_SIZE_Y, ArenaConstants::GATE_CUBE_SIZE_Z };
    bool showGateCube = true;
    bool gateCubeSolid = true;
    DirectX::XMFLOAT4 gateCubeColor = { 1.0f, 0.5f, 0.0f, 0.3f };
    DirectX::XMFLOAT3 lastValidPlayerPos = {};

    // 振動設定
    float oscillationSpeed = ArenaConstants::OSCILLATION_SPEED;
    float oscillationRange = ArenaConstants::OSCILLATION_RANGE;

    // トレイル初期化
    void InitializeGateTrail(ID3D11Device* device);

    // 中央キューブ
    DirectX::XMFLOAT3 centralCubeCenter = { ArenaConstants::CENTER_CUBE_CENTER_X, ArenaConstants::CENTER_CUBE_CENTER_Y, ArenaConstants::CENTER_CUBE_CENTER_Z };
    DirectX::XMFLOAT3 centralCubeSize = { ArenaConstants::CENTER_CUBE_SIZE_X, ArenaConstants::CENTER_CUBE_SIZE_Y, ArenaConstants::CENTER_CUBE_SIZE_Z };
    DirectX::XMFLOAT4 centralCubeColor = { 1.0f, 0.0f, 0.0f, 0.3f };
    bool showCentralCube = true;

    // 発光設定
    DirectX::XMFLOAT3 defaultEmissiveColor = {};
    DirectX::XMFLOAT3 currentEmissiveColor = { ArenaConstants::EMISSIVE_COLOR_R, ArenaConstants::EMISSIVE_COLOR_G, ArenaConstants::EMISSIVE_COLOR_B };
    float emissiveFlashTimer = 0.0f;
    DirectX::XMFLOAT3 flashColor = { ArenaConstants::FLASH_COLOR_R, ArenaConstants::FLASH_COLOR_G, ArenaConstants::FLASH_COLOR_B };
};