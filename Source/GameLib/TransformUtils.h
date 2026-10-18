#pragma once 
#include <DirectXMath.h> 

class TransformUtils
{
public:
	// 行列からヨー、ピッチ、ロールを行列を計算する。 
	static bool MatrixToRollPitchYaw(const DirectX::XMFLOAT4X4& m, float& pitch, float& yaw, float& roll);
	// クォータニオンからヨー、ピッチ、ロールを行列を計算する。 
	static bool QuaternionToRollPitchYaw(const DirectX::XMFLOAT4& q, float& pitch, float& yaw, float& roll);

	// クォータニオン → オイラー角（X: Pitch, Y: Yaw, Z: Roll）を返すユーティリティ
	static DirectX::XMFLOAT3 QuaternionToEulerAngles(const DirectX::XMFLOAT4& quat);

	// 乱数生成ヘルパー
	static float RandomRange(float min, float max);
	static float RandomCentered(int range, float scale);
	static float RandomPositive(float base, int range, float scale);
	static float RandomPositiveOnly(int range, float scale);
	static int RandomInt(int min, int max);
	static float RandomScaledLifetime(float duration, int range, float scale);
};