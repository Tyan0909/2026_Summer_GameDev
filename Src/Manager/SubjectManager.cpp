#include "SubjectManager.h"
#include "../Object/Actor/Charactor/Subject/Subject.h"
#include "../Object/Collider/ColliderBase.h"

SubjectManager::SubjectManager(void)
	:
	moveAreaMin_(VGet(-500.0f, 0.0f, -500.0f)),
	moveAreaMax_(VGet(500.0f, 0.0f, 500.0f))
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
	for (auto* subject : subjects_)
	{
		if (subject == nullptr)
		{
			continue;
		}

		subject->Update();
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

Subject* SubjectManager::CreateSubject(ResourceManager::SRC modelSrc, const VECTOR& pos)
{
	Subject* subject = new Subject();
	subject->SetModelSrc(modelSrc);
	subject->SetMoveArea(moveAreaMin_, moveAreaMax_);
	subject->Init();
	subject->SetPos(pos);

	for (const auto* hitCollider : hitColliders_)
	{
		subject->AddHitCollider(hitCollider);
	}

	subjects_.emplace_back(subject);
	return subject;
}

Subject* SubjectManager::CreateRandomSubject(ResourceManager::SRC modelSrc)
{
	const VECTOR pos = VGet(
		GetRandomRange(moveAreaMin_.x, moveAreaMax_.x),
		SUBJECT_SPAWN_HEIGHT,
		GetRandomRange(moveAreaMin_.z, moveAreaMax_.z));

	return CreateSubject(modelSrc, pos);
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