#pragma once
#include <DxLib.h>

class Player
{
	// 定数
public:

	// プレイヤーの移動速度
	static constexpr float MOVE_SPEED = 0.2f;
	// プレイヤーの回転速度
	static constexpr float ROTATE_SPEED = 0.05f;
	// コンストラクタ
	Player();
	// デストラクタ
	~Player();
	// 初期化
	void Init(void);
	// 更新
	void Update(void);
	// 描画
	void Draw(void);
	// 解放
	void Release(void);


private:

	// プレイヤーモデルID
	int modelId_;
	// 位置・角度・拡縮
	VECTOR pos_;
	VECTOR angle_;
	VECTOR scale_;


};

