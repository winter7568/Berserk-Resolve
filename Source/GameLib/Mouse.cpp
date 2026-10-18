#include "Mouse.h"

static const int KeyMap[] =
{
	VK_LBUTTON,		// 左ボタン
	VK_MBUTTON,		// 中ボタン
	VK_RBUTTON,		// 右ボタン
};

// コンストラクタ
Mouse::Mouse(HWND hWnd)
	: hWnd(hWnd)
{
	RECT rc;
	GetClientRect(hWnd, &rc);
	screenWidth = rc.right - rc.left;
	screenHeight = rc.bottom - rc.top;
}

// 更新
void Mouse::Update()
{
	// スイッチ情報
	MouseButton newButtonState = 0;

	for (int i = 0; i < ARRAYSIZE(KeyMap); ++i)
	{
		if (::GetAsyncKeyState(KeyMap[i]) & 0x8000)
		{
			newButtonState |= (1 << i);
		}
	}

	// ホイール
	wheel[1] = wheel[0];
	wheel[0] = 0;

	// ボタン情報更新
	buttonState[1] = buttonState[0];	// スイッチ履歴
	buttonState[0] = newButtonState;

	buttonDown = ~buttonState[1] & newButtonState;	// 押した瞬間
	buttonUp = ~newButtonState & buttonState[1];	// 離した瞬間

	// カーソル位置の取得
	POINT cursor;
	::GetCursorPos(&cursor);
	::ScreenToClient(hWnd, &cursor);

	// 画面のサイズを取得する。
	RECT rc;
	GetClientRect(hWnd, &rc);
	UINT screenW = rc.right - rc.left;
	UINT screenH = rc.bottom - rc.top;
	UINT viewportW = screenWidth;
	UINT viewportH = screenHeight;

	// 画面補正
	positionX[1] = positionX[0];
	positionY[1] = positionY[0];
	positionX[0] = (LONG)(cursor.x / static_cast<float>(viewportW) * static_cast<float>(screenW));
	positionY[0] = (LONG)(cursor.y / static_cast<float>(viewportH) * static_cast<float>(screenH));
}

// マウスの表示/非表示を制御
void Mouse::ShowCursor(bool show)
{
	if (show)
	{
		// カーソルを表示：カウンタが正になるまでインクリメント
		while (::ShowCursor(TRUE) < 0) {}
	}
	else
	{
		// カーソルを非表示：カウンタが負になるまでデクリメント
		while (::ShowCursor(FALSE) >= 0) {}
	}
}

// マウスカーソルを指定位置に移動
void Mouse::SetPosition(int x, int y)
{
	// クライアント座標を画面座標に変換
	POINT point = { x, y };
	ClientToScreen(hWnd, &point);
	SetCursorPos(point.x, point.y);

	// 内部の位置情報を更新（スクリーンサイズに合わせてスケーリング）
	RECT rc;
	GetClientRect(hWnd, &rc);
	UINT screenW = rc.right - rc.left;
	UINT screenH = rc.bottom - rc.top;
	UINT viewportW = screenWidth;
	UINT viewportH = screenHeight;

	// 古い位置を保存
	positionX[1] = positionX[0];
	positionY[1] = positionY[0];

	// 新しい位置を計算（viewport → screen スケーリング）
	positionX[0] = (LONG)(x / static_cast<float>(viewportW) * static_cast<float>(screenW));
	positionY[0] = (LONG)(y / static_cast<float>(viewportH) * static_cast<float>(screenH));
}

void Mouse::CenterCursor()
{
	// ウィンドウが最小化されていないかチェック
	if (IsIconic(hWnd))
	{
		return;  // 最小化中は何もしない
	}

	// ウィンドウのクライアント領域の中心を取得
	RECT rect;
	GetClientRect(hWnd, &rect);

	// クライアント領域が有効かチェック
	if (rect.right <= rect.left || rect.bottom <= rect.top)
	{
		return;
	}

	int centerX = (rect.left + rect.right) / 2;
	int centerY = (rect.top + rect.bottom) / 2;

	SetPosition(centerX, centerY);
}

// マウス状態をリセット
void Mouse::Reset()
{
	// ボタン状態をリセット
	buttonState[0] = 0;
	buttonState[1] = 0;
	buttonDown = 0;
	buttonUp = 0;

	// 位置情報をリセット
	positionX[0] = 0;
	positionX[1] = 0;
	positionY[0] = 0;
	positionY[1] = 0;

	// ホイールをリセット
	wheel[0] = 0;
	wheel[1] = 0;
}

void Mouse::SetVisible(bool visible)
{
	// Windowsのカーソル表示はカウンタ方式なので強制的に設定する
	if (visible)
	{
		// カーソルを表示：カウンタが正になるまでインクリメント
		while (::ShowCursor(TRUE) < 0) {}
	}
	else
	{
		// カーソルを非表示：カウンタが負になるまでデクリメント
		while (::ShowCursor(FALSE) >= 0) {}
	}
}

void Mouse::ConfineToWindow(bool confine)
{
	if (confine)
	{
		// ウィンドウが最小化されていないかチェック
		if (IsIconic(hWnd))
		{
			// 最小化中は制限を解除
			::ClipCursor(nullptr);
			return;
		}

		RECT rect;
		GetClientRect(hWnd, &rect);

		// クライアント領域が有効かチェック
		if (rect.right <= rect.left || rect.bottom <= rect.top)
		{
			::ClipCursor(nullptr);
			return;
		}

		POINT topLeft = { rect.left, rect.top };
		POINT bottomRight = { rect.right, rect.bottom };

		ClientToScreen(hWnd, &topLeft);
		ClientToScreen(hWnd, &bottomRight);

		rect.left = topLeft.x;
		rect.top = topLeft.y;
		rect.right = bottomRight.x;
		rect.bottom = bottomRight.y;

		::ClipCursor(&rect);
	}
	else
	{
		::ClipCursor(nullptr);
	}
}