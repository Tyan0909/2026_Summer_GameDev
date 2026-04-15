#include "Result.h"
#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"
#include <DxLib.h>

Result::Result(void) {}
Result::~Result(void) {}

void Result::Init(void) {}

void Result::Update(void)
{
    InputManager& ins = InputManager::GetInstance();
    SceneManager& scene = SceneManager::GetInstance();

    // 例：ゲームで獲得した金額
    int resultAmount = 5000;

    // SPACE：タイトルへ（リセット）
    if (ins.IsTrgDown(KEY_INPUT_SPACE))
    {
        scene.SetCarryMoney(0);
        scene.ChangeScene(SceneManager::SCENE_ID::TITLE);
    }

    // ENTER：次の買い物へ（清算：あまり + 獲得 を carry に合算）
    if (ins.IsTrgDown(KEY_INPUT_RETURN))
    {
        // 例：prevCarry = 1200（買い物後のあまり分）
        int prevCarry = scene.GetCarryMoney();

        // 例：carry = 1200 + 5000 = 6200
        scene.SetCarryMoney(prevCarry + resultAmount);

        scene.ChangeScene(SceneManager::SCENE_ID::BUYSELECT);
    }
}

void Result::Draw(void)
{
    int resultAmount = 5000;
    int currentCarry = SceneManager::GetInstance().GetCarryMoney();

    DrawString(200, 200, "リザルトシーン", GetColor(255, 255, 255));
    DrawFormatString(200, 250, GetColor(255, 255, 255), "今回の獲得G: %d", resultAmount);

    // 次回買い物で使える総額 = 最低保証1500 +（持ち越し可変分）+（今回獲得分）
    // ※UpdateでENTER押下前なので、ここは「参考表示」になっています
    DrawFormatString(200, 300, GetColor(0, 255, 255),
        "次回の購入可能総額(目安): %d", (1500 + currentCarry + resultAmount));
}

void Result::Release(void) {}