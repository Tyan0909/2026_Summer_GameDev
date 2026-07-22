#include "SubjectC.h"
#include "../../../Common/Transform.h"
#include "../../../../Utility/AsoUtility.h"
#include <cfloat>

SubjectC::SubjectC(void)
{
	// 外部からプレイヤー座標が渡される前提のため何もしない
}

SubjectC::~SubjectC(void)
{
}


void SubjectC::InitPost(void)
{
	Subject::InitPost();

	// SubjectC の移動範囲（必要なら調整）
	SetMoveArea(VGet(2000.0f, 0.0f, 2000.0f),
		VGet(2000.0f, 0.0f, 2000.0f));

	// チキンはモデルの前方向が移動方向と逆向きなので
	// 初期向きを moveDir_ の逆向きに合わせておく
	if (!AsoUtility::EqualsVZero(moveDir_))
	{
		transform_.quaRot = Quaternion::LookRotation(AsoUtility::VNormalize(VScale(moveDir_, -1.0f)));
	}
}

void SubjectC::InitTransform(void)
{
	transform_.scl = { 1.01f, 1.01f, 1.01f };
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::Identity();
	transform_.pos = INIT_POS;
	transform_.Update();
}

void SubjectC::InitLoad(void)
{
	// SubjectC のモデルロード
	Subject::InitLoad();

	//　描画されているかどうか確認
	if (transform_.modelId < 0)
	{
		// モデル
		transform_.SetModel(resMng_.LoadModelDuplicate(GetModelType()));
	}
}

VECTOR SubjectC::GetInitPos(void)
{
	// SubjectC 固有の初期位置
	return VGet(200.0f, 100.0f, 0.0f);
}

ResourceManager::SRC SubjectC::GetModelType() const
{
	return ResourceManager::SRC::ENEMY_CHICKEN;
}

void SubjectC::UpdateMove(void)
{
	// プレイヤー座標が無ければデフォルト挙動
	const auto& players = GetPlayerPos();
	if (players.empty())
	{
		Subject::UpdateMove();

		// 親で moveDir_ が変わる可能性があるので向きを修正
		if (!AsoUtility::EqualsVZero(moveDir_))
		{
			transform_.quaRot = Quaternion::LookRotation(AsoUtility::VNormalize(VScale(moveDir_, -1.0f)));
		}
		return;
	}

	// 最も近いプレイヤーを探索
	int nearestIdx = -1;
	float nearestDist = FLT_MAX;
	for (size_t i = 0; i < players.size(); ++i)
	{
		VECTOR diff = VSub(players[i], transform_.pos);
		diff.y = 0.0f;
		const float d = VSize(diff);
		if (d < nearestDist)
		{
			nearestDist = d;
			nearestIdx = static_cast<int>(i);
		}
	}

	// 検知範囲外なら通常挙動へフォールバック
	if (nearestIdx < 0 || nearestDist > playerDetectRange_)
	{
		Subject::UpdateMove();

		if (!AsoUtility::EqualsVZero(moveDir_))
		{
			transform_.quaRot = Quaternion::LookRotation(AsoUtility::VNormalize(VScale(moveDir_, -1.0f)));
		}
		return;
	}

	// 最短プレイヤー方向へ向かう（SubjectA と同様の単純追従）
	VECTOR target = players[nearestIdx];
	VECTOR dir = VSub(target, transform_.pos);
	dir.y = 0.0f;
	const float len = VSize(dir);
	if (len <= 0.0001f)
	{
		Subject::UpdateMove();

		if (!AsoUtility::EqualsVZero(moveDir_))
		{
			transform_.quaRot = Quaternion::LookRotation(AsoUtility::VNormalize(VScale(moveDir_, -1.0f)));
		}
		return;
	}

	dir = VScale(dir, 1.0f / len); // 正規化
	moveDir_ = dir;

	// 移動速度はベースの MOVE_SPEED を使用（必要なら調整）
	transform_.pos = VAdd(transform_.pos, VScale(moveDir_, MOVE_SPEED));

	// チキンはモデルの向きが逆なので moveDir_ の逆向きで回転を作る
	if (!AsoUtility::EqualsVZero(moveDir_))
	{
		transform_.quaRot = Quaternion::LookRotation(AsoUtility::VNormalize(VScale(moveDir_, -1.0f)));
	}
}