#pragma once
#include <DirectXMath.h>
#include <Effekseer.h>
#include <EffekseerRendererDX11.h>
#include <unordered_map>
#include <memory>
#include <string>

// エフェクトクラスの前方宣言
class Effect;

// エフェクト管理クラス
class EffectManager
{
private:
    EffectManager() {}
    ~EffectManager() {}

    // 読み込み済みエフェクトのキャッシュ
    std::unordered_map<std::string, std::shared_ptr<Effect>> effectCache;

    // 再生中エフェクト情報
    struct ActiveEffectInfo {
        std::string effectName;   // エフェクト名
        Effekseer::Handle handle; // ハンドル
        float lifetime;           // 自動削除用時間
    };
    std::vector<ActiveEffectInfo> activeEffects;

public:
    // シングルトン取得
    static EffectManager& Instance()
    {
        static EffectManager instance;
        return instance;
    }

    // 初期化
    void Initialize();

    // 終了処理
    void Finalize();

    // 更新処理
    void Update(float elapsedTime);

    // 描画処理
    void Render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection);

    // Effekseerマネージャ取得
    Effekseer::ManagerRef GetEffekseerManager() { return effekseerManager; }

    // エフェクト再生 単発
    void PlayEffect(const std::string& effectName,
        const DirectX::XMFLOAT3& position,
        float scale = 1.0f);

    // エフェクト再生 ハンドル取得
    Effekseer::Handle PlayEffectWithHandle(const std::string& effectName,
        const DirectX::XMFLOAT3& position,
        float scale = 1.0f);

    // エフェクト停止 ハンドル指定
    void StopEffect(Effekseer::Handle handle);

    // エフェクト停止 名前指定
    void StopEffect(const std::string& effectName);

    // エフェクト位置更新
    void SetEffectPosition(Effekseer::Handle handle, const DirectX::XMFLOAT3& position);

    // 未使用エフェクト削除
    void CleanupUnusedEffects();

    // キャッシュ数取得
    size_t GetCachedEffectCount() const { return effectCache.size(); }

    // 再生中エフェクト数取得
    size_t GetActiveEffectCount() const { return activeEffects.size(); }

private:
    Effekseer::ManagerRef              effekseerManager;   // Effekseerマネージャ
    EffekseerRenderer::RendererRef     effekseerRenderer;  // レンダラ

    // エフェクト取得または読み込み
    std::shared_ptr<Effect> GetOrLoadEffect(const std::string& effectName);
};