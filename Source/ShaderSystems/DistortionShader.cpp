#include "DistortionShader.h"
#include "GameLib/GpuResourceUtils.h"
#include "GameLib/Misc.h"
#include "GameObjects/Enemy/Enemy.h"

DistortionShader::DistortionShader(ID3D11Device* device)
{
    // トレイル用の頂点データの設定
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    // シェーダーの読み込み
    GpuResourceUtils::LoadVertexShader(
        device,
        "Data/Shader/SwordTrail_VS.cso",
        inputElementDesc,
        _countof(inputElementDesc),
        inputLayout.GetAddressOf(),
        vertexShader.GetAddressOf());

    GpuResourceUtils::LoadPixelShader(
        device,
        "Data/Shader/SwordTrail_PS_Distortion.cso",
        pixelShader.GetAddressOf());

    // 頂点シェーダー用のバッファ作成
    D3D11_BUFFER_DESC vsCBDesc = {};
    vsCBDesc.ByteWidth = sizeof(DirectX::XMFLOAT4X4);
    vsCBDesc.Usage = D3D11_USAGE_DYNAMIC;
    vsCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    vsCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    HRESULT hr = device->CreateBuffer(&vsCBDesc, nullptr, vsConstantBuffer.GetAddressOf());

    // ピクセルシェーダー用のバッファ作成
    D3D11_BUFFER_DESC psCBDesc = {};
    psCBDesc.ByteWidth = sizeof(CbTrail);
    psCBDesc.Usage = D3D11_USAGE_DYNAMIC;
    psCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    psCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = device->CreateBuffer(&psCBDesc, nullptr, psConstantBuffer.GetAddressOf());

    // ブレンドの設定
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    device->CreateBlendState(&blendDesc, blendState.GetAddressOf());

    // ポリゴンの描画設定
    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthClipEnable = TRUE;
    device->CreateRasterizerState(&rasterDesc, rasterizerState.GetAddressOf());

    // 奥行きの設定
    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = TRUE;
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    depthDesc.DepthFunc = D3D11_COMPARISON_LESS;
    device->CreateDepthStencilState(&depthDesc, depthState.GetAddressOf());
}

void DistortionShader::Begin(const RenderContext& rc)
{
    ID3D11DeviceContext* dc = rc.deviceContext;

    // シェーダーとレイアウトを設定
    dc->IASetInputLayout(inputLayout.Get());
    dc->VSSetShader(vertexShader.Get(), nullptr, 0);
    dc->PSSetShader(pixelShader.Get(), nullptr, 0);

    // 時間の更新
    currentTime += TIME_STEP;
    if (currentTime > TIME_RESET_THRESHOLD) currentTime = 0.0f;

    // 頂点シェーダー用データの更新
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(dc->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        DirectX::XMMATRIX view = DirectX::XMLoadFloat4x4(&rc.view);
        DirectX::XMMATRIX projection = DirectX::XMLoadFloat4x4(&rc.projection);
        DirectX::XMMATRIX viewProjection = view * projection;
        DirectX::XMStoreFloat4x4((DirectX::XMFLOAT4X4*)mapped.pData, viewProjection);
        dc->Unmap(vsConstantBuffer.Get(), 0);
    }

    // ピクセルシェーダー用データの更新
    CbTrail cbTrail;
    cbTrail.distortionStrength = TRAIL_DISTORTION_STRENGTH;    // ゆがみの強さ
    cbTrail.trailAlpha = TRAIL_ALPHA;            // 透明度
    cbTrail.uvScale[0] = TRAIL_UV_SCALE_X;            // テクスチャの繰り返し
    cbTrail.uvScale[1] = TRAIL_UV_SCALE_Y;
    cbTrail.time = currentTime;
    cbTrail.isPlayer = currentIsPlayer;

    if (SUCCEEDED(dc->Map(psConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        memcpy(mapped.pData, &cbTrail, sizeof(CbTrail));
        dc->Unmap(psConstantBuffer.Get(), 0);
    }

    // バッファをセット
    dc->VSSetConstantBuffers(VS_CONSTANT_BUFFER_SLOT, 1, vsConstantBuffer.GetAddressOf());
    dc->PSSetConstantBuffers(PS_CONSTANT_BUFFER_SLOT, 1, psConstantBuffer.GetAddressOf());

    // 描画状態を設定
    const float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    dc->OMSetBlendState(blendState.Get(), blendFactor, 0xFFFFFFFF);
    dc->OMSetDepthStencilState(depthState.Get(), 0);
    dc->RSSetState(rasterizerState.Get());
}

void DistortionShader::Draw(const RenderContext& rc, const Model* model)
{
    // このシェーダーはモデル全体ではなく、トレイル専用の描画を行う
    // 各オブジェクトが自身のトレイルデータを用意し、DrawTrail()を呼び出す
    // そのため、この関数は使用しない
}

void DistortionShader::DrawTrail(const RenderContext& rc, ID3D11Buffer* vertexBuffer, ID3D11Buffer* indexBuffer,
    UINT vertexCount, UINT indexCount, ID3D11ShaderResourceView* sceneSRV,
    ID3D11ShaderResourceView* normalMap, ID3D11ShaderResourceView* maskSRV)
{
    if (!vertexBuffer || vertexCount == 0) {
        return;
    }

    ID3D11DeviceContext* dc = rc.deviceContext;

    // 頂点バッファを設定
    UINT stride = sizeof(TrailVertex);
    UINT offset = 0;
    dc->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

    if (indexBuffer && indexCount > 0) {
        dc->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    }

    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // テクスチャを設定
    ID3D11ShaderResourceView* srvs[TEXTURE_COUNT] = { sceneSRV, normalMap, maskSRV };
    dc->PSSetShaderResources(TEXTURE_START_SLOT, TEXTURE_COUNT, srvs);

    // 描画
    if (indexBuffer && indexCount > 0) {
        dc->DrawIndexed(indexCount, 0, 0);
    }
    else {
        dc->Draw(vertexCount, 0);
    }
}

void DistortionShader::End(const RenderContext& rc)
{
    ID3D11DeviceContext* dc = rc.deviceContext;

    // テクスチャのクリア
    ID3D11ShaderResourceView* nullSRVs[3] = { nullptr, nullptr, nullptr };
    dc->PSSetShaderResources(TEXTURE_START_SLOT, TEXTURE_COUNT, nullSRVs);

    // 状態をリセット
    dc->VSSetShader(nullptr, nullptr, 0);
    dc->PSSetShader(nullptr, nullptr, 0);
    dc->IASetInputLayout(nullptr);
}