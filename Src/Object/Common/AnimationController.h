#pragma once
#include <string>
#include <map>
#include <DxLib.h>
//#include "../Player.h"

class Player;
class AnimationController
{
public:
	// アニメーションデータ
	struct Animation
	{
		int model = -1;
		int attachNo = -1;
		int animIndex = 0;
		float speed = 0.0f;
		float totalTime = 0.0f;
		float step = 0.0f;
		bool loop = true;
	};



	// コンストラクタ
	AnimationController(int modelId);
	// デストラクタ
	~AnimationController(void);

	// 外部FBXからアニメーション追加
	void Add(int type, float speed, const std::string path);

	// 同じFBX内のアニメーションを準備
	void AddInFbx(int type, float speed, int animIndex);

	// アニメーション再生
	void Play(int type, bool isLoop = true);

	// アニメーションの初期化
	void Init(void);
	void Update(void);
	void Release(void);

	bool IsEnd(void);

private:

	// アニメーション再生
	void UpdateAnimation(void);


	// 再生中のアニメーション
	int GetPlayType(void) const;

	// アニメーション追加の共通処理
	void Add(int type, float speed, Animation animation);

	// 再生中のアニメーションタッチNo
	int attachNo_;

	// アニメーションするモデルのハンドルID
	int modelId_;
	// 種類別のアニメーションデータ
	std::map<int, Animation> animations_;

	bool isLoop_;

	// 再生中のアニメーション
	int playType_;
	Animation playAnim_;

};

