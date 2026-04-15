#pragma once
#include "../Scene/GameScene.h"
#include <DxLib.h>
#include <vector>

class Stage
{
public:

	// 定数
	// ステージモデルの拡散光の強さ
	static constexpr float DIFFUSE_STRENGTH = 0.8f;



	// コンストラクタ
	Stage();

	// デストラクタ
	~Stage();

	// 初期化
	void Init(void);
	// 更新
	void Update(void);
	// 描画
	void Draw(void);
	// 解放
	void Release(void);

private:

	// ステージモデルID
	int modelId_;

	// 位置・角度・拡縮
	VECTOR pos_;
	VECTOR angle_;
	VECTOR scale_;

};