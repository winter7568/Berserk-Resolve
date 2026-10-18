#include "GameLib/Graphics.h"
#include "EffectManager.h"
#include "Effect.h"  

void EffectManager::Initialize()
{
    // スレッド安全のためロック
    std::lock_guard<std::mutex> lock(Graphics::Instance().GetMutex());

    Graphics& graphics = Graphics::Instance();

    // Effekseerレンダラ作成
    effekseerRenderer = EffekseerRendererDX11::Renderer::Create(
        graphics.GetDevice(),
        graphics.GetDeviceContext(),
        4096
    );

    // Effekseerマネージャ作成
    effekseerManager = Effekseer::Manager::Create(2048);

    // 各種レンダラ設定
    effekseerManager->SetSpriteRenderer(effekseerRenderer->CreateSpriteRenderer());
    effekseerManager->SetRibbonRenderer(effekseerRenderer->CreateRibbonRenderer());
    effekseerManager->SetRingRenderer(effekseerRenderer->CreateRingRenderer());
    effekseerManager->SetTrackRenderer(effekseerRenderer->CreateTrackRenderer());
    effekseerManager->SetModelRenderer(effekseerRenderer->CreateModelRenderer());

    // ローダ設定
    effekseerManager->SetTextureLoader(effekseerRenderer->CreateTextureLoader());
    effekseerManager->SetModelLoader(effekseerRenderer->CreateModelLoader());
    effekseerManager->SetMaterialLoader(effekseerRenderer->CreateMaterialLoader());

    // 座標系設定
    effekseerManager->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);
}

void EffectManager::Finalize()
{
    // キャッシュクリア
    effectCache.clear();

    // アクティブエフェクトクリア
    activeEffects.clear();
}

void EffectManager::Update(float elapsedTime)
{
    // Effekseer更新
    effekseerManager->Update(elapsedTime * 60.0f);

    // アクティブエフェクト更新と削除
    for (size_t i = 0; i < activeEffects.size(); )
    {
        activeEffects[i].lifetime += elapsedTime;

        // 一定時間経過で削除
        if (activeEffects[i].lifetime > 30.0f)
        {
            activeEffects.erase(activeEffects.begin() + i);
        }
        else
        {
            ++i;
        }
    }
}

void EffectManager::Render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection)
{
    // カメラ行列設定
    effekseerRenderer->SetCameraMatrix(
        *reinterpret_cast<const Effekseer::Matrix44*>(&view)
    );
    effekseerRenderer->SetProjectionMatrix(
        *reinterpret_cast<const Effekseer::Matrix44*>(&projection)
    );

    // 描画開始
    effekseerRenderer->BeginRendering();

    // エフェクト描画
    effekseerManager->Draw();

    // 描画終了
    effekseerRenderer->EndRendering();
}

// エフェクト取得または読み込み
std::shared_ptr<Effect> EffectManager::GetOrLoadEffect(const std::string& effectName)
{
    std::string fullPath = "Data/Effect/" + effectName + ".efk";

    auto it = effectCache.find(fullPath);
    if (it != effectCache.end())
    {
        // キャッシュから取得
        return it->second;
    }

    // 新規読み込み
    try
    {
        std::shared_ptr<Effect> effect = std::make_shared<Effect>(fullPath.c_str());
        effectCache[fullPath] = effect;
        return effect;
    }
    catch (...)
    {
        return nullptr;
    }
}

void EffectManager::PlayEffect(const std::string& effectName,
    const DirectX::XMFLOAT3& position,
    float scale)
{
    // エフェクト取得
    std::shared_ptr<Effect> effect = GetOrLoadEffect(effectName);
    if (!effect) return;

    // 再生
    Effekseer::Handle handle = effect->Play(position, scale);

    // 有効なら登録
    if (handle >= 0)
    {
        activeEffects.push_back({ effectName, handle, 0.0f });
    }
}

Effekseer::Handle EffectManager::PlayEffectWithHandle(const std::string& effectName,
    const DirectX::XMFLOAT3& position,
    float scale)
{
    // エフェクト取得
    std::shared_ptr<Effect> effect = GetOrLoadEffect(effectName);
    if (!effect) return -1;

    // 再生
    Effekseer::Handle handle = effect->Play(position, scale);

    // 有効なら登録
    if (handle >= 0)
    {
        activeEffects.push_back({ effectName, handle, 0.0f });
    }

    return handle;
}

void EffectManager::StopEffect(Effekseer::Handle handle)
{
    // ハンドル検索
    for (auto it = activeEffects.begin(); it != activeEffects.end(); ++it)
    {
        if (it->handle == handle)
        {
            // エフェクト停止
            std::shared_ptr<Effect> effect = GetOrLoadEffect(it->effectName);
            if (effect)
            {
                effect->Stop(handle);
            }

            // リストから削除
            activeEffects.erase(it);
            break;
        }
    }
}

void EffectManager::StopEffect(const std::string& effectName)
{
    for (auto it = activeEffects.begin(); it != activeEffects.end(); )
    {
        if (it->effectName == effectName)
        {
            // エフェクト停止
            auto effect = GetOrLoadEffect(effectName);
            if (effect)
            {
                effect->Stop(it->handle);
            }

            it = activeEffects.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void EffectManager::SetEffectPosition(Effekseer::Handle handle, const DirectX::XMFLOAT3& position)
{
    // 無効ハンドルチェック
    if (handle < 0) return;

    // 対象検索
    for (auto& active : activeEffects)
    {
        if (active.handle == handle)
        {
            auto effect = GetOrLoadEffect(active.effectName);
            if (effect)
            {
                // 位置更新
                effect->SetPosition(handle, position);
            }
            break;
        }
    }
}

void EffectManager::CleanupUnusedEffects()
{
    // 未使用エフェクト削除
    for (auto it = effectCache.begin(); it != effectCache.end(); )
    {
        bool isUsed = false;

        // 使用中か確認
        for (const auto& active : activeEffects)
        {
            std::string fullPath = "Data/Effect/" + active.effectName + ".efk";
            if (fullPath == it->first)
            {
                isUsed = true;
                break;
            }
        }

        // 未使用かつ参照がキャッシュのみ
        if (!isUsed && it->second.use_count() == 1)
        {
            it = effectCache.erase(it);
        }
        else
        {
            ++it;
        }
    }
}