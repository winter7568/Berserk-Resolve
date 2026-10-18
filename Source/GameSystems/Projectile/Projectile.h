#pragma once
#include <DirectXMath.h>
#include <memory>
#include <string>

#include "GameLib/RenderContext.h"
#include "ProjectileData.h"
#include "GameSystems/Effect/Effect.h"

class Projectile
{
public:
    // コンストラクタ
    Projectile(
        const std::string& ownerType,      // 発射元
        const std::string& projectileType, // 弾種類
        const DirectX::XMFLOAT3& position, // 初期位置
        const DirectX::XMFLOAT3& direction // 方向
    );

    void Update(float elapsedTime); // 更新
    void Render(const RenderContext& rc); // 描画

    // 基本取得
    const DirectX::XMFLOAT3& GetPosition() const { return base.position; } // 位置
    const DirectX::XMFLOAT3& GetDirection() const { return base.direction; } // 方向
    const DirectX::XMFLOAT4X4& GetWorldTransform() const { return base.worldTransform; } // 行列
    const std::string& GetProjectileType() const { return base.projectileType; } // 種類
    int GetDamage() const { return config.damage; } // ダメージ
    float GetImpactForce() const { return config.impactForce; } // 吹き飛ばし
    float GetRadius() const { return config.radius; } // 当たり判定
    float GetLifetime() const { return base.lifetime; } // 経過時間
    const std::string& GetHitEffect() const { return config.hitEffect; } // ヒットエフェクト
    bool IsActive() const { return base.isActive; } // 有効か
    void MarkForRemoval(); // 削除フラグ
    const std::string& GetOwnerType() const { return base.ownerType; } // 発射元

    // 形状や挙動取得
    ProjectileShape GetShape() const { return config.shape; } // 形状
    float GetCapsuleLength() const { return config.capsuleLength; } // カプセル長さ
    float GetSpeed() const { return config.speed; } // 速度
    float GetHomingStrength() const { return config.homingStrength; } // ホーミング強さ
    float GetMaxHomingAngle() const { return config.maxHomingAngle; } // 回転制限
    bool IsBeam() const { return config.isBeam; } // ビームか

    // 向き設定
    void SetOrientation(const DirectX::XMFLOAT3& orient) { base.orientation = orient; }
    const DirectX::XMFLOAT3& GetOrientation() const { return base.orientation; }

    // ビーム設定
    void SetBeamTarget(const DirectX::XMFLOAT3& target) { beamTarget = target; } // 目標
    void SetBeamConnected(bool connected) { beamStayConnected = connected; } // 接続状態
    void SetBeamSpawnPoint(const DirectX::XMFLOAT3& spawn) { beamSpawnPoint = spawn; } // 発射位置
    void SetBeamLength(float length) { beamLength = length; } // 長さ
    float GetBeamLength() const { return beamLength; } // 長さ取得

    // ビーム情報取得
    const DirectX::XMFLOAT3& GetBeamSpawnPoint() const { return beamSpawnPoint; }
    const DirectX::XMFLOAT3& GetBeamTarget() const { return beamTarget; }

    // エフェクトハンドル
    Effekseer::Handle GetVisualEffectHandle() const { return visualEffectHandle; }

private:
    // カプセル描画用端点取得
    void GetCapsuleEndpointsForRendering(DirectX::XMFLOAT3& outBase, DirectX::XMFLOAT3& outTip);

    // ホーミング処理
    void ApplyHoming(float elapsedTime);

    // 回転処理
    void ApplyRotationForProjectileType();

    ProjectileConfig config; // 設定データ
    ProjectileBaseData base; // 基本データ

    // ビーム用データ
    float beamLength = 15.0f; // 長さ
    DirectX::XMFLOAT3 beamTarget = { 0.0f, 0.0f, 0.0f }; // 目標位置
    DirectX::XMFLOAT3 beamSpawnPoint = { 0.0f, 0.0f, 0.0f }; // 発射位置
    bool beamStayConnected = false; // 発射元に固定するか

    Effekseer::Handle visualEffectHandle = -1;  // エフェクトハンドル
    bool effectSpawned = false;                 // 重複生成防止
    float edgeGlowTimer = 0.0f;                 // エッジ発光タイマー

    // プロジェクタイル定数
    struct ProjectileConstants
    {
        // エフェクト設定
        static constexpr float BOW_ELECTRIC_SCALE = 1.0f;
        static constexpr float BOW_ELECTRIC_INTENSITY = 0.3f;
        static constexpr float SLASH_EDGE_GLOW_INTERVAL = 0.08f;

        // 寿命設定
        static constexpr float BEAM_LIFETIME = 0.05f;
        static constexpr float MAX_PROJECTILE_LIFETIME = 20.0f;
        static constexpr float MAX_TRAVEL_DISTANCE = 400.0f;
        static constexpr float METEOR_IMPACT_LIFETIME = 1.5f;
        static constexpr float DANGER_ZONE_LIFETIME = 0.8f;

        // ホーミング設定
        static constexpr float MIN_HOMING_DISTANCE = 30.0f;
        static constexpr float FOV_CONE_ANGLE = 60.0f;
        static constexpr float HOMING_FRAMERATE = 60.0f;

        // モデル設定
        static constexpr float MODEL_BAKED_X_ROTATION_DEG = 90.0f;

        // エフェクトスケール
        static constexpr float EFFECT_SCALE = 1.0f;
    };
};