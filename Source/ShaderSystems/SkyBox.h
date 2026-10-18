#pragma once
#pragma once
#include <DirectXTex.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include "GameLib/RenderState.h"
#include "GameLib/RenderContext.h"

class Skybox
{
public:
    Skybox();
    void Render(const RenderContext& rc, const DirectX::XMMATRIX& viewProj);
    void Initialize(ID3D11Device* device);

private:
    struct SkyboxVertex
    {
        DirectX::XMFLOAT3 Position;
    };

    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> cubeTexture;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skyboxSRV;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

    // íËêî
    static constexpr UINT CONSTANT_BUFFER_SLOT = 0;
    static constexpr UINT TEXTURE_SLOT = 0;
    static constexpr UINT SAMPLER_SLOT = 0;
    static constexpr UINT CUBE_INDEX_COUNT = 36;
    static constexpr UINT CUBE_FACE_COUNT = 6;

    void CreateCubeMesh(ID3D11Device* device);
    void CreateCubeTexture(ID3D11Device* device);
    void CreateShaders(ID3D11Device* device);
};