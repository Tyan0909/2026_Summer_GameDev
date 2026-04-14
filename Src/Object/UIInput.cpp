#include "UIInput.h"
#include <DxLib.h>

static inline bool KeyTrg(int k) {
    return InputManager::GetInstance().IsTrgDown(k);
}

bool UIInput::IsPadConnected(InputManager::JOYPAD_NO no)
{
    return GetJoypadType(static_cast<int>(no)) != 0;
}

bool UIInput::PadBtn(InputManager::JOYPAD_NO no, InputManager::JOYPAD_BTN btn)
{
    return InputManager::GetInstance().IsPadBtnTrgDown(no, btn);
}

bool UIInput::PadAKeyThreshold(InputManager::JOYPAD_NO no, float thresholdX, float thresholdY)
{
	// アナログスティックの入力値を取得
	InputManager& ins = InputManager::GetInstance();
	InputManager::JOYPAD_IN_STATE padState = ins.GetJPadInputState(no);
	// 指定された閾値を超えているかを判定
	if (padState.AKeyLX >= thresholdX && padState.AKeyLY >= thresholdY) {
		return true;
	}
	return false;
}

InputManager::JOYPAD_NO UIInput::ToPadNo(int idx)
{
    switch (idx)
    {
    case 0: return InputManager::JOYPAD_NO::PAD1;
    case 1: return InputManager::JOYPAD_NO::PAD2;
    case 2: return InputManager::JOYPAD_NO::PAD3;
    case 3: return InputManager::JOYPAD_NO::PAD4;
    default: return InputManager::JOYPAD_NO::PAD1;
    }
}

UINav UIInput::GetNavigate()
{
    UINav n{}; // 明示的に初期化
    n.up = KeyTrg(KEY_INPUT_UP);
    n.down = KeyTrg(KEY_INPUT_DOWN);
    n.left = KeyTrg(KEY_INPUT_LEFT);
    n.right = KeyTrg(KEY_INPUT_RIGHT);
    n.ok = KeyTrg(KEY_INPUT_RETURN) || KeyTrg(KEY_INPUT_SPACE);
    n.cancel = KeyTrg(KEY_INPUT_B);
    return n;
}

TitleInput UIInput::GetTitleInput()
{
    TitleInput ti{}; // 初期化

    ti.anyPadConnected =
        IsPadConnected(InputManager::JOYPAD_NO::PAD1) ||
        IsPadConnected(InputManager::JOYPAD_NO::PAD2) ||
        IsPadConnected(InputManager::JOYPAD_NO::PAD3) ||
        IsPadConnected(InputManager::JOYPAD_NO::PAD4);

    // キーボード入力
    ti.keyboardGoPlayerSelect = KeyTrg(KEY_INPUT_RETURN);
    ti.keyboardGoHowToPlay = KeyTrg(KEY_INPUT_SPACE);

    // どのパッドでも良い場合は全パッドをチェック
    ti.goPlayerSelect =
        PadBtn(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::RIGHT) ||
        PadBtn(InputManager::JOYPAD_NO::PAD2, InputManager::JOYPAD_BTN::RIGHT) ||
        PadBtn(InputManager::JOYPAD_NO::PAD3, InputManager::JOYPAD_BTN::RIGHT) ||
        PadBtn(InputManager::JOYPAD_NO::PAD4, InputManager::JOYPAD_BTN::RIGHT);

    ti.goHowToPlay =
        PadBtn(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN) ||
        PadBtn(InputManager::JOYPAD_NO::PAD2, InputManager::JOYPAD_BTN::DOWN) ||
        PadBtn(InputManager::JOYPAD_NO::PAD3, InputManager::JOYPAD_BTN::DOWN) ||
        PadBtn(InputManager::JOYPAD_NO::PAD4, InputManager::JOYPAD_BTN::DOWN);

    return ti;
}

ManualInput UIInput::GetManualInput()
{
    ManualInput mi{}; // 初期化

    // キーボード
    mi.backToTitle = KeyTrg(KEY_INPUT_SPACE);
    mi.goPlayerSelect = KeyTrg(KEY_INPUT_RETURN);
    mi.prevPage = KeyTrg(KEY_INPUT_LEFT);
    mi.nextPage = KeyTrg(KEY_INPUT_RIGHT);

    // パッド（どれかの PAD の入力で良い）
    bool anyPadBack = PadBtn(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::RIGHT) ||
        PadBtn(InputManager::JOYPAD_NO::PAD2, InputManager::JOYPAD_BTN::RIGHT) ||
        PadBtn(InputManager::JOYPAD_NO::PAD3, InputManager::JOYPAD_BTN::RIGHT) ||
        PadBtn(InputManager::JOYPAD_NO::PAD4, InputManager::JOYPAD_BTN::RIGHT);

    bool anyPadSelect = PadBtn(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN) ||
        PadBtn(InputManager::JOYPAD_NO::PAD2, InputManager::JOYPAD_BTN::DOWN) ||
        PadBtn(InputManager::JOYPAD_NO::PAD3, InputManager::JOYPAD_BTN::DOWN) ||
        PadBtn(InputManager::JOYPAD_NO::PAD4, InputManager::JOYPAD_BTN::DOWN);

    bool anyPadPrev = PadBtn(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::LEFT) ||
        PadBtn(InputManager::JOYPAD_NO::PAD2, InputManager::JOYPAD_BTN::LEFT) ||
        PadBtn(InputManager::JOYPAD_NO::PAD3, InputManager::JOYPAD_BTN::LEFT) ||
        PadBtn(InputManager::JOYPAD_NO::PAD4, InputManager::JOYPAD_BTN::LEFT);

    bool anyPadNext = PadBtn(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::TOP) ||
        PadBtn(InputManager::JOYPAD_NO::PAD2, InputManager::JOYPAD_BTN::TOP) ||
        PadBtn(InputManager::JOYPAD_NO::PAD3, InputManager::JOYPAD_BTN::TOP) ||
        PadBtn(InputManager::JOYPAD_NO::PAD4, InputManager::JOYPAD_BTN::TOP);

    mi.backToTitle = mi.backToTitle || anyPadBack;
    mi.goPlayerSelect = mi.goPlayerSelect || anyPadSelect;
    mi.prevPage = mi.prevPage || anyPadPrev;
    mi.nextPage = mi.nextPage || anyPadNext;

    return mi;
}

