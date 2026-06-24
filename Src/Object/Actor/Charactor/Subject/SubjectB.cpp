#include "SubjectB.h"

void SubjectB::InitPost(void)
{
	Subject::InitPost();

	// SubjectB‚ÌˆÚ“®”ÍˆÍ‚ğİ’è
	SetMoveArea(VGet(-2000.0f, 0.0f, 500.0f),
		VGet(-2000.0f, 0.0f, 500.0f));
}

VECTOR SubjectB::GetInitPos(void)
{
	return VGet(0.0f, 200.0f, 0.0f);
}

ResourceManager::SRC SubjectB::GetModelType() const
{
	return ResourceManager::SRC::SUBJECT;
}

void SubjectB::UpdateMove(void)
{
	Subject::UpdateMove();
}

