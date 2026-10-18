#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>

class Sprite
{
public:
	Sprite(ID3D11Device* device, const char* filename);
	Sprite(ID3D11Device* device);
	Sprite(const char* filename);

	struct Vertex
	{
		DirectX::XMFLOAT3 position; // 位置
		DirectX::XMFLOAT4 color;    // 色
		DirectX::XMFLOAT2 texcoord; // テクスチャ座標
	};

	// スプライトを画面に描画する
	void Render(ID3D11DeviceContext* dc,
		float dx, float dy,
		float dw, float dh,
		float r, float g, float b, float a
	) const;

	// 回転と奥行きを使って描画する
	void Render(ID3D11DeviceContext* dc,
		float dx, float dy,
		float dz,
		float dw, float dh,
		float angle,
		float r, float g, float b, float a
	) const;

	// テクスチャの一部を使って描画する
	void Render(ID3D11DeviceContext* dc,
		float dx, float dy,
		float dz,
		float dw, float dh,
		float sw, float sh,
		float sx, float sy,
		float angle,
		float r, float g, float b, float a
	) const;

	// テクスチャを取得
	ID3D11ShaderResourceView* GetShaderResourceView() const {
		return shaderResourceView.Get();
	}

	// テクスチャの横サイズ
	float GetTextureWidth() const { return textureWidth; }

	// テクスチャの縦サイズ
	float GetTextureHeight() const { return textureHeight; }


private:
	// シェーダー
	Microsoft::WRL::ComPtr<ID3D11VertexShader>    vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>     pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>     inputLayout;

	// バッファとテクスチャ
	Microsoft::WRL::ComPtr<ID3D11Buffer>          vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;

private:
	float textureWidth = 0;  // 横サイズ
	float textureHeight = 0; // 縦サイズ

	// 定数
	static constexpr UINT TEXTURE_SLOT = 0;
	static constexpr UINT SPRITE_VERTEX_COUNT = 4;
};