PlayerNumInput UIInput::GetPlayerNumInput()
{
    PlayerNumInput pi{}; // 初期化
    pi.pad1Connected = IsPadConnected(InputManager::JOYPAD_NO::PAD1);

    // キーボード操作（左右を複数キーで受け付け）
    pi.left = KeyTrg(KEY_INPUT_LEFT) || KeyTrg(KEY_INPUT_A) || KeyTrg(KEY_INPUT_UP) || KeyTrg(KEY_INPUT_W);
    pi.right = KeyTrg(KEY_INPUT_RIGHT) || KeyTrg(KEY_INPUT_D) || KeyTrg(KEY_INPUT_DOWN) || KeyTrg(KEY_INPUT_S);
    pi.decide = KeyTrg(KEY_INPUT_RETURN) || KeyTrg(KEY_INPUT_SPACE);
    pi.back = KeyTrg(KEY_INPUT_B);


	// PAD1 のみ反応
	if (pi.pad1Connected)
	{
        VECTOR worldInputVec = { 0.0f,0.0f,0.0f };
        static int prevAKeyLY = 0; // 前フレームの値を保持

        InputManager& ins = InputManager::GetInstance();
        InputManager::JOYPAD_IN_STATE padState = ins.GetJPadInputState(InputManager::JOYPAD_NO::PAD1);

        // トリガー判定
        if (padState.AKeyLY == -1000 && prevAKeyLY != -1000)
        {
            pi.left = true;
        }
        else if (padState.AKeyLY == 1000 && prevAKeyLY != 1000)
        {
            pi.right = true;
        }

        prevAKeyLY = padState.AKeyLY; // 前回値を更新
        

		// D-Pad / ボタンでも判定
		//pi.left = pi.left || PadBtn(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::LEFT);   // X / 左
		//pi.right = pi.right || PadBtn(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::RIGHT);  // B / 右

		pi.decide = pi.decide || PadBtn(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN); // A / 決定
		pi.back = pi.back || PadBtn(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::TOP);      // Y / 戻る
	}

    return pi;
}

CharacterSelectInput UIInput::GetCharacterSelectInput(int activePlayerIndex)
{
    CharacterSelectInput ci{}; // 明示的に初期化

    // 有効PAD判定
    auto padNo = ToPadNo(activePlayerIndex);
    ci.activePadConnected = IsPadConnected(padNo);

    // キーボードは常に現在の順番プレイヤー操作として扱う
    ci.left = KeyTrg(KEY_INPUT_LEFT) || KeyTrg(KEY_INPUT_A);
    ci.right = KeyTrg(KEY_INPUT_RIGHT) || KeyTrg(KEY_INPUT_D);
    ci.decide = KeyTrg(KEY_INPUT_RETURN) || KeyTrg(KEY_INPUT_SPACE);
    ci.back = KeyTrg(KEY_INPUT_B);

    // アクティブプレイヤーのPADのみ反応
    if (ci.activePadConnected)
    {
        InputManager& ins = InputManager::GetInstance();
        InputManager::JOYPAD_IN_STATE padState = ins.GetJPadInputState(padNo);

        // 前フレームの値をstatic変数で保持
        static int prevAKeyLX = 0;

        // 左スティックX軸トリガー判定
        const int THRESHOLD = 800; // しきい値（好みで調整）

        // 左に倒した瞬間
        if (padState.AKeyLX < -THRESHOLD && prevAKeyLX >= -THRESHOLD) {
            ci.left = true;
        }
        // 右に倒した瞬間
        if (padState.AKeyLX > THRESHOLD && prevAKeyLX <= THRESHOLD) {
            ci.right = true;
        }

        prevAKeyLX = padState.AKeyLX; // 前回値を更新

        // D-Pad / ボタンでも判定
        /*ci.left = ci.left || PadBtn(padNo, InputManager::JOYPAD_BTN::LEFT);
        ci.right = ci.right || PadBtn(padNo, InputManager::JOYPAD_BTN::RIGHT);*/
        ci.decide = ci.decide || PadBtn(padNo, InputManager::JOYPAD_BTN::DOWN);
        ci.back = ci.back || PadBtn(padNo, InputManager::JOYPAD_BTN::TOP);
    }


    return ci;
}