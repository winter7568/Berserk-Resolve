#include "GameLib/Misc.h"

#include "Graphics.h"
#include "ShaderSystems/PBRShader.h"
#include "ShaderSystems/DistortionShader.h"

void Graphics::Initialize(HWND hWnd)
{
	this->hWnd = hWnd;

	//Screen Size
	RECT rc;
	GetClientRect(hWnd, &rc);
	UINT screenWidth = rc.right - rc.left;
	UINT screenHeight = rc.bottom - rc.top;

	this->screenWidth = static_cast<float>(screenWidth);
	this->screenHeight = static_cast<float>(screenHeight);

	HRESULT hr = S_OK;

	//device and swapchain creation
	{
		UINT createDeviceFlags = 0;
#if defined(DEBUG)||defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1,
		};

		DXGI_SWAP_CHAIN_DESC swapchainDesc;
		{
			swapchainDesc.BufferDesc.Width = screenWidth;
			swapchainDesc.BufferDesc.Height = screenHeight;
			swapchainDesc.BufferDesc.RefreshRate.Numerator = 60;
			swapchainDesc.BufferDesc.RefreshRate.Denominator = 1;
			swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapchainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			swapchainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			swapchainDesc.SampleDesc.Count = 1;
			swapchainDesc.SampleDesc.Quality = 0;
			swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapchainDesc.BufferCount = 1;
			swapchainDesc.OutputWindow = hWnd;
			swapchainDesc.Windowed = true;
			swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapchainDesc.Flags = 0;
		}

		D3D_FEATURE_LEVEL featureLevel;

		hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			createDeviceFlags,
			featureLevels,
			ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION,
			&swapchainDesc,
			swapchain.GetAddressOf(),
			device.GetAddressOf(),
			&featureLevel,
			immediateContext.GetAddressOf()
		);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	//frameBuffer = std::make_unique<FrameBuffer>(device.Get(), swapchain.Get());
	// フレームバッファ作成
	frameBuffers[static_cast<int>(FrameBufferId::Display)] = std::make_unique<FrameBuffer>(device.Get(),
		                                                                                      swapchain.Get());
	frameBuffers[static_cast<int>(FrameBufferId::Scene)] = std::make_unique<FrameBuffer>(device.Get(),
		                                                                                      screenWidth, screenHeight);
	frameBuffers[static_cast<int>(FrameBufferId::Luminance)] = std::make_unique<FrameBuffer>(device.Get(),
		                                                                                      screenWidth, screenHeight);

	//レンダーステート生成
	renderState = std::make_unique<RenderState>(device.Get());

	// レンダラ生成
	shapeRenderer = std::make_unique<ShapeRenderer>(device.Get());

	//シェーダー生成
	shaders[static_cast<int>(ShaderId::PBR)] = std::make_unique<PBRShader>(device.Get());
	shaders[static_cast<int>(ShaderId::Distortion)] = std::make_unique<DistortionShader>(device.Get());

	// シャドウマップ生成 
	shadowMap = std::make_unique<ShadowMap>(device.Get());
}

void Graphics::Present(UINT syncInterval)
{
	swapchain->Present(syncInterval, 0);
}
