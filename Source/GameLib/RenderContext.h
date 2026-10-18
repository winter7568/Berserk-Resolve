#pragma once
#include "CameraFiles/Camera.h"
#include "RenderState.h"
#include "ShaderSystems/Light.h"
#include "ShaderSystems/ShadowMap.h"

struct RenderContext
{
	ID3D11DeviceContext*  deviceContext;
	const RenderState*    renderState;
	const Camera*         camera;
	const LightManager*   lightManager;
	DirectX::XMFLOAT4X4	  view;
	DirectX::XMFLOAT4X4   projection;
    ShadowMap*            shadowMap;
	DirectX::XMFLOAT3     shadowColor;
};