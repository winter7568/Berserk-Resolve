#pragma once

#include <wrl.h> 
#include <d3d11.h> 
#include "GameLib/RenderContext.h" 

class PostEffect
{
public:
	PostEffect(ID3D11Device* device);

	// 開始処理 
	void Begin(const RenderContext& rc);

	// 輝度抽出処理 
	//void LuminanceExtraction(const RenderContext& rc);
	void LuminanceExtraction(const RenderContext& rc, ID3D11ShaderResourceView* colorMap);

	// ブルーム処理 
	void Bloom(const RenderContext& rc, ID3D11ShaderResourceView* colorMap,
		                                          ID3D11ShaderResourceView* luminanceMap);
	// 終了処理 
	void End(const RenderContext& rc);

	void ApplyVignette(const RenderContext& rc, ID3D11ShaderResourceView* colorMap,
		ID3D11ShaderResourceView* vignetteTexture);

	// デバッグGUI描画 
	void DrawDebugGUI();

	// Vignette control methods
	void SetVignetteIntensity(float intensity) { cbPostEffect.vignetteIntensity = intensity; }
	void SetVignetteScale(float scale) { cbPostEffect.vignetteScale = scale; }
	void SetVignetteEnabled(bool enabled) { cbPostEffect.vignetteEnabled = enabled ? 1.0f : 0.0f; }

	// Set vignette texture
	void SetVignetteTexture(ID3D11ShaderResourceView* texture) { vignetteTextureSRV = texture; }

	void SetScreenFadeAlpha(float alpha) { cbPostEffect.screenFadeAlpha = alpha; }

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader>  fullscreenQuadVS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>  luminanceExtractionPS;

	struct CbPostEffect
	{
		float luminanceExtractionLowerEdge = 0.3f;
		float luminanceExtractionHigherEdge = 0.6f;
		float gaussianSigma = 0.3f;
		float bloomIntensity = 1.0f;

		// Vignette parameters
		float vignetteIntensity = 0.0f;   // 0 = no effect, 1 = full texture effect
		float vignetteScale = 1.0f;       // Scale of vignette (1.0 = full screen, <1.0 = zoomed in)
		float vignetteEnabled = 0.0f;     // 0 = disabled, 1 = enabled
		float screenFadeAlpha = 0.0f;     // For padding to 16-byte alignment
	};

	CbPostEffect       cbPostEffect;
	Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>  bloomPS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> vignettePS;
	ID3D11ShaderResourceView* vignetteTextureSRV = nullptr;

	// 定数
	static constexpr UINT SAMPLER_SLOT = 0;
	static constexpr UINT CONSTANT_BUFFER_SLOT = 0;
	static constexpr UINT TEXTURE_SLOT = 0;
	static constexpr UINT FULLSCREEN_QUAD_VERTEX_COUNT = 4;
};