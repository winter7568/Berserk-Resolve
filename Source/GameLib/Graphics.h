#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <memory>
#include <mutex>

#include "FrameBuffer.h"
#include "RenderState.h"
#include "ShaderSystems/Shader.h"
#include "ShaderSystems/ShadowMap.h"
#include "ShapeRenderer.h"

enum class ShaderId
{
	Phong,
	PBR,
	Distortion,

	EnumCount
};

enum class FrameBufferId
{
	Display,
	Scene,
	Luminance,

	EnumCount
};

// グラフィックス
class Graphics
{
private:
	Graphics() = default;
	~Graphics() = default;

public:
	static Graphics& Instance()
	{
		static Graphics instance;
		return instance;
	}

	void Initialize(HWND hWnd);
	void Present(UINT syncInterval);

	// ウインドウハンドル取得
	HWND GetWindowHandle() { return hWnd; }

	ID3D11Device* GetDevice() { return device.Get(); }
	ID3D11DeviceContext* GetDeviceContext() { return immediateContext.Get(); }

	float GetScreenWidth()const { return screenWidth; }
	float GetScreenHeight()const { return screenHeight; }

	//FrameBuffer* GetFrameBuffer() { return frameBuffer.get(); }
	// フレームバッファ取得
	FrameBuffer* GetFrameBuffer(FrameBufferId frameBufferId)
	{
		return frameBuffers[static_cast<int>(frameBufferId)].get();
	}

	RenderState* GetRenderState() { return renderState.get(); }

	// シェイプレンダラ取得
	ShapeRenderer* GetShapeRenderer() { return shapeRenderer.get(); }


	//シェーダー取得
	Shader* GetShader(ShaderId shaderId) { return shaders[static_cast<int>(shaderId)].get(); }

	// シャドウマップ取得 
	ShadowMap* GetShadowMap() { return shadowMap.get(); }

	std::mutex& GetMutex() { return mutex; }



private:
	HWND											hWnd = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Device>          device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>   immediateContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain>        swapchain;

	float screenWidth = 0.0f;
	float screenHeight = 0.0f;

	std::unique_ptr<FrameBuffer> frameBuffers[static_cast<int>(FrameBufferId::EnumCount)];
	std::unique_ptr<RenderState>                  renderState;
	std::unique_ptr<Shader>                       shaders[static_cast<int>(ShaderId::EnumCount)];
	std::unique_ptr<ShapeRenderer>				  shapeRenderer;
	std::unique_ptr<ShadowMap>                    shadowMap;
	std::mutex                                    mutex;

};