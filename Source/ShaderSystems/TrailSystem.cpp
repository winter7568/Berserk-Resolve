#include <algorithm>

#include "TrailSystem.h"
#include "GameLib/Graphics.h"
#include "GameLib/GpuResourceUtils.h"

TrailSystem::TrailSystem()
{
}

TrailSystem::~TrailSystem()
{
    Cleanup(); // 終了処理
}

bool TrailSystem::Initialize(ID3D11Device* device, const TrailConfig& config)
{
    this->config = config;

    // GPUバッファ作成
    if (!CreateBuffers(device)) {
        return false;
    }

    // テクスチャ読み込み（必須）
    if (!LoadTextures(device)) {
        return false;  // テクスチャ読み込み失敗 → 初期化失敗
    }

    isActive = true;        // 有効化
    isInitialized = true;   // 初期化完了
    return true;
}

bool TrailSystem::LoadTextures(ID3D11Device* device)
{
    bool success = true;

    // ノーマルマップ読み込み
    if (!config.normalMapPath.empty()) {
        HRESULT hr = GpuResourceUtils::LoadTexture(device, config.normalMapPath.c_str(),
            normalMapSRV.ReleaseAndGetAddressOf());
        if (FAILED(hr)) {
            success = false;
        }
    }

    // マスクテクスチャ読み込み
    if (!config.maskPath.empty()) {
        HRESULT hr = GpuResourceUtils::LoadTexture(device, config.maskPath.c_str(),
            maskSRV.ReleaseAndGetAddressOf());
        if (FAILED(hr)) {
            success = false;
        }
    }

    return success;
}

bool TrailSystem::ReloadTextures(ID3D11Device* device)
{
    if (!isInitialized) return false;

    // 古いテクスチャ解放
    normalMapSRV.Reset();
    maskSRV.Reset();

    // 再読み込み
    return LoadTextures(device);
}

void TrailSystem::Cleanup()
{
    segments.clear();          // データ削除
    vertexBuffer.Reset();      // バッファ解放
    indexBuffer.Reset();
    normalMapSRV.Reset();      // テクスチャ解放
    maskSRV.Reset();

    vertexCount = 0;
    indexCount = 0;
    isActive = false;
    isInitialized = false;
}

bool TrailSystem::CreateBuffers(ID3D11Device* device)
{
    // 頂点バッファ作成
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = sizeof(TrailVertex) * MAX_VERTICES;
    vbDesc.Usage = D3D11_USAGE_DYNAMIC;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = device->CreateBuffer(&vbDesc, nullptr, vertexBuffer.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    // インデックスバッファ作成
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.ByteWidth = sizeof(UINT) * MAX_INDICES;
    ibDesc.Usage = D3D11_USAGE_DYNAMIC;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = device->CreateBuffer(&ibDesc, nullptr, indexBuffer.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    return true;
}

void TrailSystem::Update(float elapsedTime, const DirectX::XMFLOAT4X4& weaponTransform)
{
    if (!isActive) return;

    // ワールド座標に変換
    DirectX::XMMATRIX weaponWorld = DirectX::XMLoadFloat4x4(&weaponTransform);

    DirectX::XMVECTOR startWorld = DirectX::XMVector3TransformCoord(
        DirectX::XMLoadFloat3(&config.startOffset), weaponWorld);
    DirectX::XMVECTOR endWorld = DirectX::XMVector3TransformCoord(
        DirectX::XMLoadFloat3(&config.endOffset), weaponWorld);

    DirectX::XMFLOAT3 startPos, endPos;
    DirectX::XMStoreFloat3(&startPos, startWorld);
    DirectX::XMStoreFloat3(&endPos, endWorld);

    // 新しいセグメント追加
    TrailSegment newSegment{};
    newSegment.startPos = startPos;
    newSegment.endPos = endPos;
    newSegment.creationTime = 0.0f;

    segments.push_back(newSegment);

    // 古いセグメント削除
    for (auto it = segments.begin(); it != segments.end(); ) {
        it->creationTime += elapsedTime;
        if (it->creationTime > config.maxLifetime) {
            it = segments.erase(it);
        }
        else {
            ++it;
        }
    }

    // 最大数制限
    while (segments.size() > config.maxSegments) {
        segments.pop_front();
    }

    // バッファ更新
    if (segments.size() >= VERTICES_PER_SEGMENT) {
        UpdateBuffers(Graphics::Instance().GetDeviceContext());
    }
}

void TrailSystem::UpdateBuffers(ID3D11DeviceContext* dc)
{
    if (segments.size() < VERTICES_PER_SEGMENT) {
        vertexCount = 0;
        indexCount = 0;
        return;
    }

    // 頂点生成
    std::vector<TrailVertex> vertices;
    std::vector<UINT> indices;

    const int numSegments = static_cast<int>(segments.size());

    // 各セグメントの頂点作成
    for (int i = 0; i < numSegments; ++i) {
        float t = static_cast<float>(i) / (numSegments - 1);
        const auto& segment = segments[i];

        vertices.emplace_back(segment.startPos, DirectX::XMFLOAT2(t, 0.0f));
        vertices.emplace_back(segment.endPos, DirectX::XMFLOAT2(t, 1.0f));
    }

    // インデックス作成
    for (int i = 0; i < numSegments - 1; ++i) {
        int base = i * VERTICES_PER_SEGMENT;
        indices.push_back(base);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base + 1);
        indices.push_back(base + 3);
        indices.push_back(base + 2);
    }

    // GPUバッファ更新
    D3D11_MAPPED_SUBRESOURCE mapped;

    if (SUCCEEDED(dc->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        memcpy(mapped.pData, vertices.data(), vertices.size() * sizeof(TrailVertex));
        dc->Unmap(vertexBuffer.Get(), MAP_SUBRESOURCE_INDEX);
        vertexCount = static_cast<UINT>(vertices.size());
    }

    if (SUCCEEDED(dc->Map(indexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        memcpy(mapped.pData, indices.data(), indices.size() * sizeof(UINT));
        dc->Unmap(indexBuffer.Get(), MAP_SUBRESOURCE_INDEX);
        indexCount = static_cast<UINT>(indices.size());
    }
}