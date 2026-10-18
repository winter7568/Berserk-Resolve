#pragma once
#include "Shader.h"

class DistortionShader : public Shader
{
public:
    DistortionShader(ID3D11Device* device);
    ~DistortionShader() override = default;

    // シェーダーの処理の開始と描画と終了
    void Begin(const RenderContext& rc) override;
    void Draw(const RenderContext& rc, const Model* model) override;
    void End(const RenderContext& rc) override;

    // トレイルを描画する関数
    void DrawTrail(const RenderContext& rc, ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer,
        UINT vertexCount, UINT indexCount, ID3D11ShaderResourceView* sceneSRV,
        ID3D11ShaderResourceView* normalMap, ID3D11ShaderResourceView* maskSRV);

    // トレイルの種類を設定する
    void SetPlayerTrail() { currentIsPlayer = 1.0f; }
    void SetEnemyTrail() { currentIsPlayer = 0.0f; }

private:
    // トレイルのパラメータを送るバッファ
    struct CbTrail
    {
        float distortionStrength = 0.0f;  // ゆがみの強さ
        float trailAlpha = 0.0f;           // トレイルの透明度
        float uvScale[2] = {}; // テクスチャの大きさ調整
        float time = 0.0f;                 // 時間で動きをつける
        float isPlayer = 0.0f;             // プレイヤーか敵かの判定
        float padding[2] = {};                  // メモリ調整用
    };

    // シェーダーのリソース
    Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer;    // 頂点シェーダー用の行列データ
    Microsoft::WRL::ComPtr<ID3D11Buffer> psConstantBuffer;    // ピクセルシェーダー用のトレイルデータ
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;    // 頂点のデータ形式

    // 描画の設定
    Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;           // 透明処理
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState; // ポリゴンの描画設定
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthState;    // 奥行きの判定

    // 実行中の値
    float currentTime = 0.0f;      // 時間のカウント
    float currentIsPlayer = 0.0f;  // トレイルの種類

    // 定数
    static constexpr int PS_CONSTANT_BUFFER_SLOT = 2;
    static constexpr int VS_CONSTANT_BUFFER_SLOT = 0;
    static constexpr float TIME_STEP = 0.016f;             // 約60フレーム用の時間
    static constexpr float TIME_RESET_THRESHOLD = 1000.0f; // 時間をリセットする上限
    static constexpr int TEXTURE_START_SLOT = 8;           // テクスチャの開始スロット
    static constexpr int TEXTURE_COUNT = 3;                // テクスチャの数
    // トレイル用の定数
    static constexpr float TRAIL_DISTORTION_STRENGTH = 1.0f;
    static constexpr float TRAIL_ALPHA = 1.0f;
    static constexpr float TRAIL_UV_SCALE_X = 6.0f;
    static constexpr float TRAIL_UV_SCALE_Y = 3.0f;

};