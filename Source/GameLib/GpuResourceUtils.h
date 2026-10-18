#pragma once
#include <d3d11.h>

// GPUリソースユーティリティ
class GpuResourceUtils
{
public:
	// 頂点シェーダー読み込み
	static HRESULT LoadVertexShader(
		ID3D11Device* device,
		const char* filename,
		const D3D11_INPUT_ELEMENT_DESC inputElementDesc[],
		UINT inputElementCount,
		ID3D11InputLayout** inputLayout,
		ID3D11VertexShader** vertexShader);  // ← ここ修正
	//ピクセルシェーダの読み込み
	static HRESULT LoadPixelShader(
		ID3D11Device* device,
		const char* filename,
		ID3D11PixelShader** pixelShader
	);
	// ジオメトリシェーダーの読み込み
	static HRESULT LoadGeometryShader(
		ID3D11Device* device,
		const char* filename,
		ID3D11GeometryShader** geometryShader);
	// テクスチャ読み込み
	static HRESULT LoadTexture(
		ID3D11Device* device,
		const char* filename,
		ID3D11ShaderResourceView** shaderResourceView,
		D3D11_TEXTURE2D_DESC* texture2dDesc = nullptr);
	//
	static
		HRESULT LoadCubeMap(
			ID3D11Device* device,
			const wchar_t* filename,
			ID3D11ShaderResourceView** outSRV);
	//ダミーテキスチャ生成
	static HRESULT CreateDummyTexture(
		ID3D11Device* device,
		UINT color,
		ID3D11ShaderResourceView** shaderResourceView,
		D3D11_TEXTURE2D_DESC* texture2dDesc = nullptr);

	//定数バッファ作成
	static HRESULT CreateConstantBuffer(
		ID3D11Device* device,
		UINT bufferSize,
		ID3D11Buffer** constantBuffer);

	static HRESULT CreateTextureCopy(
		ID3D11Device* device,
		ID3D11DeviceContext* dc,
		ID3D11ShaderResourceView* sourceSRV,
		ID3D11ShaderResourceView** destSRV);

};

