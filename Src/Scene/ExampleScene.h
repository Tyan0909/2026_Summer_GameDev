#pragma once
#include "SceneBase.h"
#include <DxLib.h>

// 説明シーン
class ExampleScene : public SceneBase
{
public:

	// コンストラクタ
	ExampleScene(void);

	// デストラクタ
	~ExampleScene(void) override;

	// 初期化
	void Init(void) override;

	// 更新
	void Update(void) override;

	// 描画
	void Draw(void) override;

	// 解放
	void Release(void) override;

private:

	// ここにメンバ変数を追加していく
	int exampleImg_;
};