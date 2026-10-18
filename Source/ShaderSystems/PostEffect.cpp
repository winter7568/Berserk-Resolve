#include <imgui.h>

#include "PostEffect.h" 
#include "GameLib/GpuResourceUtils.h" 

PostEffect::PostEffect(ID3D11Device* device)
{
	// フルクリーンクアッド頂点シェーダー読み込み 
	GpuResourceUtils::LoadVertexShader(
		device,
		"Data/Shader/FullScreenQuadVS.cso",
		nullptr, 0,
		nullptr,
		fullscreenQuadVS.GetAddressOf());

	// 輝度抽出ピクセルシェーダー読み込み 
	GpuResourceUtils::LoadPixelShader(
		device,
		"Data/Shader/LuminanceExtractionPS.cso",
		luminanceExtractionPS.GetAddressOf());

	// 定数バッファ作成 
	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbPostEffect),
		constantBuffer.GetAddressOf());

	// ブルームピクセルシェーダー読み込み 
	GpuResourceUtils::LoadPixelShader(
		device,
		"Data/Shader/BloomPS.cso",
		bloomPS.GetAddressOf());

	// Vignetteピクセルシェーダー読み込み
	GpuResourceUtils::LoadPixelShader(
		device,
		"Data/Shader/VignettePS.cso",  
		vignettePS.GetAddressOf());
}

// 開始処理 
void PostEffect::Begin(const RenderContext& rc)
{
	ID3D11DeviceContext* dc = rc.deviceContext;
	const RenderState* renderState = rc.renderState;

	// ブレンドステート設定 
	FLOAT blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), blendFactor, 0xFFFFFFFF);

	// 深度ステンシルステート設定 
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::NoTestNoWrite), 0);

	// ラスタライザーステート設定 
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	// 頂点バッファ設定（使用しない） 
	dc->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	dc->IASetInputLayout(nullptr);

	// サンプラステート設定 
	ID3D11SamplerState* samplers[] =
	{
	 renderState->GetSamplerState(SamplerState::LinearWrap)
	};
	dc->PSSetSamplers(SAMPLER_SLOT, _countof(samplers), samplers);

	// 定数バッファ設定 
	dc->PSSetConstantBuffers(CONSTANT_BUFFER_SLOT, 1, constantBuffer.GetAddressOf());

	// 定数バッファ更新 
	dc->UpdateSubresource(constantBuffer.Get(), 0, 0, &cbPostEffect, 0, 0);
}

// 輝度抽出処理 
void PostEffect::LuminanceExtraction(const RenderContext& rc, ID3D11ShaderResourceView* colorMap) 
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	// シェーダー設定 
	dc->VSSetShader(fullscreenQuadVS.Get(), 0, 0);
	dc->PSSetShader(luminanceExtractionPS.Get(), 0, 0);

	// シェーダーリソース設定 
	ID3D11ShaderResourceView* srvs[] = { colorMap };
	dc->PSSetShaderResources(TEXTURE_SLOT, _countof(srvs), srvs);

	// 描画 
	dc->Draw(FULLSCREEN_QUAD_VERTEX_COUNT, 0);
}

void PostEffect::Bloom(const RenderContext& rc, ID3D11ShaderResourceView* colorMap, ID3D11ShaderResourceView* luminanceMap)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	// シェーダー設定 
	dc->VSSetShader(fullscreenQuadVS.Get(), 0, 0);
	dc->PSSetShader(bloomPS.Get(), 0, 0);

	// シェーダーリソース設定 
	ID3D11ShaderResourceView* srvs[] = { colorMap, luminanceMap };
	dc->PSSetShaderResources(0, _countof(srvs), srvs);

	// 描画 
	dc->Draw(4, 0);
}

// 終了処理 
void PostEffect::End(const RenderContext& rc)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	// 設定されているシェーダーリソースを解除 
	ID3D11ShaderResourceView* srvs[] = { nullptr, nullptr };
	dc->PSSetShaderResources(0, _countof(srvs), srvs);
}

// Vignette処理
void PostEffect::ApplyVignette(const RenderContext& rc, ID3D11ShaderResourceView* colorMap,
	ID3D11ShaderResourceView* vignetteTexture)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	// 定数バッファ更新 
	dc->UpdateSubresource(constantBuffer.Get(), 0, 0, &cbPostEffect, 0, 0);

	// シェーダー設定 
	dc->VSSetShader(fullscreenQuadVS.Get(), 0, 0);
	dc->PSSetShader(vignettePS.Get(), 0, 0);

	// シェーダーリソース設定 
	// Use the provided texture or the stored one
	ID3D11ShaderResourceView* textureToUse = vignetteTexture ? vignetteTexture : vignetteTextureSRV;

	if (!textureToUse)
	{
		// If no texture is provided, just draw without vignette
		ID3D11ShaderResourceView* srvs[] = { colorMap };
		dc->PSSetShaderResources(0, _countof(srvs), srvs);
	}
	else
	{
		ID3D11ShaderResourceView* srvs[] = { colorMap, textureToUse };
		dc->PSSetShaderResources(0, _countof(srvs), srvs);
	}

	// 描画 
	dc->Draw(4, 0);
}

// デバッグGUI描画
void PostEffect::DrawDebugGUI()
{
	ImGui::DragFloat("LuminanceLowerEdge", &cbPostEffect.luminanceExtractionLowerEdge, 0.01f, 0, 1.0f);
	ImGui::DragFloat("LuminanceHigherEdge", &cbPostEffect.luminanceExtractionHigherEdge, 0.01f, 0, 1.0f);
	ImGui::DragFloat("GaussianSigma", &cbPostEffect.gaussianSigma, 0.01f, 0, 10.0f);
	ImGui::DragFloat("BloomIntensity", &cbPostEffect.bloomIntensity, 0.1f, 0, 100.0f);
	// Vignette controls
	ImGui::Separator();
	ImGui::Text("Vignette");
	ImGui::Checkbox("Enabled", reinterpret_cast<bool*>(&cbPostEffect.vignetteEnabled));
	ImGui::DragFloat("Intensity", &cbPostEffect.vignetteIntensity, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Scale", &cbPostEffect.vignetteScale, 0.01f, 0.1f, 5.0f);
}