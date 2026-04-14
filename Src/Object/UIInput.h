#pragma once
#include "../Manager/InputManager.h"


struct UINav
{
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool ok = false;
    bool cancel = false;
};

// タイトル専用入力結果
struct TitleInput
{
    bool goPlayerSelect = false; // B (人数選択へ)
    bool goHowToPlay = false; // A (操作説明へ)
    bool anyPadConnected = false;
    bool keyboardGoPlayerSelect = false; // Enter
    bool keyboardGoHowToPlay = false; // Space
};

// 説明画面専用入力
struct ManualInput
{
    bool backToTitle = false; // B / Esc
    bool goPlayerSelect = false; // A / Enter / Space
    bool nextPage = false; // Y / →
    bool prevPage = false; // X / ←
};

// 人数選択専用 (キーボード + PAD1 のみ)
struct PlayerNumInput
{
    bool pad1Connected = false;
    bool left = false;   // ← / A / ↑ / W / (PAD1: X)
    bool right = false;  // → / D / ↓ / S / (PAD1: B)
    bool decide = false; // Enter / Space / (PAD1: A)
    bool back = false;   // B / (PAD1: Y)
};

// キャラ選択: 現在の順番で選択中プレイヤーのみ操作可能
struct CharacterSelectInput
{
    bool activePadConnected = false; // 現在順番プレイヤーのPAD接続状態
    bool left = false;               // キャラ切替 (← / A / PAD: X)
    bool right = false;              // キャラ切替 (→ / D / PAD: B)
    bool decide = false;             // 確定 (Enter / Space / PAD: A)
    bool back = false;               // 戻る (B / PAD: Y)
};

class UIInput
{
public:
    static UINav GetNavigate();
    static TitleInput GetTitleInput();
    static ManualInput GetManualInput();
    static PlayerNumInput GetPlayerNumInput();

    // activePlayerIndex: 0～(playerCount-1)
    static CharacterSelectInput GetCharacterSelectInput(int activePlayerIndex);

private:
    static bool IsPadConnected(InputManager::JOYPAD_NO no);
    static bool PadBtn(InputManager::JOYPAD_NO no, InputManager::JOYPAD_BTN btn);
    
	// スティックの傾きを一定以上検出
	static bool PadAKeyThreshold(InputManager::JOYPAD_NO no, float thresholdX, float thresholdY);

    static InputManager::JOYPAD_NO ToPadNo(int idx); // 0->PAD1 ...

};