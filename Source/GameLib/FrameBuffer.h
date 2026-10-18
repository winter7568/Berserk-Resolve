#pragma once
#include <d3d11.h>
#include <wrl.h>

class FrameBuffer
{
public:
	FrameBuffer(ID3D11Device* device, IDXGISwapChain* swapchain);

	FrameBuffer(ID3D11Device* device, UINT width, UINT height);

	void Clear(ID3D11DeviceContext* dc, float r, float g, float b, float a);

	void SetRenderTargets(ID3D11DeviceContext* dc);
	// カラーマップ取得 
	ID3D11ShaderResourceView* GetColorMap() const { return colorMap.Get(); }
	ID3D11DepthStencilView* GetDepthStencilView() const { return depthStencilView.Get(); }
	ID3D11RenderTargetView* GetRenderTargetView() const { return renderTargetView.Get(); }
	const D3D11_VIEWPORT& GetViewport() const { return viewport; }
private:
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>    renderTargetView;
	D3D11_VIEWPORT                                    viewport;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>    depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>  colorMap;
};