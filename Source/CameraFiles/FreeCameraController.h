#pragma once 
#include "Camera.h" 

class FreeCameraController
{
public:
	// カメラの値をコントローラーにコピー
	void SyncCameraToController(const Camera& camera);

	// コントローラーの値をカメラに反映
	void SyncControllerToCamera(Camera& camera);

	// カメラ操作の更新
	void Update();

	// デバッグ用UI表示
	void ShowCameraDebugWindow();

	// 自動でカメラを回転させる
	void AutoRotate(float elapsedTime);

private:
	// カメラの位置と向き
	DirectX::XMFLOAT3 eye;    // カメラ位置
	DirectX::XMFLOAT3 focus;  // 注視点（見ている場所）
	DirectX::XMFLOAT3 up;     // 上方向ベクトル

	DirectX::XMFLOAT3 right;  // 右方向ベクトル
	float distance;           // カメラと注視点の距離

	// 回転角度（球面座標）
	float angleX; // 縦回転（Pitch）
	float angleY; // 横回転（Yaw）

	// カメラ操作定数
	static constexpr float MOUSE_SENSITIVITY = 0.02f;      // マウス感度
	static constexpr float ROTATION_SENSITIVITY = 0.5f;    // 回転感度
	static constexpr float PAN_SENSITIVITY = 0.035f;       // パン感度
	static constexpr float ZOOM_SENSITIVITY = 0.1f;        // ズーム感度
	static constexpr float AUTO_ROTATION_SPEED = 0.03f;    // 自動回転速度（ラジアン/秒）
};