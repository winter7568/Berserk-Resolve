#pragma once
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <string>
#include <map>

#include "CharacterConstants.h"
#include "CharacterEnums.h"
#include "GameLib/RenderContext.h"
#include "GameLib/ShapeRenderer.h"
#include "GameLib/Model.h"
#include "StateMachine/StateManager.h"
#include "ShaderSystems/TrailSystem.h"
#include "Audio/Audio.h"

class Character
{
public:
    Character() {}
    virtual ~Character() {}

    // 純粋仮想関数（派生クラスで実装が必要）
    virtual void Initialize() = 0;                  // 初期化
    virtual void Update(float elapsedTime) = 0;     // 更新処理
    virtual void Reset() = 0;                       // リセット
    virtual void Render(const RenderContext& rc) = 0; // 描画
    virtual std::string GetCurrentStateName() const = 0; // 状態名取得

    // 共通構造体（PlayerとEnemyで共有）

    // 武器オブジェクト
    struct WeaponObject {
        DirectX::XMFLOAT3 position = { 0,0,0 };   // 位置
        DirectX::XMFLOAT3 angle = { 0,0,0 };      // 回転
        DirectX::XMFLOAT3 scale = { 1,1,1 };      // サイズ
        DirectX::XMFLOAT4X4 transform = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 }; // 行列
        std::unique_ptr<Model> model;             // モデル
    };

    // サウンド読み込み情報
    template<typename T>
    struct SoundLoadInfo {
        T soundId;           // サウンドID
        const char* filename; // ファイル名

        // コンストラクタ
        SoundLoadInfo(T id, const char* file) : soundId(id), filename(file) {}
    };

    // トレイル用オフセット
    struct TrailOffsets {
        DirectX::XMFLOAT3 tipOffset = {};   // 先端
        DirectX::XMFLOAT3 baseOffset = {};  // 根元
    };

    // デバッグ用カプセル
    struct DebugCapsuleParams {
        bool enabled = true;                // 有効/無効
        float radius = 0.0f;                // 半径
        float height = 0.0f;                // 高さ
        DirectX::XMFLOAT3 offset = {};      // 位置オフセット
        DirectX::XMFLOAT3 rotation = {};    // 回転（度）
        DirectX::XMFLOAT4 color = {};       // 色
    };

    // デバッグ用球
    struct DebugSphereParams {
        bool enabled = true;                // 有効/無効
        float radius = 0.0f;                // 半径
        DirectX::XMFLOAT3 offset = {};      // 位置オフセット
        DirectX::XMFLOAT4 color = {};       // 色
    };

    // ヒットタイプ
    enum class HitType {
        Regular,    // 通常ヒット
        Stun,       // 小さい吹き飛び
        Launch      // 大きく打ち上げ
    };

    // ヒット情報
    struct HitData {
        int damage = 0;                     // ダメージ
        DirectX::XMFLOAT3 hitOrigin = {};  // ヒット位置
        DirectX::XMFLOAT3 hitDirection = {}; // 方向
        bool isBigHit = false;             // 強い攻撃か
        HitType hitType = HitType::Regular;// ヒット種類
        int comboStep = 0;                 // コンボ段階
        float hitForce = 0.0f;             // 力
    };

    // 共通処理
    void UpdateTransform();                // 行列更新
    void Turn(float elapsedTime, float vx, float vz, float speed); // 回転
    void UpdateInvincibleTimer(float elapsedTime); // 無敵時間更新

    // Getter / Setter

    // 位置
    const DirectX::XMFLOAT3& GetPosition() const { return position; }
    void SetPosition(const DirectX::XMFLOAT3& pos) { position = pos; }

    // 回転
    const DirectX::XMFLOAT3& GetAngle() const { return angle; }
    void SetAngle(const DirectX::XMFLOAT3& ang) { angle = ang; }

    // スケール
    const DirectX::XMFLOAT3& GetScale() const { return scale; }
    void SetScale(const DirectX::XMFLOAT3& scl) { scale = scl; }

    // 行列
    const DirectX::XMFLOAT4X4& GetTransform() const { return transform; }

    // 速度
    const DirectX::XMFLOAT3& GetVelocity() const { return velocity; }
    void SetVelocity(const DirectX::XMFLOAT3& vel) { velocity = vel; }

    // 前方向ベクトル
    DirectX::XMFLOAT3 GetForwardVector() const {
        return { sinf(angle.y), 0.0f, cosf(angle.y) };
    }

    // 死亡状態
    bool IsDead() const { return isDead; }
    void SetDead(bool dead) { isDead = dead; }

    // 表示
    void SetVisibility(bool visible) { isVisible = visible; }
    bool IsVisible() const { return isVisible && (model->GetDissolveThreshold() < 0.99f); }

    // 地面状態
    bool IsGround() const { return isGround; }
    void SetGrounded(bool grounded) { isGround = grounded; }

    // ステータス
    float GetInvincibleTimer() const { return invincibleTimer; }
    float GetHeight() const { return height; }
    void  SetHeight(float h) { height = h; }
    int   GetHealth() const { return health; }
    void  SetHealth(int h) { health = h; }
    float GetWeight() const { return weight; }
    void  SetWeight(float w) { weight = w; }

    // モデル取得
    Model* GetModel() const { return model.get(); }
    DirectX::XMFLOAT3 GetLeftHandPosition() const;


    // ===== アニメーション関連 =====
    virtual int GetCurrentAnimationIndex() const { return animationIndex; } // 現在のアニメーションID
    virtual std::string GetCurrentAnimationName() const; // アニメーション名取得
    virtual std::string GetCurrentAttackAnimationName() const; // 攻撃アニメーション名取得
    virtual float GetAnimationTime() const { return animationSeconds; } // 再生時間
    virtual bool IsAnimationLooping() const { return animationLoop; } // ループしているか
    void StartAnimation(int animIndex, bool loop); // アニメーション開始
    float GetAnimationBlendSeconds() const { return animationBlendSeconds; }
    void SetAnimationBlendSeconds(float blendTime) { animationBlendSeconds = blendTime; }

    // ===== 戦闘関連 =====
    bool IsWeaponHitActive() const { return isWeaponHitActive; } // 当たり判定ONか

    void SetWeaponHitActive(bool active) {
        // 状態が変わったときだけ更新
        if (isWeaponHitActive != active) isWeaponHitActive = active;
    }

    int GetCurrentAttackIndex() const { return currentAttackIndex; } // 現在の攻撃番号
    void SetCurrentAttackIndex(int index) { currentAttackIndex = index; }

    // ヒット情報
    HitData GetLastHitData() const { return lastHitData; }
    void SetLastHitData(const HitData& data) { lastHitData = data; }

    // ===== モデルデータ取得 =====
    virtual const std::vector<Model::Animation>& GetAnimations() const {
        // モデルがあれば取得
        if (model) return model->GetAnimations();

        // 無い場合は空配列
        static std::vector<Model::Animation> empty;
        return empty;
    }

    virtual const std::vector<Model::Node>& GetNodes() const {
        if (model) return model->GetNodes();

        static std::vector<Model::Node> empty;
        return empty;
    }

    // ノード選択
    virtual Model::Node* GetSelectedNode() const { return selectionNode; }
    virtual void SetSelectedNode(Model::Node* node) { selectionNode = node; }

    // ===== 武器関連（派生クラスで実装） =====
    virtual bool HasWeapon() const { return false; } // 武器を持っているか
    virtual DirectX::XMFLOAT4X4 GetWeaponTransform() const = 0; // 武器の行列
    virtual DebugCapsuleParams& GetWeaponCapsuleParams() = 0;   // 武器カプセル
    virtual DebugSphereParams& GetWeaponSphereParams() = 0;     // 武器球

    // ===== 体のカプセル =====
    const DebugCapsuleParams& GetCapsuleParams() const { return capsuleParams; }
    DebugCapsuleParams& GetCapsuleParams() { return capsuleParams; }

    void SetCapsuleParams(const DebugCapsuleParams& params) { capsuleParams = params; }

    // カプセルの両端を取得
    void GetBodyCapsuleEndpoints(DirectX::XMFLOAT3& outBase, DirectX::XMFLOAT3& outTip) const;

    // スケール込み半径
    virtual float GetScaledBodyRadius() const { return capsuleParams.radius * scale.x; }

    // 共通ユーティリティ 
    template<typename StateID, typename Manager>
    void ClearStatesByIDs(const std::vector<StateID>& stateIDs, Manager& manager) {
        // 指定された状態を削除
        for (const auto& id : stateIDs) {
            manager.RemoveState(id);
        }
    }

