#include "Input.h"

std::unique_ptr<Input> Input::instance = nullptr;

Input* Input::GetInstance()
{
	if (instance == nullptr) {
		instance = std::make_unique<Input>();
	}

	return instance.get();
}

void Input::Initialize(WinApp* winApp)
{
	this->winApp = winApp;

	HRESULT result;

	// DirectInputのインスタンス生成
	result = DirectInput8Create(winApp->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	// キーボードデバイス生成
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));

	// 入力データ形式のセット
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));

	// 排他制御レベルのセット
	result = keyboard->SetCooperativeLevel(winApp->GetHWND(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

	// マウスデバイスの作成
	result = directInput->CreateDevice(GUID_SysMouse, &mouse, nullptr);
	assert(SUCCEEDED(result));

	result = mouse->SetDataFormat(&c_dfDIMouse2);
	assert(SUCCEEDED(result));

	result = mouse->SetCooperativeLevel(winApp->GetHWND(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(result));

	result = mouse->Acquire();
	
	if (FAILED(result)) {
		// 必要ならループで再取得
		while (result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED) {
			result = mouse->Acquire();
		}
	}

	assert(SUCCEEDED(result));

	leftStick = { 0.0f,0.0f };
	rightStick = { 0.0f,0.0f };
}

void Input::Finalize()
{

}

void Input::Update()
{
	if (!isReception_)
	{
		return;
	}
	// マウスの更新	
	MouseUpdate();

	// キーボードの更新	
	KeyBoardUpdate();	

	// ゲームパッドの更新
	GamePadUpdate();	
	
}

bool Input::PushKey(BYTE keyNumber)
{
	if (key[keyNumber]) {
		return true;
	}
	return false;
}

bool Input::Triggerkey(BYTE keyNumber)
{
	// キーが押された瞬間かどうかを判定
	if (!keyPre[keyNumber] && key[keyNumber]) {
		return true;

	}
	return false;
}

bool Input::PushMouseButton(MouseButton mouseButton)
{
	// mouseButton を配列のインデックスに変換
	int buttonIndex = static_cast<int>(mouseButton);

	// インデックスが範囲外でないか確認（念のため）
	if (buttonIndex < 0 || buttonIndex >= 8) {
		return false;
	}

	// 指定されたマウスボタンが押されているかチェック
	return (mouseState.rgbButtons[buttonIndex] & 0x80) != 0;
}

bool Input::TriggerMouseButton(MouseButton mouseButton)
{
	// mouseButton を配列のインデックスに変換
	int buttonIndex = static_cast<int>(mouseButton);
	// インデックスが範囲外でないか確認（念のため）
	if (buttonIndex < 0 || buttonIndex >= 8) {
		return false;
	}
	// 指定されたマウスボタンが押された瞬間かチェック
	return (mouseState.rgbButtons[buttonIndex] & 0x80) != 0 && !(mouseStatePre.rgbButtons[buttonIndex] & 0x80);
}

bool Input::PushGamePadButton(GamePadButton button)
{
	// 指定されたボタンが押されているかチェック
	if (gamePadState.Gamepad.wButtons & static_cast<WORD>(button))
	{
		return true;
	}
	return false;
}

bool Input::TriggerGamePadButton(GamePadButton button)
{
	// 指定されたボタンが押された瞬間かチェック
	if ((gamePadState.Gamepad.wButtons & static_cast<WORD>(button)) && !(gamePadStatePre.Gamepad.wButtons & static_cast<WORD>(button)))
	{
		return true;
	}
	return false;
}

void Input::MouseUpdate()
{
	HRESULT result;

	// マウスの入力状態を取得
	result = mouse->Acquire();
	if (SUCCEEDED(result)) {
		mouse->GetDeviceState(sizeof(DIMOUSESTATE2), &mouseState);
	}

	GetCursorPos(&mousePos);
	ScreenToClient(winApp->GetHWND(), &mousePos);
}

void Input::KeyBoardUpdate()
{
	HRESULT result;

	// 前回のキー入力を保存
	memcpy(keyPre, key, sizeof(key));

	// キーボード情報取得
	result = keyboard->Acquire();

	// 全キーの入力情報を取得する
	result = keyboard->GetDeviceState(sizeof(key), key);
}

void Input::GamePadUpdate()
{
	const float MAX_STICK_VALUE = 32767.0f; // スティックの最大値

	// 成分の初期化
	leftStick = { 0.0f,0.0f };
	rightStick = { 0.0f,0.0f };

	// 前回の入力情報を保存
	gamePadStatePre = gamePadState;

	// 入力情報をクリア
	ZeroMemory(&gamePadState, sizeof(XINPUT_STATE));

	// 接続されている場合
	if (XInputGetState(0, &gamePadState) == ERROR_SUCCESS)
	{
		// 左スティックの入力情報を取得	
		leftStick = { static_cast<float>(gamePadState.Gamepad.sThumbLX), static_cast<float>(gamePadState.Gamepad.sThumbLY) };

		// スティックの位置を正規化
		leftStick.x /= MAX_STICK_VALUE;
		leftStick.y /= MAX_STICK_VALUE;

		if (std::abs(leftStick.x) < deadZone) 
		{
			// デッドゾーン内なので、入力なしとして処理する
			leftStick.x = 0.0f;
		}

		if (std::abs(leftStick.y) < deadZone)
		{
			// デッドゾーン内なので、入力なしとして処理する
			leftStick.y = 0.0f;
		}
		
		// 右スティックの入力情報を取得
		rightStick = { static_cast<float>(gamePadState.Gamepad.sThumbRX), static_cast<float>(gamePadState.Gamepad.sThumbRY) };

		// スティックの位置を正規化
		rightStick.x /= MAX_STICK_VALUE;
		rightStick.y /= MAX_STICK_VALUE;



		if (std::abs(rightStick.x) < deadZone)
		{
			// デッドゾーン内なので、入力なしとして処理する
			rightStick.x = 0.0f;
		}

		if (std::abs(rightStick.y) < deadZone)
		{
			// デッドゾーン内なので、入力なしとして処理する
			rightStick.y = 0.0f;
		}

		rightStick.x /= 3.0f;
		rightStick.y /= 3.0f;
	}
}
