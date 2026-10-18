#pragma once
#include <string>
#include <unordered_map>
#include <DirectXMath.h>

// 弾の形状タイプ
enum class ProjectileShape
{
    SPHERE,
    CAPSULE,
    BEAM
};

// ============================================================================
// プロジェクタイル基本データ
// ============================================================================
struct ProjectileBaseData
{
    // 基本情報
    std::string ownerType;       // 発射元
    std::string projectileType;  // 種類

    // 位置と状態
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT3 direction;
    DirectX::XMFLOAT3 orientation = { 0.0f, 0.0f, 0.0f }; // カプセル用
    DirectX::XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT4X4 worldTransform;

    // 生存管理
    bool isActive = true;
    float lifetime = 0.0f;

    // 描画
    std::shared_ptr<Model> model;

    void UpdateTransform()
    {
        DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
        DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
        DirectX::XMMATRIX world = S * T;
        DirectX::XMStoreFloat4x4(&worldTransform, world);
    }
};

// ============================================================================
// プロジェクタイル設定
// ============================================================================
struct ProjectileConfig
{
    // 基本設定
    std::string type;
    float manaCost = 0.0f;
    int damage = 10;
    float impactForce = 5.0f;
    float speed = 50.0f;
    float radius = 1.0f;

    // エフェクト
    std::string hitEffect = "default_hit";
    std::string spawnNode = "staff_tip";

    // 挙動フラグ
    bool penetrates = false; // 貫通
    bool homing = false;     // ホーミング

    // 形状設定
    ProjectileShape shape = ProjectileShape::SPHERE;
    float capsuleLength = 2.0f;  // カプセル長さ

    // ホーミング設定
    float homingStrength = 0.5f; // 強さ
    float maxHomingAngle = 0.1f; // 回転制限

    // ビーム設定
    bool isBeam = false;
    float beamMaxRange = 20.0f;
    bool beamStayConnected = false;
};

// ============================================================================
// プロジェクタイルデータ管理
// ============================================================================
class ProjectileDataManager
{
public:
    static ProjectileDataManager& Instance()
    {
        static ProjectileDataManager instance;
        return instance;
    }

    // コピー禁止
    ProjectileDataManager(const ProjectileDataManager&) = delete;
    ProjectileDataManager& operator=(const ProjectileDataManager&) = delete;

    // 設定取得
    const ProjectileConfig& GetConfig(const std::string& projectileType)
    {
        auto it = projectileConfigs.find(projectileType);
        if (it != projectileConfigs.end())
        {
            return it->second;
        }
        return defaultConfig;
    }

    void Initialize()
    {
        // ================================================================
        // プレイヤー弾
        // ================================================================

        // ピストル弾
        projectileConfigs["pistol"] = {
            "pistol",           // type
            10.0f,              // manaCost
            8,                  // damage
            5.0f,               // impactForce
            320.0f,             // speed
            0.5f,               // radius
            "spark_hit",        // hitEffect
            "hand_l"            // spawnNode
        };

        // スタッフ弾
        projectileConfigs["staff"] = {
            "staff",            // type
            15.0f,              // manaCost
            16,                  // damage
            8.0f,               // impactForce
            340.0f,             // speed
            1.0f,               // radius
            "magic_hit",        // hitEffect
            "staff_tip"         // spawnNode
        };

        // クイックショット
        projectileConfigs["bow_quick"] = {
            "bow_quick",        // type
            5.0f,               // manaCost
            7,                  // damage
            3.0f,               // impactForce
            200.0f,             // speed
            0.3f,               // radius
            "arrow_hit",        // hitEffect
            "staff_tip",        // spawnNode
            false,              // penetrates
            true                // homing
        };

        // チャージショット
        projectileConfigs["bow_charged"] = {
            "bow_charged",      // type
            30.0f,              // manaCost
            40,                 // damage
            15.0f,              // impactForce
            200.0f,             // speed
            2.0f,               // radius
            "explosion_hit",    // hitEffect
            "staff_base"        // spawnNode
        };

        // ================================================================
        // 空中攻撃
        // ================================================================

        // 空気斬り
        projectileConfigs["air_slash"] = {
            "air_slash",        // type
            20.0f,              // manaCost
            25,                 // damage
            10.0f,              // impactForce
            340.0f,             // speed
            0.3f,               // radius
            "slash_hit",        // hitEffect
            "staff_tip",        // spawnNode
            false,              // penetrates
            true,               // homing
            ProjectileShape::CAPSULE, // shape
            12.0f,              // capsuleLength
            0.2f,               // homingStrength
            0.08f,              // maxHomingAngle
            false               // isBeam
        };

        // 空気ビーム
        projectileConfigs["air_beam"] = {
            "air_beam",         // type
            35.0f,              // manaCost
            55,                 // damage
            25.0f,              // impactForce
            0.0f,               // speed (移動なし)
            0.5f,               // radius
            "beam_hit",         // hitEffect
            "staff_tip",        // spawnNode
            false,              // penetrates
            false,              // homing
            ProjectileShape::BEAM, // shape
            1.0f,               // capsuleLength
            0.0f,               // homingStrength
            0.0f,               // maxHomingAngle
            true,               // isBeam
            200.0f,             // beamMaxRange
            true                // beamStayConnected
        };

        // ================================================================
        // 敵弾
        // ================================================================

        // 敵オーブ
        projectileConfigs["enemy_orb"] = {
            "enemy_orb",        // type
            0.0f,               // manaCost
            8,                  // damage
            5.0f,               // impactForce
            320.0f,             // speed
            0.5f,               // radius
            "enemy_spark_hit",  // hitEffect
            "hand_l"            // spawnNode
        };

        // メテオインパクト
        projectileConfigs["berserk_meteor_impact"] = {
            "berserk_meteor_impact", // type
            0.0f,               // manaCost
            60,                 // damage
            30.0f,              // impactForce
            0.0f,               // speed (移動なし)
            50.0f,              // radius
            "meteor_explosion", // hitEffect
            "hand_l",           // spawnNode
            false,              // penetrates
            false,              // homing
            ProjectileShape::SPHERE // shape
        };

        // デンジャーゾーン
        projectileConfigs["berserk_danger_zone_hit"] = {
            "berserk_danger_zone_hit", // type
            0.0f,               // manaCost
            40,                 // damage
            35.0f,              // impactForce
            0.0f,               // speed (移動なし)
            8.0f,               // radius
            "meteor_explosion_small", // hitEffect
            "hand_l",           // spawnNode
            false,              // penetrates
            false,              // homing
            ProjectileShape::SPHERE // shape
        };
    }

private:
    ProjectileDataManager()
    {
        Initialize();
    }

    std::unordered_map<std::string, ProjectileConfig> projectileConfigs;

    // デフォルト設定（見つからない場合）
    ProjectileConfig defaultConfig = {
        "default",          // type
        0.0f,               // manaCost
        10,                 // damage
        5.0f,               // impactForce
        50.0f,              // speed
        1.0f,               // radius
        "default_hit",      // hitEffect
        "staff_tip"         // spawnNode
    };
};