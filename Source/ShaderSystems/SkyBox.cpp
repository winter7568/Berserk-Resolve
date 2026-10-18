#include <vector>
#include "string"
#include "SkyBox.h"
#include "GameLib/GpuResourceUtils.h"

void Skybox::Initialize(ID3D11Device* device)
{
    // 初期化
    CreateCubeMesh(device);
    CreateCubeTexture(device);
    CreateShaders(device);
}

Skybox::Skybox()
{

}

void Skybox::CreateCubeMesh(ID3D11Device* device)
{
    // 頂点とインデックスの作成
    std::vector<SkyboxVertex> vertices = {
        // 前
        { {-1.0f,  1.0f, -1.0f} }, { {1.0f,  1.0f, -1.0f} }, { {1.0f, -1.0f, -1.0f} }, { {-1.0f, -1.0f, -1.0f} },
        // 後ろ
        { {-1.0f,  1.0f, 1.0f} }, { {1.0f,  1.0f, 1.0f} }, { {1.0f, -1.0f, 1.0f} }, { {-1.0f, -1.0f, 1.0f} },
        // 上
        { {-1.0f, 1.0f, -1.0f} }, { {1.0f, 1.0f, -1.0f} }, { {1.0f, 1.0f, 1.0f} }, { {-1.0f, 1.0f, 1.0f} },
        // 下
        { {-1.0f, -1.0f, -1.0f} }, { {1.0f, -1.0f, -1.0f} }, { {1.0f, -1.0f, 1.0f} }, { {-1.0f, -1.0f, 1.0f} },
        // 左
        { {-1.0f,  1.0f, 1.0f} }, { {-1.0f,  1.0f, -1.0f} }, { {-1.0f, -1.0f, -1.0f} }, { {-1.0f, -1.0f, 1.0f} },
        // 右
        { {1.0f,  1.0f, -1.0f} }, { {1.0f,  1.0f, 1.0f} }, { {1.0f, -1.0f, 1.0f} }, { {1.0f, -1.0f, -1.0f} }
    };

    std::vector<WORD> indices = {
        // 前
        0, 1, 2, 0, 2, 3,
        // 後ろ
        4, 5, 6, 4, 6, 7,
        // 上
        8, 9, 10, 8, 10, 11,
        // 下
        12, 13, 14, 12, 14, 15,
        // 左
        16, 17, 18, 16, 18, 19,
        // 右
        20, 21, 22, 20, 22, 23
    };

    // 頂点バッファ作成
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.ByteWidth = static_cast<UINT>(sizeof(SkyboxVertex) * vertices.size());
    vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vbData = { vertices.data() };
    device->CreateBuffer(&vbDesc, &vbData, &vertexBuffer);

    // インデックスバッファ作成
    D3D11_BUFFER_DESC ibDesc = {};
    ibDesc.ByteWidth = static_cast<UINT>(sizeof(WORD) * indices.size());
    ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
    ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA ibData = { indices.data() };
    device->CreateBuffer(&ibDesc, &ibData, &indexBuffer);
}

