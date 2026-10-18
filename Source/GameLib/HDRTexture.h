#pragma once
#include <d3d11.h>
#include <stdio.h>
#include <wrl/client.h>

class HDRTexture
{
private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV;
	int width;
	int height;

public:
	HDRTexture();
	~HDRTexture();

	// シェーダーリソースビュー取得
	inline const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetShaderResourceView() const { return SRV; }

	bool Load(const char* filename);
	bool LoadMipmap(const char* filename);
	void Set(int slot = 0);

	int GetWidth() { return width; }
	int GetHeight() { return height; }

private:
	void HDR_CheckHeader(FILE* fp);
	bool HDR_ReadLine(BYTE* scanline, FILE* fp);
	bool HDR_ReadPixels(FILE* fp, float* buf);
};

