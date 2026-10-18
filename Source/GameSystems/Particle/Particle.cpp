#include "particle.h"
#include "GameLib/GpuResourceUtils.h"

particleSystem::particleSystem(ID3D11Device* device, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView, int komax, int komay, int num)
	: komax(komax)
	, komay(komay)
	, numParticles(num)
{
	HRESULT hr;

	// パーティクルデータ初期化
	data = new particleData[num];
	ZeroMemory(data, sizeof(particleData) * num);

	vertices = new vertex[num];
	ZeroMemory(vertices, sizeof(vertex) * num);

	// 未使用状態を設定
	for (int i = 0; i < numParticles; i++) {
		data[i].type = -1;
	}

	this->shaderResourceView = shaderResourceView;

	// 頂点バッファ作成
	D3D11_BUFFER_DESC vb_desc = {};
	vb_desc.Usage = D3D11_USAGE_DEFAULT;
	vb_desc.ByteWidth = sizeof(vertex) * numParticles;
	vb_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	hr = device->CreateBuffer(&vb_desc, nullptr, vertexBuffer.GetAddressOf());
	assert(SUCCEEDED(hr));

	// 定数バッファ作成
	D3D11_BUFFER_DESC cb_desc = {};
	cb_desc.Usage = D3D11_USAGE_DEFAULT;
	cb_desc.ByteWidth = sizeof(constants);
	cb_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	hr = device->CreateBuffer(&cb_desc, nullptr, constantBuffer.GetAddressOf());
	assert(SUCCEEDED(hr));

	// 入力レイアウト定義
	D3D11_INPUT_ELEMENT_DESC input_layout_desc[] =
	{
		{ "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT,   0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT,      0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",      0, DXGI_FORMAT_R32G32B32A32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "PARAMETER",  0, DXGI_FORMAT_R32G32B32A32_FLOAT,0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// 頂点シェーダ読み込み
	GpuResourceUtils::LoadVertexShader(
		device,
		"Data/Shader/ParticleSystemVS.cso",
		input_layout_desc,
		_countof(input_layout_desc),
		inputLayout.GetAddressOf(),
		vertexShader.GetAddressOf()
	);

	// ジオメトリシェーダ読み込み
	GpuResourceUtils::LoadGeometryShader(
		device,
		"Data/Shader/ParticleSystemGS.cso",
		geometryShader.GetAddressOf()
	);

	// ピクセルシェーダ読み込み
	GpuResourceUtils::LoadPixelShader(
		device,
		"Data/Shader/ParticleSystemPS.cso",
		pixelShader.GetAddressOf()
	);
}

particleSystem::~particleSystem()
{
	delete[] data;
	delete[] vertices;
}

void particleSystem::update(float elapsedTime)
{
	for (int i = 0; i < numParticles; ++i)
	{
		// 非アクティブはスキップ
		if (data[i].type < 0) continue;

		// 加速度を速度に反映
		data[i].vx += data[i].ax * elapsedTime;
		data[i].vy += data[i].ay * elapsedTime;
		data[i].vz += data[i].az * elapsedTime;

		// 位置更新
		data[i].x += data[i].vx * elapsedTime;
		data[i].y += data[i].vy * elapsedTime;
		data[i].z += data[i].vz * elapsedTime;

		// 残り時間減少
		data[i].timer -= elapsedTime;

		// フェードアウト
		data[i].alpha = sqrtf(max(0.0f, data[i].timer));

		// アニメーション更新
		if (data[i].anime)
		{
			data[i].type += elapsedTime * data[i].animeSpeed;
		}

		// 寿命終了で無効化
		if (data[i].timer <= 0.0f)
		{
			data[i].type = -1;
		}
	}
}

void particleSystem::render(const RenderContext& rc)
{
	// カメラ行列を定数バッファに設定
	constants cb;
	cb.view = Camera::Instance().GetView();
	cb.projection = Camera::Instance().GetProjection();

	rc.deviceContext->UpdateSubresource(constantBuffer.Get(), 0, nullptr, &cb, 0, 0);

	// トポロジと入力レイアウト設定
	rc.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	rc.deviceContext->IASetInputLayout(inputLayout.Get());

	// シェーダ設定
	rc.deviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);
	rc.deviceContext->GSSetShader(geometryShader.Get(), nullptr, 0);
	rc.deviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);

	// 定数バッファ設定
	ID3D11Buffer* constantBuffers[] = { constantBuffer.Get() };
	rc.deviceContext->VSSetConstantBuffers(0, 1, constantBuffers);
	rc.deviceContext->GSSetConstantBuffers(0, 1, constantBuffers);
	rc.deviceContext->PSSetConstantBuffers(0, 1, constantBuffers);

	// テクスチャ設定
	rc.deviceContext->PSSetShaderResources(0, 1, shaderResourceView.GetAddressOf());

	// パーティクルデータを頂点に変換
	int n = 0;
	for (int i = 0; i < numParticles; i++)
	{
		if (data[i].type < 0) continue;

		vertices[n].position.x = data[i].x;
		vertices[n].position.y = data[i].y;
		vertices[n].position.z = data[i].z;

		vertices[n].texcoord.x = data[i].w;
		vertices[n].texcoord.y = data[i].h;

		// カラーは白固定
		vertices[n].color.x =
			vertices[n].color.y =
			vertices[n].color.z = 1.0f;
		vertices[n].color.w = data[i].alpha;

		// パラメータ設定
		vertices[n].param.x = 0.0f;           // 未使用
		vertices[n].param.y = data[i].type;   // フレーム
		vertices[n].param.z = (float)komax;   // 横コマ数
		vertices[n].param.w = (float)komay;   // 縦コマ数

		++n;
	}

	// 頂点バッファ更新
	rc.deviceContext->UpdateSubresource(vertexBuffer.Get(), 0, nullptr, vertices, 0, 0);

	// 頂点バッファ設定
	UINT stride = sizeof(vertex);
	UINT offset = 0;
	rc.deviceContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);

	// アクティブ数カウント
	int active_count = 0;
	for (int i = 0; i < numParticles; ++i)
	{
		if (data[i].type >= 0) active_count++;
	}

	// 描画
	rc.deviceContext->Draw(active_count, 0);

	// シェーダ解除
	rc.deviceContext->VSSetShader(nullptr, nullptr, 0);
	rc.deviceContext->GSSetShader(nullptr, nullptr, 0);
	rc.deviceContext->PSSetShader(nullptr, nullptr, 0);
}

void particleSystem::set(int type, float timer, DirectX::XMFLOAT3 pos,
	DirectX::XMFLOAT3 vel, DirectX::XMFLOAT3 acc,
	DirectX::XMFLOAT2 size, bool anime,
	float animeSpeed, float rotation,
	float rotationSpeed)
{
	for (int i = 0; i < numParticles; ++i)
	{
		// 使用中スロットはスキップ
		if (data[i].type >= 0) continue;

		// 初期値設定
		data[i].type = static_cast<float>(type);
		data[i].x = pos.x;
		data[i].y = pos.y;
		data[i].z = pos.z;

		data[i].vx = vel.x;
		data[i].vy = vel.y;
		data[i].vz = vel.z;

		data[i].ax = acc.x;
		data[i].ay = acc.y;
		data[i].az = acc.z;

		data[i].w = size.x;
		data[i].h = size.y;

		data[i].alpha = 1.0f;
		data[i].timer = timer;

		data[i].anime = anime;
		data[i].animeSpeed = animeSpeed;

		data[i].rotation = rotation;
		data[i].rotationSpeed = rotationSpeed;

		break;
	}
}