void Skybox::CreateCubeTexture(ID3D11Device* device)
{
    // キューブマップの画像
    const std::vector<std::wstring> faces = {
        L"Data/Sprite/Skybox/px.png",
        L"Data/Sprite/Skybox/nx.png",
        L"Data/Sprite/Skybox/py.png",
        L"Data/Sprite/Skybox/ny.png",
        L"Data/Sprite/Skybox/pz.png",
        L"Data/Sprite/Skybox/nz.png"
    };

    std::vector<DirectX::ScratchImage> images;
    DirectX::TexMetadata metadata = {};

    // 画像を読み込む
    for (const auto& face : faces) {
        DirectX::ScratchImage image;
        HRESULT hr = DirectX::LoadFromWICFile(face.c_str(), DirectX::WIC_FLAGS_NONE, &metadata, image);
        if (FAILED(hr)) {
            MessageBoxW(nullptr, (L"Failed to load: " + face).c_str(), L"Error", MB_OK);
            return;
        }
        images.push_back(std::move(image));
    }

    // キューブテクスチャ作成
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = static_cast<UINT>(metadata.width);
    texDesc.Height = static_cast<UINT>(metadata.height);
    texDesc.MipLevels = 1;
    texDesc.ArraySize = CUBE_FACE_COUNT;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    // 各面のデータ
    std::vector<D3D11_SUBRESOURCE_DATA> subResources(CUBE_FACE_COUNT);
    for (int i = 0; i < CUBE_FACE_COUNT; ++i) {
        subResources[i].pSysMem = images[i].GetPixels();
        subResources[i].SysMemPitch = static_cast<UINT>(images[i].GetImage(0, 0, 0)->rowPitch);
        subResources[i].SysMemSlicePitch = 0;
    }

    // テクスチャ作成
    HRESULT hr = device->CreateTexture2D(&texDesc, subResources.data(), cubeTexture.GetAddressOf());
    if (FAILED(hr)) {
        MessageBoxW(nullptr, L"Failed to create cube texture", L"Error", MB_OK);
        return;
    }

    // SRV作成
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MipLevels = 1;

    hr = device->CreateShaderResourceView(cubeTexture.Get(), &srvDesc, skyboxSRV.GetAddressOf());
    if (FAILED(hr)) {
        MessageBoxW(nullptr, L"Failed to create SRV", L"Error", MB_OK);
    }
}

void Skybox::CreateShaders(ID3D11Device* device)
{
    // 頂点の形式
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    // シェーダーの読み込み
    GpuResourceUtils::LoadVertexShader(
        device,
        "Data/Shader/SkyBoxVS.cso",
        layout,
        ARRAYSIZE(layout),
        &inputLayout,
        &vertexShader
    );

    GpuResourceUtils::LoadPixelShader(
        device,
        "Data/Shader/SkyBoxPS.cso",
        &pixelShader
    );

    // 行列用バッファ作成
    D3D11_BUFFER_DESC cbDesc = {
        sizeof(DirectX::XMMATRIX),
        D3D11_USAGE_DYNAMIC,
        D3D11_BIND_CONSTANT_BUFFER,
        D3D11_CPU_ACCESS_WRITE
    };

    device->CreateBuffer(&cbDesc, nullptr, &constantBuffer);
}

void Skybox::Render(const RenderContext& rc, const DirectX::XMMATRIX& viewProj)
{
    ID3D11DeviceContext* context = rc.deviceContext;

    // 行列を更新
    D3D11_MAPPED_SUBRESOURCE mapped;
    context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

    DirectX::XMMATRIX transposedViewProj = DirectX::XMMatrixTranspose(viewProj);
    memcpy(mapped.pData, &transposedViewProj, sizeof(DirectX::XMMATRIX));

    context->Unmap(constantBuffer.Get(), 0);

    // バッファ設定
    UINT stride = sizeof(SkyboxVertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->IASetInputLayout(inputLayout.Get());

    // シェーダー設定
    context->VSSetShader(vertexShader.Get(), nullptr, 0);
    context->PSSetShader(pixelShader.Get(), nullptr, 0);
    context->VSSetConstantBuffers(CONSTANT_BUFFER_SLOT, 1, constantBuffer.GetAddressOf());

    // テクスチャ設定
    context->PSSetShaderResources(TEXTURE_SLOT, 1, skyboxSRV.GetAddressOf());

    ID3D11SamplerState* samplerStates[] = {
        rc.renderState->GetSamplerState(SamplerState::LinearClamp)
    };
    context->PSSetSamplers(SAMPLER_SLOT, _countof(samplerStates), samplerStates);

    // 描画状態
    context->OMSetDepthStencilState(rc.renderState->GetDepthStencilState(DepthState::TestOnly), 0);
    context->RSSetState(rc.renderState->GetRasterizerState(RasterizerState::SolidCullNone));
    context->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);

    // 描画
    context->DrawIndexed(CUBE_INDEX_COUNT, 0, 0);
}