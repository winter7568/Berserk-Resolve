#pragma once
#include <deque>
#include <memory>
#include <string>
#include <wrl/client.h>
#include <d3d11.h>
#include <DirectXMath.h>

// トレイルの頂点
struct TrailVertex {
    DirectX::XMFLOAT3 position; // 位置
    DirectX::XMFLOAT2 texcoord; // テクスチャ座標

    TrailVertex() : position(0, 0, 0), texcoord(0, 0) {}
    TrailVertex(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT2& uv)
        : position(pos), texcoord(uv) {
    }
};

class TrailSystem
{
public:
    // トレイルの1つの区間
    struct TrailSegment {
        DirectX::XMFLOAT3 startPos;   // 開始位置
        DirectX::XMFLOAT3 endPos;     // 終了位置
        float creationTime;           // 作成時間
    };

    // 設定データ
    struct TrailConfig {
        float maxLifetime = 0.5f;              // 表示時間
        int maxSegments = 40;                  // 最大数
        DirectX::XMFLOAT3 startOffset = { 0.0f, 0.0f, 0.0f }; // 開始オフセット
        DirectX::XMFLOAT3 endOffset = { 0.0f, 0.0f, 0.0f };   // 終了オフセット
        std::string normalMapPath = "";        // ノーマルマップのパス
        std::string maskPath = "";             // マスクのパス
    };

    TrailSystem();
    ~TrailSystem();

    // 初期化
    bool Initialize(ID3D11Device* device, const TrailConfig& config = TrailConfig());

    // 更新
    void Update(float elapsedTime, const DirectX::XMFLOAT4X4& weaponTransform);

    // 終了処理
    void Cleanup();

    // 有効と無効の切り替え
    void SetActive(bool active) { isActive = active; }

    // 有効かどうか
    bool IsActive() const { return isActive; }

    // 描画用データ取得
    ID3D11Buffer* GetVertexBuffer() const { return vertexBuffer.Get(); }
    ID3D11Buffer* GetIndexBuffer() const { return indexBuffer.Get(); }
    ID3D11ShaderResourceView* GetNormalMap() const { return normalMapSRV.Get(); }
    ID3D11ShaderResourceView* GetMask() const { return maskSRV.Get(); }
    UINT GetVertexCount() const { return vertexCount; }
    UINT GetIndexCount() const { return indexCount; }

    // オフセット設定
    void SetOffsets(const DirectX::XMFLOAT3& startOffset, const DirectX::XMFLOAT3& endOffset) {
        config.startOffset = startOffset;
        config.endOffset = endOffset;
    }

    // 設定取得
    TrailConfig& GetConfig() { return config; }
    const TrailConfig& GetConfig() const { return config; }

    // テクスチャ再読み込み
    bool ReloadTextures(ID3D11Device* device);

private:
    // バッファ作成
    bool CreateBuffers(ID3D11Device* device);

    // バッファ更新
    void UpdateBuffers(ID3D11DeviceContext* dc);

    // テクスチャ読み込み
    bool LoadTextures(ID3D11Device* device);

    TrailConfig config;
    std::deque<TrailSegment> segments; // トレイルのデータ

    bool isActive = false;       // 有効かどうか
    bool isInitialized = false;  // 初期化済みか

    // GPUリソース
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalMapSRV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> maskSRV;

    UINT vertexCount = 0; // 頂点数
    UINT indexCount = 0;  // インデックス数

    // 定数
    static constexpr UINT MAP_SUBRESOURCE_INDEX = 0;
    static constexpr int MAX_VERTICES = 100;       // 最大頂点数
    static constexpr int MAX_INDICES = 300;        // 最大インデックス数
    static constexpr int VERTICES_PER_SEGMENT = 2; // 1セグメントの頂点数
};