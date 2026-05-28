#pragma once
#include "SceneBase.h"
#include <DxLib.h>

class AnimationController;

// プレイヤー人数選択シーン
class PlayerNumScene : public SceneBase
{
public:

	// コンストラクタ
	PlayerNumScene(void);
	// デストラクタ
	~PlayerNumScene(void) override;
	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

	// 選択中の人数を取得
	int GetSelectNum(void) const { return selectNum_ + 1; }

	// 各プレイヤーのスクリーンオフセットを設定 / 取得（px）
	void SetPlayerOffset(int idx, int offsetX, int offsetY);
	void GetPlayerOffset(int idx, int& outOffsetX, int& outOffsetY) const;

private:
	// 選択肢
	enum SELECT
	{
		SELECT_1P = 0,
		SELECT_2P,
		SELECT_3P,
		SELECT_4P,
		SELECT_MAX
	};

	SELECT currentSelect_;

	// 選択中の番号
	int selectNum_;

	// 背景用画像
	int bgImg_;

	// 人数選択画像（2Dフォールバック）
	int selectImg_[SELECT_MAX];

	// 生成した何もないクリーンなガレージ背景画像を読み込みます
	int selectPromptImg_;

	int useImg_;
	int notUseImg_;
	int decideSE_;

	bool isUsePlayer_[4];
	int cursor_;

	// 3Dモデルハンドル（各スロット）
	int modelId_[SELECT_MAX];

	int playAnimIndex_[SELECT_MAX];

	// AnimationController ハンドル
	AnimationController* animCtrl_[SELECT_MAX];

	// 追加：各プレイヤーごとのスクリーンオフセット（px）
	int playerOffsetX_[SELECT_MAX];
	int playerOffsetY_[SELECT_MAX];
};

