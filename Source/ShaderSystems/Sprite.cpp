#include <fstream>
#include "Sprite.h"
#include "GameLib/Misc.h"
#include "GameLib/GpuResourceUtils.h"
#include "GameLib/Graphics.h"


Sprite::Sprite(ID3D11Device* device,const char* filename)
{
	HRESULT hr = S_OK;

	//Creation of VertexBuffer
	{
		//Settings
		D3D11_BUFFER_DESC buffer_desc = {};
		buffer_desc.ByteWidth = sizeof(Vertex) * 4;
		buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;
		//vertex buffer object creation
		hr = device->CreateBuffer(&buffer_desc, nullptr, vertexBuffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}
	//vertex shader
	{
		//std::ifstream fin("Data/Shader/SpriteVS.cso", std::ios::in | std::ios::binary);

		////show file size
		//fin.seekg(0, std::ios_base::end);
		//long long size = fin.tellg();
		//fin.seekg(0, std::ios_base::beg);

		////reading
		//std::unique_ptr<char[]> data = std::make_unique<char[]>(size);
		//fin.read(data.get(), size);

		////vertex shader creation
		//HRESULT hr = device->CreateVertexShader(data.get(), size, nullptr, vertexShader.GetAddressOf());
		//_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		//Input Layout
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,
																  D3D11_INPUT_PER_VERTEX_DATA,0 },
			{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,
															      D3D11_INPUT_PER_VERTEX_DATA,0 },
			{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,
																  D3D11_INPUT_PER_VERTEX_DATA,0 },

		};
		hr = GpuResourceUtils::LoadVertexShader(
			device,
			"Data/Shader/SpriteVS.cso",
			inputElementDesc,
			ARRAYSIZE(inputElementDesc),
			inputLayout.GetAddressOf(),
			vertexShader.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		//hr = device->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc),
		//	data.get(), size, inputLayout.GetAddressOf());
		//_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	}

	//PixelShader
	{
		//std::ifstream fin("Data/Shader/SpritePS.cso", std::ios::in | std::ios::binary);

		//show file size
		//fin.seekg(0, std::ios_base::end);
		//long long size = fin.tellg();
		//fin.seekg(0, std::ios_base::beg);

		////reading
		//std::unique_ptr<char[]> data = std::make_unique<char[]>(size);
		//fin.read(data.get(), size);

		////vertex shader creation
		//HRESULT hr = device->CreatePixelShader(data.get(), size, nullptr, pixelShader.GetAddressOf());
		//_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
		hr = GpuResourceUtils::LoadPixelShader(
			device,
			"Data/Shader/SpritePS.cso",
			pixelShader.GetAddressOf()
		);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	}
	//texture creation
	if(filename != nullptr)
	{
		//texture reading
		D3D11_TEXTURE2D_DESC desc;
		hr = GpuResourceUtils::LoadTexture(device, filename, shaderResourceView.GetAddressOf(),&desc);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		textureWidth = static_cast<float>(desc.Width);
		textureHeight = static_cast<float>(desc.Height);
	}
	else
	{
		//texture reading
		D3D11_TEXTURE2D_DESC desc;
		hr = GpuResourceUtils::CreateDummyTexture(device, 0xFFFFFFFF, shaderResourceView.GetAddressOf(),
			                                                                                        &desc);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		textureWidth = static_cast<float>(desc.Width);
		textureHeight = static_cast<float>(desc.Height);

	}
}

Sprite::Sprite(ID3D11Device* device)
	: Sprite(device,nullptr)
{
}

Sprite::Sprite(const char* filename)
{
	// コンストラクタ
	ID3D11Device* device = Graphics::Instance().GetDevice();

	HRESULT hr = S_OK;

	// 頂点バッファの生成
	{
		// 頂点バッファを作成するための設定オプション
		D3D11_BUFFER_DESC buffer_desc = {};
		buffer_desc.ByteWidth = sizeof(Vertex) * 4;
		buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;
		// 頂点バッファオブジェクトの生成
		hr = device->CreateBuffer(&buffer_desc, nullptr, vertexBuffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	// 頂点シェーダー
	{
		// 入力レイアウト
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		hr = GpuResourceUtils::LoadVertexShader(
			device,
			"Data/Shader/SpriteVS.cso",
			inputElementDesc,
			ARRAYSIZE(inputElementDesc),
			inputLayout.GetAddressOf(),
			vertexShader.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	}

	// ピクセルシェーダー
	{
		hr = GpuResourceUtils::LoadPixelShader(
			device,
			"Data/Shader/SpritePS.cso",
			pixelShader.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	// テクスチャの生成	
	if (filename != nullptr)
	{
		// テクスチャファイル読み込み
		D3D11_TEXTURE2D_DESC desc;
		hr = GpuResourceUtils::LoadTexture(device, filename, shaderResourceView.GetAddressOf(), &desc);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		textureWidth = static_cast<float>(desc.Width);
		textureHeight = static_cast<float>(desc.Height);
	}
	else
	{
		// ダミーテクスチャ生成
		D3D11_TEXTURE2D_DESC desc;
		hr = GpuResourceUtils::CreateDummyTexture(device, 0xFFFFFFFF, shaderResourceView.GetAddressOf(),
			&desc);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		textureWidth = static_cast<float>(desc.Width);
		textureHeight = static_cast<float>(desc.Height);
	}

}

void Sprite::Render(ID3D11DeviceContext* dc, float dx, float dy, float dw, float dh, float r, float g, float b, float a) const
{
	//vertex coordinates
	DirectX::XMFLOAT2 positions[] = {
	DirectX::XMFLOAT2(dx,        dy),
	DirectX::XMFLOAT2(dx + dw   ,dy),
	DirectX::XMFLOAT2(dx,   dy + dh),
	DirectX::XMFLOAT2(dx +dw,dy +dh),
	};

	//get screen coordinates from viewport
	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	dc->RSGetViewports(&numViewports, &viewport);
	float screenWidth = viewport.Width;
	float screenHeight = viewport.Height;

	//change of screen coordiantes into NDC coordinates
	for (DirectX::XMFLOAT2& p : positions)
	{
		p.x = 2.0f * p.x / screenWidth - 1.0f;
		p.y = 1.0f - 2.0f * p.y / screenHeight;
	}

	//start vertex buffer values to map
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	HRESULT hr = dc->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));


	Vertex* v = static_cast<Vertex*>(mappedSubresource.pData);
	for (int i = 0; i < 4; ++i)
	{
		v[i].position.x = positions[i].x;
		v[i].position.y = positions[i].y;
		v[i].position.z = 0;

		v[i].color.x = r;
		v[i].color.y = g;
		v[i].color.z = b;
		v[i].color.w = a;
	}
	//end
	dc->Unmap(vertexBuffer.Get(), 0);

	//return values to GPU for rendering
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	dc->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	dc->IASetInputLayout(inputLayout.Get());
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	dc->VSSetShader(vertexShader.Get(), nullptr, 0);
	dc->PSSetShader(pixelShader.Get(), nullptr, 0);

	//render
	dc->Draw(4, 0);

}

//void Sprite::Render(ID3D11DeviceContext* dc, float dx, float dy, float dw, float dh, float angle, float r, float g, float b, float a) const
//{
//	//vertex coordinates
//	DirectX::XMFLOAT2 positions[] = {
//	DirectX::XMFLOAT2(dx,        dy),
//	DirectX::XMFLOAT2(dx + dw   ,dy),
//	DirectX::XMFLOAT2(dx,   dy + dh),
//	DirectX::XMFLOAT2(dx + dw,dy + dh),
//	};
//
//	//texture coordinates
//	DirectX::XMFLOAT2 texcoords[] = {
//		DirectX::XMFLOAT2(0.0f,0.0f),
//		DirectX::XMFLOAT2(1.0f,0.0f),
//		DirectX::XMFLOAT2(0.0f,1.0f),
//		DirectX::XMFLOAT2(1.0f,1.0f),
//
//	};
//
//	//スプライトの中心で回転させるために4頂点の中心位置が
//	//原点【０，０】になるように一回原点を移動させる
//	float mx = dx + dw * 0.5f;
//	float my = dy + dh * 0.5f;
//	for (auto& p : positions)
//	{
//		p.x -= mx;
//		p.y -= my;
//	}
//
//	//頂点を回転させる
//	float theta = DirectX::XMConvertToRadians(angle);
//	float c = cosf(theta);
//	float s = sinf(theta);
//	for (auto& p : positions)
//	{
//		DirectX::XMFLOAT2 r = p;
//		p.x = c * r.x + -s * r.y;
//		p.y = s * r.x + c * r.y;
//	}
//
//	//回転のために移動させた頂点をもとの位置に戻す
//	for (auto& p : positions)
//	{
//		p.x += mx;
//		p.y += my;
//	}
//
//	//get screen coordinates from viewport
//	D3D11_VIEWPORT viewport;
//	UINT numViewports = 1;
//	dc->RSGetViewports(&numViewports, &viewport);
//	float screenWidth = viewport.Width;
//	float screenHeight = viewport.Height;
//
//	//change of screen coordiantes into NDC coordinates
//	for (DirectX::XMFLOAT2& p : positions)
//	{
//		p.x = 2.0f * p.x / screenWidth - 1.0f;
//		p.y = 1.0f - 2.0f * p.y / screenHeight;
//	}
//
//	//start vertex buffer values to map
//	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
//	HRESULT hr = dc->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
//	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
//
//
//	Vertex* v = static_cast<Vertex*>(mappedSubresource.pData);
//	for (int i = 0; i < 4; ++i)
//	{
//		v[i].position.x = positions[i].x;
//		v[i].position.y = positions[i].y;
//		v[i].position.z = 0;
//
//		v[i].color.x = r;
//		v[i].color.y = g;
//		v[i].color.z = b;
//		v[i].color.w = a;
//
//		v[i].texcoord.x=texcoords[i].x;
//		v[i].texcoord.y = texcoords[i].y;
//	}
//	//end
//	dc->Unmap(vertexBuffer.Get(), 0);
//
//	//return values to GPU for rendering
//	UINT stride = sizeof(Vertex);
//	UINT offset = 0;
//	dc->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
//	dc->IASetInputLayout(inputLayout.Get());
//	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
//	dc->VSSetShader(vertexShader.Get(), nullptr, 0);
//	dc->PSSetShader(pixelShader.Get(), nullptr, 0);
//	dc->PSSetShaderResources(0, 1, shaderResourceView.GetAddressOf());
//	//render
//	dc->Draw(4, 0);
//
//
//}

void Sprite::Render(ID3D11DeviceContext* dc, float dx, float dy, float dz, float dw, float dh, float angle, float r, float g, float b, float a) const
{
	Render(dc, dx, dy, dz, dw, dh, 0, 0, textureWidth, textureHeight, angle, r, g, b, a);
}
void Sprite::Render(ID3D11DeviceContext* dc, float dx, float dy, float dz, float dw, float dh, float sx, float sy, float sw, float sh,
	float angle, float r, float g, float b, float a) const
{
	//vertex coordinates
	DirectX::XMFLOAT2 positions[] = {
	DirectX::XMFLOAT2(dx,        dy),
	DirectX::XMFLOAT2(dx + dw   ,dy),
	DirectX::XMFLOAT2(dx,   dy + dh),
	DirectX::XMFLOAT2(dx + dw,dy + dh),
	};

	//texture coordinates
	DirectX::XMFLOAT2 texcoords[] = {
		DirectX::XMFLOAT2(sx,  sy),
		DirectX::XMFLOAT2(sx + sw,sy),
		DirectX::XMFLOAT2(sx,  sy + sh),
		DirectX::XMFLOAT2(sx + sw,sy + sh),

	};

	//スプライトの中心で回転させるために4頂点の中心位置
	float mx = dx + dw * 0.5f;
	float my = dy + dh * 0.5f;
	for (auto& p : positions)
	{
		p.x -= mx;
		p.y -= my;
	}

	//頂点を回転させる
	float theta = DirectX::XMConvertToRadians(angle);
	float c = cosf(theta);
	float s = sinf(theta);
	for (auto& p : positions)
	{
		DirectX::XMFLOAT2 r = p;
		p.x = c * r.x + -s * r.y;
		p.y = s * r.x + c * r.y;
	}

	//回転のために移動させた頂点をもとの位置に戻す
	for (auto& p : positions)
	{
		p.x += mx;
		p.y += my;
	}

	//get screen coordinates from viewport
	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	dc->RSGetViewports(&numViewports, &viewport);
	float screenWidth = viewport.Width;
	float screenHeight = viewport.Height;

	//change of screen coordiantes into NDC coordinates
	for (DirectX::XMFLOAT2& p : positions)
	{
		p.x = 2.0f * p.x / screenWidth - 1.0f;
		p.y = 1.0f - 2.0f * p.y / screenHeight;
	}

	//start vertex buffer values to map
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	HRESULT hr = dc->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));


	Vertex* v = static_cast<Vertex*>(mappedSubresource.pData);
	for (int i = 0; i < 4; ++i)
	{
		v[i].position.x = positions[i].x;
		v[i].position.y = positions[i].y;
		v[i].position.z = dz;

		v[i].color.x = r;
		v[i].color.y = g;
		v[i].color.z = b;
		v[i].color.w = a;

		v[i].texcoord.x = texcoords[i].x / textureWidth;
		v[i].texcoord.y = texcoords[i].y / textureHeight;
	}
	//end
	dc->Unmap(vertexBuffer.Get(), 0);

	//return values to GPU for rendering
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	dc->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	dc->IASetInputLayout(inputLayout.Get());
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	dc->VSSetShader(vertexShader.Get(), nullptr, 0);
	dc->PSSetShader(pixelShader.Get(), nullptr, 0);
	dc->PSSetShaderResources(TEXTURE_SLOT, 1, shaderResourceView.GetAddressOf());
	//render
	dc->Draw(SPRITE_VERTEX_COUNT, 0);


}