protected:
    // モデル
    std::unique_ptr<Model> model;

    // アニメーション
    std::vector<Model::NodeCache> nodePoses;
    int animationIndex = -1;           // 現在のアニメーション
    bool animationLoop = false;        // ループするか
    float animationSeconds = 0;        // 再生時間
    float oldAnimationSeconds = 0;     // 前フレーム時間
    float animationBlendSeconds = 0.0f;// ブレンド時間
    Model::Node* selectionNode = nullptr; // 選択ノード

    // 変換情報
    DirectX::XMFLOAT3 position = { 0,0,0 }; // 位置
    DirectX::XMFLOAT3 angle = { 0,0,0 };    // 回転
    DirectX::XMFLOAT3 scale = { 1,1,1 };    // スケール
    DirectX::XMFLOAT3 velocity = { 0,0,0 }; // 速度
    DirectX::XMFLOAT4X4 transform = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 }; // 行列

    // 状態
    int health = 0;               // 体力
    bool isDead = false;          // 死亡状態
    bool isVisible = true;        // 表示状態
    bool isGround = false;        // 地面にいるか
    float height = 0.0f;          // 高さ
    float weight = 0.0f;          // 重さ
    float invincibleTimer = 0.0f; // 無敵時間
    float gravity = CharacterConstants::DEFAULT_GRAVITY;   // 重力

    // 戦闘
    float leftHandOffset[3] = { 0.0f, 0.0f, 0.0f };
    bool isWeaponHitActive = false; // 当たり判定
    int currentAttackIndex = 0;     // 攻撃番号


    // 内部メンバ 
    DebugCapsuleParams capsuleParams; // 体カプセル
    HitData lastHitData = {};         // 最後のヒット情報    

    // 内部処理 
    virtual const char* GetLeftHandBoneName() const = 0;
    // 武器オブジェクト取得（派生クラスで実装）
    virtual WeaponObject& GetWeapon() = 0;
    virtual const WeaponObject& GetWeapon() const = 0;

    // 武器を手に追従させる
    virtual void UpdateWeaponAttachment();
    virtual void RootMotion(float elapsedTime) {}  // アニメーション移動
    void AdvanceAnimationTime(float elapsedTime); // 時間更新
};