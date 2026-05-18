#include "SubjectManager.h"
#include "../Object/Actor/Charactor/Subject/Subject.h"
#include "../Object/Collider/ColliderBase.h"

SubjectManager::SubjectManager(void)
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
	subject->Init();
	subject->SetPos(pos);

	for (const auto* hitCollider : hitColliders_)
	{
		subject->AddHitCollider(hitCollider);
	}

	subjects_.emplace_back(subject);
	return subject;
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

const std::vector<Subject*>& SubjectManager::GetSubjects(void) const
{
	return subjects_;
}