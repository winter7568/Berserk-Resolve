#pragma once
#include "Shader.h"

class PBRShader : public Shader
{
public:
    PBRShader(ID3D11Device* device);
    ~PBRShader() override = default;

    // シェーダーの開始と描画と終了
    void Begin(const RenderContext& rc) override;
    void Draw(const RenderContext& rc, const Model* model) override;
    void End(const RenderContext& rc) override;

protected:
    // シーン全体のデータ
    struct CbScene
    {
        DirectX::XMFLOAT4X4 viewProjection;   // カメラの行列
        DirectX::XMFLOAT4 lightDirection;     // ライトの方向
        DirectX::XMFLOAT4 lightColor;         // ライトの色
        DirectX::XMFLOAT4 cameraPosition;     // カメラの位置
        DirectX::XMFLOAT4X4 lightViewProjection; // シャドウ用の行列
        DirectX::XMFLOAT4 shadowColor;        // 影の色
        float shadowTexelSize;                // 影のサイズ
        float padding[3];                     // メモリ調整
    };

    // マテリアルのデータ
    struct CbMaterial
    {
        DirectX::XMFLOAT4 albedo;             // ベースカラー
        DirectX::XMFLOAT4 emissiveColor;      // 発光色
        float metallic;                       // 金属感
        float roughness;                      // 粗さ
        float ao;                             // 環境光
        float materialEmissiveStrength;       // 発光の強さ

        float dissolveThreshold;              // ディゾルブの開始
        float dissolveEdgeWidth;              // エッジの幅
        float dissolveEdgeIntensity;          // エッジの強さ
        float hasDissolve;                    // ディゾルブの有無

        float materialEnvironmentStrength;    // 環境反射の強さ
        float padding[3];                     // メモリ調整
    };

    // 武器の発光データ
    struct CbWeaponGlow
    {
        DirectX::XMFLOAT4 uvScrollSpeed_glowIntensity; // テクスチャの動きと発光
        DirectX::XMFLOAT4 fresnelGlowPower_glowTime;   // フレネルと時間
    };

    // スケルトンのデータ
    struct CbSkeleton
    {
        DirectX::XMFLOAT4X4 boneTransforms[256]; // ボーンの変形
    };

    // バッファ
    Microsoft::WRL::ComPtr<ID3D11Buffer> sceneConstantBuffer;     // シーン用
    Microsoft::WRL::ComPtr<ID3D11Buffer> materialConstantBuffer;  // マテリアル用
    Microsoft::WRL::ComPtr<ID3D11Buffer> skeletonConstantBuffer;  // スケルトン用
    Microsoft::WRL::ComPtr<ID3D11Buffer> weaponGlowConstantBuffer;// 発光用

    // シェーダー
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout; // 頂点の形式

private:
    // 武器の発光データを計算する
    void SetWeaponGlowData(CbWeaponGlow& cbWeaponGlow, const Model* model);

    // シェーダースロット
    static constexpr UINT CB_SLOT = 0;
    static constexpr UINT SAMPLER_SLOT = 0;
    static constexpr UINT TEX_SLOT = 0;
    static constexpr UINT DISSOLVE_SLOT = 8;

    // マテリアルデフォルト
    static constexpr float DEFAULT_METALLIC = 1.0f;
    static constexpr float DEFAULT_ROUGHNESS = 0.3f;

    // 武器発光
    static constexpr float GLOW_SPEED = 1.5f;
    static constexpr float GLOW_INTENSITY = 2.0f;
    static constexpr float FRESNEL_POWER = 3.0f;
};