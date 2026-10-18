#pragma once
#include <wrl/client.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include "GameLib/RenderContext.h"

// パーティクルシステム管理クラス
class particleSystem
{
public:
    // コンストラクタ 初期化
    particleSystem(ID3D11Device* device,
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView,
        int komax, int komay, int num);

    // デストラクタ メモリ解放
    ~particleSystem();

    // 更新処理
    void update(float elapsedTime);

    // 描画処理
    void render(const RenderContext& rc);

    // パーティクル生成
    void set(int type, float timer,
        DirectX::XMFLOAT3 pos,
        DirectX::XMFLOAT3 vel,
        DirectX::XMFLOAT3 acc,
        DirectX::XMFLOAT2 size,
        bool anime, float animeSpeed,
        float rotation = 0.0f,
        float rotationSpeed = 0.0f);

private:
    // シェーダ用定数バッファ
    struct constants
    {
        DirectX::XMFLOAT4X4 view;        // ビュー行列
        DirectX::XMFLOAT4X4 projection;  // 射影行列
    };

    // 頂点データ構造
    struct vertex
    {
        DirectX::XMFLOAT3 position;  // 位置
        DirectX::XMFLOAT2 texcoord;  // サイズ情報
        DirectX::XMFLOAT4 color;     // 色とアルファ
        DirectX::XMFLOAT4 param;     // フレームや分割情報
    };

    // パーティクルデータ
    struct particleData
    {
        float x, y, z;        // 位置
        float vx, vy, vz;     // 速度
        float ax, ay, az;     // 加速度
        float w, h;           // サイズ
        float type;           // フレームまたは状態
        float timer;          // 残り寿命
        float alpha;          // 透明度
        bool  anime;          // アニメーション有無
        float animeSpeed;     // アニメーション速度
        float rotation;       // 回転角度
        float rotationSpeed;  // 回転速度
    };

private:
    int numParticles;  // 最大パーティクル数
    int komax;         // 横コマ数
    int komay;         // 縦コマ数

    particleData* data = nullptr;   // パーティクル配列
    vertex* vertices = nullptr;     // 頂点配列

    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;    // 頂点バッファ
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;  // 定数バッファ

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;     // 頂点シェーダ
    Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometryShader; // ジオメトリシェーダ
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;       // ピクセルシェーダ

    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;               // 入力レイアウト
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView; // テクスチャ
};