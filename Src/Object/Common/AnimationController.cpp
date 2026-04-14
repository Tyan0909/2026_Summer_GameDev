#include <DxLib.h>
#include "AnimationController.h"
#include "../../Manager/SceneManager.h"

AnimationController::AnimationController(int modelId)
{
	modelId_ = modelId;
	playType_ = -1;

}

AnimationController::~AnimationController(void)
{
	int eh[5];

	std::map<int, int>enemysHp_;
	enemysHp_.emplace(0, 1);

}

void AnimationController::Add(int type, float speed, const std::string path)
{
	Animation animation;
	animation.model = MV1LoadModel(path.c_str());
	animation.animIndex = -1;

	Add(type, speed, animation);
}

void AnimationController::AddInFbx(int type, float speed, int animIndex)
{
	Animation animation;
	animation.model = -1;
	animation.animIndex = animIndex;


	Add(type, speed, animation);

}

void AnimationController::Play(int type, bool isLoop)
{
	if (playType_ == type)
	{
		// 同じアニメーションだったら再生を継続する
		return;
	}

	if (playType_ != -1)
	{
		// モデルからアニメーションを外す
		MV1DetachAnim(modelId_, playAnim_.attachNo);
	}

	// アニメーション種別を変更
	playType_ = type;
	playAnim_ = animations_[type];

	// 初期化
	playAnim_.step = 0.0f; ;

	// モデルにアニメーションを付ける
	if (playAnim_.model == -1)
	{
		// モデルと同じファイルからアニメーションをアタッチする
		playAnim_.attachNo = MV1AttachAnim(modelId_, playAnim_.animIndex);
	}
	else
	{
		// 別のモデルファイルからアニメーションをアタッチする
		// DxModelViewerを確認すること(大体0か1)
		int animIdx = 0;
		playAnim_.attachNo =
			MV1AttachAnim(modelId_, animIdx, playAnim_.model);
	}


	// アニメーション総時間の取得
	playAnim_.totalTime = MV1GetAnimTotalTime(modelId_, playAnim_.animIndex);

	isLoop_ = isLoop;
}

void AnimationController::Init(void)
{
}

void AnimationController::Update(void)
{
	// 経過時間の取得
	float deltaTime = SceneManager::GetInstance().GetDeltaTime();
	// 再生
	playAnim_.step += (deltaTime * playAnim_.speed);

	// アニメーションのループ
	if (playAnim_.step > playAnim_.totalTime)
	{
		if (isLoop_)
		{
			// ループする
			playAnim_.step = 0.0f;
		}
		else
		{
			// 終了する
			playAnim_.step = playAnim_.totalTime;
		}
	}

	// アニメーション設定
	MV1SetAttachAnimTime(modelId_, playAnim_.attachNo, playAnim_.step);


}

void AnimationController::Release(void)
{
	//可変長配列を削除
	animations_.clear();

	//外部のアニメーションの削除
	for (const std::pair<int, Animation>& pair : animations_)
	{
		if (pair.second.model != -1)
		{
			// モデルの削除
			MV1DeleteModel(pair.second.model);
		}
	}
}


void AnimationController::UpdateAnimation(void)
{

}

bool AnimationController::IsEnd(void)
{
	bool ret = false;

	if (isLoop_)
	{
		//ループ設定されたら
		//無条件で終了してない（false）を返す
		return ret;
	}

	//アニメーションが終了したら
	if (playAnim_.step >= playAnim_.totalTime)
	{
		//再生時間が過ぎている
		return  true;
	}
	return false;
}

int AnimationController::GetPlayType(void) const
{
	return playType_;
}

void AnimationController::Add(int type, float speed, Animation animation)
{
	animation.speed = speed;
	if (animations_.count(type) == 0)
	{
		animations_.emplace(type, std::move(animation));
	}
}
