#include "SubjectManager.h"
#include "../Object/Actor/Charactor/Subject/Subject.h"
#include "../Object/Actor/Charactor/Subject/SubjectA.h"
#include "../Object/Actor/Charactor/Subject/SubjectB.h"
#include "../Object/Collider/ColliderBase.h"
#include "../Utility/AsoUtility.h"

SubjectManager::SubjectManager(void)
	:
	moveAreaMin_(VGet(-3600.0f, 0.0f, -790.0f)),
	moveAreaMax_(VGet(11100.0f, 0.0f, 11900.0f)),
	spawnAreaMax_(VGet(11100.0f, 0.0f, 11900.0f)),
	spawnAreaMin_(VGet(-3600.0f, 0.0f, -790.0f))
{
}

SubjectManager::~SubjectManager(void)
{
}

void SubjectManager::Init(void)
{
}

void SubjectManager::Update(void)
{
	

	for (auto it = subjects_.begin();
		it != subjects_.end();)
	{
		Subject* subject = *it;

		if (subject == nullptr)
		{
			it = subjects_.erase(it);
			continue;
		}

		subject->Update();

		if (subject->IsDead())
		{

			subject->Release();
			delete subject;

			it = subjects_.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void SubjectManager::Draw(void)
{
	for (auto* subject : subjects_)
	{
		if (subject == nullptr)
		{
			continue;
		}

		subject->Draw();
	}
}

void SubjectManager::Release(void)
{
	for (auto* subject : subjects_)
	{
		if (subject == nullptr)
		{
			continue;
		}

		subject->Release();
		delete subject;
	}
	subjects_.clear();
	hitColliders_.clear();
}

Subject* SubjectManager::CreateSubject(SUBJECT_TYPE type, const VECTOR& pos)
{
	Subject* subject = nullptr;

	switch (type)
	{
	case SubjectManager::SUBJECT_TYPE::SUBJECT_A:
		subject = new SubjectA();
		break;
	case SubjectManager::SUBJECT_TYPE::SUBJECT_B:
		subject = new SubjectB();
		break;
	/*case SubjectManager::SUBJECT_TYPE::SUBJECT_C:*/
	/*	break;*/
	/*case SubjectManager::SUBJECT_TYPE::SUBJECT_D:*/
		/*break;*/
	default:
		return nullptr;
		break;
	}

	subject->Init();
	subject->SetPos(pos);

	for (const auto* hitCollider : hitColliders_ )
	{
		subject->AddHitCollider(hitCollider);
	}
	subject->SetMoveArea(moveAreaMin_, moveAreaMax_);
	subjects_.emplace_back(subject);
	return subject;
}


Subject* SubjectManager::CreateRandomSubject()
{
	// ämó¶ïœìÆÇÕ50%Ç≈ÅASUBJECT_AÇ∆SUBJECT_BÇÃÇ¢Ç∏ÇÍÇ©Çê∂ê¨Ç∑ÇÈ
	const VECTOR pos = VGet(
		GetRandomRange(spawnAreaMin_.x, spawnAreaMax_.x),
		SUBJECT_SPAWN_HEIGHT,
		GetRandomRange(spawnAreaMin_.z, spawnAreaMax_.z));

	SUBJECT_TYPE type =
		static_cast<SUBJECT_TYPE>(
			GetRand(static_cast<int>(SUBJECT_TYPE::MAX) - 1));

	//if (type == SUBJECT_TYPE::SUBJECT_A)
	//{
	//	printfDx("SubjectAê∂ê¨\n");
	//}
	//else if (type == SUBJECT_TYPE::SUBJECT_B)
	//{
	//	printfDx("SubjectBê∂ê¨\n");
	//}


	return CreateSubject(type, pos);
}

void SubjectManager::SetSpawnArea(const VECTOR& minPos, const VECTOR& maxPos)
{
	spawnAreaMin_ = minPos;
	spawnAreaMax_ = maxPos;
}

void SubjectManager::AddHitCollider(const ColliderBase* hitCollider)
{
	if (hitCollider == nullptr)
	{
		return;
	}

	for (const auto* collider : hitColliders_)
	{
		if (collider == hitCollider)
		{
			return;
		}
	}

	hitColliders_.emplace_back(hitCollider);

	for (auto* subject : subjects_)
	{
		if (subject == nullptr)
		{
			continue;
		}

		subject->AddHitCollider(hitCollider);
	}
}

void SubjectManager::SetMoveArea(const VECTOR& minPos, const VECTOR& maxPos)
{
	moveAreaMin_ = minPos;
	moveAreaMax_ = maxPos;

	for (auto* subject : subjects_)
	{
		if (subject == nullptr)
		{
			continue;
		}

		subject->SetMoveArea(moveAreaMin_, moveAreaMax_);
	}
}

const std::vector<Subject*>& SubjectManager::GetSubjects(void) const
{
	return subjects_;
}

void SubjectManager::RemoveSubject(Subject* subject)
{
	if (subject == nullptr) return;
	for (auto it = subjects_.begin(); it != subjects_.end(); ++it)
	{
		if (*it == subject)
		{
			(*it)->Release();
			delete* it;
			subjects_.erase(it);
			return;
		}
	}
}

float SubjectManager::GetRandomRange(float minValue, float maxValue) const
{
	if (maxValue <= minValue)
	{
		return minValue;
	}

	const float t = static_cast<float>(GetRand(10000)) / 10000.0f;
	return minValue + (maxValue - minValue) * t;
}