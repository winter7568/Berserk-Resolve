#include "PBRShader.h"
#include "GameLib/Misc.h"
#include "GameLib/GpuResourceUtils.h"
#include "GameObjects/Player/Player.h"
#include "GameObjects/Enemy/Enemy.h"

PBRShader::PBRShader(ID3D11Device* device)
{
    // 頂点データの形式
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
    {
        {"POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BONE_WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BONE_INDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT",      0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    // 頂点シェーダーの読み込み
    GpuResourceUtils::LoadVertexShader(
        device,
        "Data/Shader/PBR_VS.cso",
        inputElementDesc,
        _countof(inputElementDesc),
        inputLayout.GetAddressOf(),
        vertexShader.GetAddressOf());

    // ピクセルシェーダーの読み込み
    GpuResourceUtils::LoadPixelShader(
        device,
        "Data/Shader/PBR_PS.cso",
        pixelShader.GetAddressOf());

    // 定数バッファの作成
    GpuResourceUtils::CreateConstantBuffer(device, sizeof(CbScene), sceneConstantBuffer.GetAddressOf());
    GpuResourceUtils::CreateConstantBuffer(device, sizeof(CbMaterial), materialConstantBuffer.GetAddressOf());
    GpuResourceUtils::CreateConstantBuffer(device, sizeof(CbSkeleton), skeletonConstantBuffer.GetAddressOf());
    GpuResourceUtils::CreateConstantBuffer(device, sizeof(CbWeaponGlow), weaponGlowConstantBuffer.GetAddressOf());
}

void PBRShader::Begin(const RenderContext& rc)
{
    ID3D11DeviceContext* dc = rc.deviceContext;

    // シェーダーの設定
    dc->IASetInputLayout(inputLayout.Get());
    dc->VSSetShader(vertexShader.Get(), nullptr, 0);
    dc->PSSetShader(pixelShader.Get(), nullptr, 0);

    // バッファの設定
    ID3D11Buffer* constantBuffers[] = {
        sceneConstantBuffer.Get(),
        materialConstantBuffer.Get(),
        skeletonConstantBuffer.Get(),
        weaponGlowConstantBuffer.Get(),
    };
    dc->VSSetConstantBuffers(CB_SLOT, _countof(constantBuffers), constantBuffers);
    dc->PSSetConstantBuffers(CB_SLOT, _countof(constantBuffers), constantBuffers);

    // サンプラーの設定
    ID3D11SamplerState* samplerStates[] = {
        rc.renderState->GetSamplerState(SamplerState::LinearWrap),
        rc.shadowMap->GetSamplerState(),
        rc.renderState->GetSamplerState(SamplerState::LinearWrap)
    };
    dc->PSSetSamplers(SAMPLER_SLOT, _countof(samplerStates), samplerStates);

    // 描画の状態を設定
    const float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    dc->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Opaque), blendFactor, 0xFFFFFFFF);
    dc->OMSetDepthStencilState(rc.renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
    dc->RSSetState(rc.renderState->GetRasterizerState(RasterizerState::SolidCullBack));

    // シーンデータの更新
    CbScene cbScene{};
    DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&Camera::Instance().GetView());
    DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&Camera::Instance().GetProjection());
    DirectX::XMStoreFloat4x4(&cbScene.viewProjection, V * P);

    const DirectionalLight& directionalLight = rc.lightManager->GetDirectionalLight();
    cbScene.lightDirection = DirectX::XMFLOAT4(
        directionalLight.direction.x,
        directionalLight.direction.y,
        directionalLight.direction.z,
        0.0f);

    cbScene.lightColor = DirectX::XMFLOAT4(
        directionalLight.color.x,
        directionalLight.color.y,
        directionalLight.color.z,
        1.0f);

    cbScene.lightViewProjection = rc.shadowMap->GetLightViewProjection();

    cbScene.shadowColor.x = rc.shadowColor.x;
    cbScene.shadowColor.y = rc.shadowColor.y;
    cbScene.shadowColor.z = rc.shadowColor.z;

    cbScene.shadowTexelSize = rc.shadowMap->GetTexelSize();

    const DirectX::XMFLOAT3& eye = Camera::Instance().GetEye();
    cbScene.cameraPosition = DirectX::XMFLOAT4(eye.x, eye.y, eye.z, 1.0f);

    dc->UpdateSubresource(sceneConstantBuffer.Get(), 0, 0, &cbScene, 0, 0);
}

