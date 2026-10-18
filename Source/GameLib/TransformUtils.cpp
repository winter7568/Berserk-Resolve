#include <stdlib.h>
#include <algorithm>
#include <random>

#include "TransformUtils.h"

// 行列からヨー、ピッチ、ロールを行列を計算する。
bool TransformUtils::MatrixToRollPitchYaw(const DirectX::XMFLOAT4X4& m, float& pitch, float& yaw, float& roll)
{
	float xRadian = asinf(-m._32);
	pitch = xRadian;
	if (xRadian < DirectX::XM_PI / 2.0f)
	{
		if (xRadian > -DirectX::XM_PI / 2.0f)
		{
			roll = atan2f(m._12, m._22);
			yaw = atan2f(m._31, m._33);
			return true;
		}
		else
		{
			roll = (float)-atan2f(m._13, m._11);
			yaw = 0.0f;
			return false;
		}
	}
	else
	{
		roll = (float)atan2f(m._13, m._11);
		yaw = 0.0f;
		return false;
	}
}

// クォータニオンからヨー、ピッチ、ロールを行列を計算する。
bool TransformUtils::QuaternionToRollPitchYaw(const DirectX::XMFLOAT4& q, float& pitch, float& yaw, float& roll)
{
	DirectX::XMVECTOR Q = DirectX::XMLoadFloat4(&q);
	DirectX::XMMATRIX M = DirectX::XMMatrixRotationQuaternion(Q);
	DirectX::XMFLOAT4X4 m;
	DirectX::XMStoreFloat4x4(&m, M);
	return MatrixToRollPitchYaw(m, pitch, yaw, roll);
}


DirectX::XMFLOAT3 TransformUtils::QuaternionToEulerAngles(const DirectX::XMFLOAT4& quat)
{
	DirectX::XMVECTOR q = DirectX::XMLoadFloat4(&quat);
	DirectX::XMMATRIX rotMatrix = DirectX::XMMatrixRotationQuaternion(q);
	DirectX::XMFLOAT4X4 rotFloat4x4;
	DirectX::XMStoreFloat4x4(&rotFloat4x4, rotMatrix);

	float pitch, yaw, roll;
	MatrixToRollPitchYaw(rotFloat4x4, pitch, yaw, roll);
	return DirectX::XMFLOAT3(pitch, yaw, roll);
}

float TransformUtils::RandomRange(float min, float max)
{
	static std::mt19937 rng{ std::random_device{}() };
	std::uniform_real_distribution<float> dist(min, max);
	return dist(rng);
}

// 中央0のランダム値
float TransformUtils::RandomCentered(int range, float scale)
{
	return (rand() % range - range / 2) * scale;
}

// 基準値にランダム加算
float TransformUtils::RandomPositive(float base, int range, float scale)
{
	return base + (rand() % range) * scale;
}

// 単純な正のランダム値
float TransformUtils::RandomPositiveOnly(int range, float scale)
{
	return (rand() % range) * scale;
}

// 整数ランダム値
int TransformUtils::RandomInt(int min, int max)
{
	return min + (rand() % (max - min + 1));
}

// 継続時間ベースのランダムライフタイム
float TransformUtils::RandomScaledLifetime(float duration, int range, float scale)
{
	return duration * (0.5f + (rand() % range) * scale);
}