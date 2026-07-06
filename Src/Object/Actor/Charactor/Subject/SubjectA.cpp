#include "SubjectA.h"
#include "../../../Common/Transform.h"
#include "../Player/Player.h"

SubjectA::SubjectA(void)
{
	player_ = new Player();
}

SubjectA::~SubjectA(void)
{
}

void SubjectA::InitPost(void)
{
	Subject::InitPost();

	// SubjectAの移動範囲を設定
	SetMoveArea(VGet(-40000.0f, 0.0f, -40000.0f),
		VGet(40000.0f, 0.0f, 40000.0f));
}

VECTOR SubjectA::GetInitPos(void)
{
	return VGet(0.0f, 100.0f, 0.0f);
}

ResourceManager::SRC SubjectA::GetModelType() const
{
	return ResourceManager::SRC::SUBJECT;
}

void SubjectA::UpdateMove(void)
{
	//printfDx("before = %f\n", transform_.pos.x);
	 //printfDx("after = %f\n", transform_.pos.x);

	Subject::UpdateMove();

	// 個別処理

	// Aの追従処理を追加

	// プレイヤーの位置を取得







}
