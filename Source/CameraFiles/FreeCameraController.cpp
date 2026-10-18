#include <imgui.h> 
#include "CameraFiles/FreeCameraController.h"

void FreeCameraController::SyncCameraToController(const Camera& camera)
{
	// カメラの値をコントローラーにコピー
	eye = camera.GetEye();
	focus = camera.GetFocus();
	up = camera.GetUp();
	right = camera.GetRight();

	// eye と focus の距離を計算
	DirectX::XMVECTOR Eye = DirectX::XMLoadFloat3(&eye);
	DirectX::XMVECTOR Focus = DirectX::XMLoadFloat3(&focus);
	DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(Focus, Eye);
	DirectX::XMVECTOR Distance = DirectX::XMVector3Length(Vec);
	DirectX::XMStoreFloat(&distance, Distance);

	// カメラの向きから回転角度を計算
	const DirectX::XMFLOAT3& front = camera.GetFront();
	angleX = ::asinf(-front.y);

	if (up.y < 0)
	{
		// 上下が反転している場合の補正
		if (front.y > 0)
		{
			angleX = -DirectX::XM_PI - angleX;
		}
		else
		{
			angleX = DirectX::XM_PI - angleX;
		}
		angleY = ::atan2f(front.x, front.z);
	}
	else
	{
		angleY = ::atan2f(-front.x, -front.z);
	}
}

void FreeCameraController::SyncControllerToCamera(Camera& camera)
{
	// コントローラーの値をカメラに反映
	camera.SetLookAt(eye, focus, up);
}

void FreeCameraController::Update()
{
	// ImGui操作中はカメラ更新しない
	if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
	{
		return;
	}

	// ImGuiのマウス入力取得
	ImGuiIO io = ImGui::GetIO();

	// マウス移動量
	float moveX = io.MouseDelta.x * MOUSE_SENSITIVITY;
	float moveY = io.MouseDelta.y * MOUSE_SENSITIVITY;

	// 左クリック：回転
	if (io.MouseDown[ImGuiMouseButton_Left])
	{
		// 横回転（Yaw）
		angleY += moveX * ROTATION_SENSITIVITY;
		if (angleY > DirectX::XM_PI)
			angleY -= DirectX::XM_2PI;
		else if (angleY < -DirectX::XM_PI)
			angleY += DirectX::XM_2PI;

		// 縦回転（Pitch）
		angleX += moveY * ROTATION_SENSITIVITY;
		if (angleX > DirectX::XM_PI)
			angleX -= DirectX::XM_2PI;
		else if (angleX < -DirectX::XM_PI)
			angleX += DirectX::XM_2PI;
	}
	// 中クリック：パン（注視点を移動）
	else if (io.MouseDown[ImGuiMouseButton_Middle])
	{
		float s = distance * PAN_SENSITIVITY;
		float x = moveX * s;
		float y = moveY * s;

		// 左右移動
		focus.x -= right.x * x;
		focus.y -= right.y * x;
		focus.z -= right.z * x;

		// 上下移動
		focus.x += up.x * y;
		focus.y += up.y * y;
		focus.z += up.z * y;
	}
	// 右クリック：ズーム
	else if (io.MouseDown[ImGuiMouseButton_Right])
	{
		distance += (-moveY - moveX) * distance * ZOOM_SENSITIVITY;
	}
	// ホイール：ズーム
	else if (io.MouseWheel != 0)
	{
		distance -= io.MouseWheel * distance * ZOOM_SENSITIVITY;
	}

	// 三角関数計算
	float sx = ::sinf(angleX);
	float cx = ::cosf(angleX);
	float sy = ::sinf(angleY);
	float cy = ::cosf(angleY);

	// カメラ方向ベクトル計算
	DirectX::XMVECTOR Front = DirectX::XMVectorSet(-cx * sy, -sx, -cx * cy, 0.0f);
	DirectX::XMVECTOR Right = DirectX::XMVectorSet(cy, 0, -sy, 0.0f);
	DirectX::XMVECTOR Up = DirectX::XMVector3Cross(Right, Front);

	// カメラ位置計算（focus - front * distance）
	DirectX::XMVECTOR Focus = DirectX::XMLoadFloat3(&focus);
	DirectX::XMVECTOR Distance = DirectX::XMVectorSet(distance, distance, distance, 0.0f);
	DirectX::XMVECTOR Eye = DirectX::XMVectorSubtract(Focus, DirectX::XMVectorMultiply(Front, Distance));

	// ビュー行列作成
	DirectX::XMMATRIX View = DirectX::XMMatrixLookAtLH(Eye, Focus, Up);
	DirectX::XMMATRIX World = DirectX::XMMatrixTranspose(View);

	// Right と Up をワールドから再計算
	Right = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(1, 0, 0, 0), World);
	Up = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0, 1, 0, 0), World);

	// 結果を保存
	DirectX::XMStoreFloat3(&eye, Eye);
	DirectX::XMStoreFloat3(&up, Up);
	DirectX::XMStoreFloat3(&right, Right);
}


