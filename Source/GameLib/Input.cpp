#include "Input.h"

// ‰Šú‰»
void Input::Initialize(HWND hWnd)
{
	gamePad = std::make_unique<GamePad>();
	mouse = std::make_unique<Mouse>(hWnd);
}

// XVˆ—
void Input::Update()
{
	gamePad->Update();
	mouse->Update();
}

bool Input::IsKeyDown(int vkCode) const
{
    return (GetAsyncKeyState(vkCode) & 0x8000) != 0;
}

bool Input::IsKeyPressed(int vkCode)
{
    bool isPressed = IsKeyDown(vkCode);
    bool justPressed = isPressed && !wasKeyPressed[vkCode];
    wasKeyPressed[vkCode] = isPressed;
    return justPressed;
}

bool Input::IsGamePadButtonPressed(GamePadButton button) const
{
    return (gamePad->GetButtonDown() & button) != 0;
}