void PBRShader::Draw(const RenderContext& rc, const Model* model)
{
    ID3D11DeviceContext* dc = rc.deviceContext;
    CbMaterial cbMaterial{};
    CbWeaponGlow cbWeaponGlow{};
    CbSkeleton cbSkeleton{};

    for (const Model::Mesh& mesh : model->GetMeshes())
    {
        // 頂点バッファの設定
        UINT stride = sizeof(Model::Vertex);
        UINT offset = 0;
        dc->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);
        dc->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
        dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // マテリアルデータの設定
        cbMaterial.albedo = mesh.material->color;
        cbMaterial.emissiveColor = DirectX::XMFLOAT4(
            mesh.material->emissiveColor.x,
            mesh.material->emissiveColor.y,
            mesh.material->emissiveColor.z,
            1.0f
        );
        cbMaterial.metallic = DEFAULT_METALLIC;
        cbMaterial.roughness = DEFAULT_ROUGHNESS;
        cbMaterial.materialEmissiveStrength = mesh.material->emissiveStrength * model->emissiveStrengthMultiplier;

        cbMaterial.dissolveThreshold = mesh.material->dissolveThreshold;
        cbMaterial.dissolveEdgeWidth = mesh.material->dissolveEdgeWidth;
        cbMaterial.dissolveEdgeIntensity = mesh.material->dissolveEdgeIntensity;
        cbMaterial.hasDissolve = mesh.material->hasDissolve ? 1.0f : 0.0f;

        cbMaterial.materialEnvironmentStrength = model->environmentStrength;
        cbMaterial.padding[0] = 0.0f;
        cbMaterial.padding[1] = 0.0f;
        cbMaterial.padding[2] = 0.0f;

        dc->UpdateSubresource(materialConstantBuffer.Get(), 0, 0, &cbMaterial, 0, 0);

        // スケルトンデータの更新
        if (mesh.bones.size() > 0)
        {
            for (size_t i = 0; i < mesh.bones.size(); ++i)
            {
                const Model::Bone& bone = mesh.bones.at(i);
                DirectX::XMMATRIX WorldTransform = DirectX::XMLoadFloat4x4(&bone.node->worldTransform);
                DirectX::XMMATRIX OffsetTransform = DirectX::XMLoadFloat4x4(&bone.offsetTransform);
                DirectX::XMMATRIX BoneTransform = OffsetTransform * WorldTransform;
                DirectX::XMStoreFloat4x4(&cbSkeleton.boneTransforms[i], BoneTransform);
            }
        }
        else
        {
            cbSkeleton.boneTransforms[0] = mesh.node->worldTransform;
        }
        rc.deviceContext->UpdateSubresource(skeletonConstantBuffer.Get(), 0, 0, &cbSkeleton, 0, 0);

        // 武器の発光データ
        SetWeaponGlowData(cbWeaponGlow, model);

        dc->UpdateSubresource(weaponGlowConstantBuffer.Get(), 0, 0, &cbWeaponGlow, 0, 0);

        // テクスチャの設定
        // t0 から t6 を使用
        // t7 は環境マップ用に予約しているのでここでは使わない
        ID3D11ShaderResourceView* srvs[] = {
            mesh.material->diffuseMap.Get(),        // t0
            mesh.material->normalMap.Get(),         // t1
            mesh.material->EmissiveMap.Get(),       // t2
            mesh.material->OcclusionMap.Get(),      // t3
            mesh.material->MetalnessMap.Get(),      // t4
            mesh.material->RoughnessMap.Get(),      // t5
            rc.shadowMap->GetShaderResourceView(),  // t6
        };
        dc->PSSetShaderResources(TEX_SLOT, _countof(srvs), srvs);

        // ディゾルブ用のマスクは別で設定
        // t7 を使わずに t8 に入れる
        if (mesh.material->dissolveMask)
        {
            dc->PSSetShaderResources(DISSOLVE_SLOT, 1, mesh.material->dissolveMask.GetAddressOf());
        }

        // 描画
        dc->DrawIndexed(static_cast<UINT>(mesh.indices.size()), 0, 0);
    }
}

void PBRShader::End(const RenderContext& rc)
{
    ID3D11DeviceContext* dc = rc.deviceContext;

    // テクスチャをクリア
    ID3D11ShaderResourceView* srvs[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
    dc->PSSetShaderResources(TEX_SLOT, _countof(srvs), srvs);

    // ディゾルブ用のクリア
    ID3D11ShaderResourceView* nullDissolve = nullptr;
    dc->PSSetShaderResources(DISSOLVE_SLOT, 1, &nullDissolve);

    // シェーダーをリセット
    dc->VSSetShader(nullptr, nullptr, 0);
    dc->PSSetShader(nullptr, nullptr, 0);
    dc->IASetInputLayout(nullptr);
}

void PBRShader::SetWeaponGlowData(CbWeaponGlow& cbWeaponGlow, const Model* model)
{
    Model* playerModel = Player::Instance().GetWeaponModel();
    Enemy& enemy = Enemy::Instance();

    bool isPlayerWeapon = (model == playerModel);
    bool isEnemy = (model == enemy.GetModel());

    if (isPlayerWeapon) {
        // プレイヤーの武器は赤い発光
        cbWeaponGlow.uvScrollSpeed_glowIntensity = DirectX::XMFLOAT4(0.0f, GLOW_SPEED, GLOW_INTENSITY, 0.0f);
    }
    else if (isEnemy && enemy.IsbodyGlow()) {
        // 敵は青い発光
        cbWeaponGlow.uvScrollSpeed_glowIntensity = DirectX::XMFLOAT4(0.0f, GLOW_SPEED, GLOW_INTENSITY, 1.0f);
    }
    else {
        // 発光なし
        cbWeaponGlow.uvScrollSpeed_glowIntensity = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    cbWeaponGlow.fresnelGlowPower_glowTime = DirectX::XMFLOAT4(FRESNEL_POWER, static_cast<float>(GetTickCount64() / 1000.0f), 0.0f, 0.0f);
}