void FreeCameraController::AutoRotate(float elapsedTime)
{

	// Y軸（横方向）を自動回転
	angleY += AUTO_ROTATION_SPEED * elapsedTime;

	// 角度を -π ～ π に収める
	if (angleY > DirectX::XM_PI)
	{
		angleY -= DirectX::XM_2PI;
	}
	else if (angleY < -DirectX::XM_PI)
	{
		angleY += DirectX::XM_2PI;
	}

	// 三角関数の計算
	float sx = ::sinf(angleX);
	float cx = ::cosf(angleX);
	float sy = ::sinf(angleY);
	float cy = ::cosf(angleY);

	// カメラ方向ベクトルを計算
	DirectX::XMVECTOR Front = DirectX::XMVectorSet(-cx * sy, -sx, -cx * cy, 0.0f);
	DirectX::XMVECTOR Right = DirectX::XMVectorSet(cy, 0, -sy, 0.0f);
	DirectX::XMVECTOR Up = DirectX::XMVector3Cross(Right, Front);

	// カメラ位置（Eye）を計算
	DirectX::XMVECTOR Focus = DirectX::XMLoadFloat3(&focus);
	DirectX::XMVECTOR Distance = DirectX::XMVectorSet(distance, distance, distance, 0.0f);
	DirectX::XMVECTOR Eye = DirectX::XMVectorSubtract(Focus, DirectX::XMVectorMultiply(Front, Distance));

	// ビュー行列を作成
	DirectX::XMMATRIX View = DirectX::XMMatrixLookAtLH(Eye, Focus, Up);
	DirectX::XMMATRIX World = DirectX::XMMatrixTranspose(View);

	// Right と Up を再計算
	Right = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(1, 0, 0, 0), World);
	Up = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0, 1, 0, 0), World);

	// 値を保存
	DirectX::XMStoreFloat3(&eye, Eye);
	DirectX::XMStoreFloat3(&up, Up);
	DirectX::XMStoreFloat3(&right, Right);
}

void FreeCameraController::ShowCameraDebugWindow()
{
	ImGui::Begin("Camera Debug");

	// カメラ位置（Eye）
	ImGui::Text("Eye (Position):");
	ImGui::Text("  X: %.3f", eye.x);
	ImGui::Text("  Y: %.3f", eye.y);
	ImGui::Text("  Z: %.3f", eye.z);

	ImGui::Separator();

	// Upベクトル表示
	ImGui::Text("Up Vector:");
	ImGui::Text("  X: %.3f", up.x);
	ImGui::Text("  Y: %.3f", up.y);
	ImGui::Text("  Z: %.3f", up.z);

	ImGui::Separator();

	// Frontベクトル計算して表示
	ImGui::Text("Front Vector:");
	float sx = sinf(angleX);
	float cx = cosf(angleX);
	float sy = sinf(angleY);
	float cy = cosf(angleY);

	DirectX::XMFLOAT3 front{};
	front.x = -cx * sy;
	front.y = -sx;
	front.z = -cx * cy;

	ImGui::Text("  X: %.3f", front.x);
	ImGui::Text("  Y: %.3f", front.y);
	ImGui::Text("  Z: %.3f", front.z);

	ImGui::End();
}

