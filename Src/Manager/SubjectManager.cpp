#include "SubjectManager.h"
#include "../Object/Actor/Charactor/Subject/Subject.h"
#include "../Object/Actor/Charactor/Subject/SubjectA.h"
#include "../Object/Actor/Charactor/Subject/SubjectB.h"
#include "../Object/Actor/Charactor/Subject/SubjectC.h"
#include "../Object/Collider/ColliderBase.h"
#include "../Utility/AsoUtility.h"
#include <algorithm>

SubjectManager::SubjectManager(void)
	:
	moveAreaMin_(VGet(-3600.0f, 0.0f, -790.0f)),
	moveAreaMax_(VGet(11100.0f, 0.0f, 11900.0f)),
	spawnAreaMax_(VGet(11100.0f, 0.0f, 11900.0f)),
	spawnAreaMin_(VGet(-3600.0f, 0.0f, -790.0f))
{
	// デフォルト: 全タイプ同等の重み、同時出現数無制限
	const int typeCount = static_cast<int>(SUBJECT_TYPE::MAX);
	spawnWeights_.assign(typeCount, 1.0f);
	maxConcurrent_.assign(typeCount, -1);
}

SubjectManager::~SubjectManager(void)
{
}

void SubjectManager::Init(void)
{
	// デフォルトの出現比率と同時出現上限をここで設定する
	// 例: SubjectA を多めに出現させ、同時最大 3 体までに制限する
	SetSpawnWeight(SUBJECT_TYPE::SUBJECT_A, 0.2f);
	SetSpawnWeight(SUBJECT_TYPE::SUBJECT_B, 0.8f);
	SetSpawnWeight(SUBJECT_TYPE::SUBJECT_C, 0.8f); 

	// SubjectA は同時最大 3、SubjectB は制限なし（-1）
	SetMaxConcurrent(SUBJECT_TYPE::SUBJECT_A, 10);
	SetMaxConcurrent(SUBJECT_TYPE::SUBJECT_B, 20);
	SetMaxConcurrent(SUBJECT_TYPE::SUBJECT_C, 20);
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
	case SubjectManager::SUBJECT_TYPE::SUBJECT_C:
		subject = new SubjectC();
		break;
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
	// 最低間隔（XZ平面）。必要に応じて調整してください。
	constexpr float SUBJECT_MIN_SPACING = 500.0f;
	constexpr int MAX_TRIES = 64;

	// generate candidate positions until one is sufficiently far from existing subjects
	VECTOR candidate = VGet(
		GetRandomRange(spawnAreaMin_.x, spawnAreaMax_.x),
		SUBJECT_SPAWN_HEIGHT,
		GetRandomRange(spawnAreaMin_.z, spawnAreaMax_.z));

	auto is_free = [&](const VECTOR& pos) -> bool
	{
		const float minSq = SUBJECT_MIN_SPACING * SUBJECT_MIN_SPACING;
		for (const auto* s : subjects_)
		{
			if (s == nullptr) continue;
			const VECTOR& ep = s->GetTransform().pos;
			const float dx = ep.x - pos.x;
			const float dz = ep.z - pos.z;
			const float distSq = dx * dx + dz * dz;
			if (distSq < minSq)
			{
				return false;
			}
		}
		return true;
	};

	// まずは初回候補をチェック
	if (!is_free(candidate))
	{
		bool found = false;
		for (int i = 0; i < MAX_TRIES; ++i)
		{
			VECTOR p = VGet(
				GetRandomRange(spawnAreaMin_.x, spawnAreaMax_.x),
				SUBJECT_SPAWN_HEIGHT,
				GetRandomRange(spawnAreaMin_.z, spawnAreaMax_.z));
			if (is_free(p))
			{
				candidate = p;
				found = true;
				break;
			}
		}
		// 見つからなければ、最後に試した candidate のまま使用（致し方なし）
		(void)found;
	}

	// 重み付きかつ最大同時出現数を尊重した選択
	std::vector<std::pair<SUBJECT_TYPE, float>> available;
	const int typeCount = static_cast<int>(SUBJECT_TYPE::MAX);
	for (int i = 0; i < typeCount; ++i)
	{
		const SUBJECT_TYPE t = static_cast<SUBJECT_TYPE>(i);
		const float w = (i < (int)spawnWeights_.size()) ? spawnWeights_[i] : 0.0f;
		const int maxC = (i < (int)maxConcurrent_.size()) ? maxConcurrent_[i] : -1;
		if (w <= 0.0f) continue; // 重み0は無効
		if (maxC >= 0 && CountSubjectsOfType(t) >= maxC) continue; // 上限到達で無効
		available.emplace_back(t, w);
	}

	// フォールバック: 全て無効なら従来ランダムを使う
	SUBJECT_TYPE type = SUBJECT_TYPE::SUBJECT_A;
	if (available.empty())
	{
		type = static_cast<SUBJECT_TYPE>(
			GetRand(typeCount - 1));
	}
	else
	{
		// 合計重みを計算して乱択
		float sum = 0.0f;
		for (const auto& p : available) sum += p.second;
		float r = sum * (static_cast<float>(GetRand(10000)) / 10000.0f);
		for (const auto& p : available)
		{
			if (r <= p.second)
			{
				type = p.first;
				break;
			}
			r -= p.second;
		}
	}

	return CreateSubject(type, candidate);
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

void SubjectManager::SetSpawnWeight(SUBJECT_TYPE type, float weight)
{
	const int idx = static_cast<int>(type);
	if (idx < 0 || idx >= static_cast<int>(spawnWeights_.size())) return;
	spawnWeights_[idx] = (weight > 0.0f) ? weight : 0.0f;
}

void SubjectManager::SetMaxConcurrent(SUBJECT_TYPE type, int maxCount)
{
	const int idx = static_cast<int>(type);
	if (idx < 0 || idx >= static_cast<int>(maxConcurrent_.size())) return;
	maxConcurrent_[idx] = maxCount;
}

int SubjectManager::CountSubjectsOfType(SUBJECT_TYPE type) const
{
	int count = 0;
	for (const auto* s : subjects_)
	{
		if (s == nullptr) continue;
		// 型判定は安全に dynamic_cast で行う
		switch (type)
		{
		case SUBJECT_TYPE::SUBJECT_A:
			if (dynamic_cast<const SubjectA*>(s) != nullptr) ++count;
			break;
		case SUBJECT_TYPE::SUBJECT_B:
			if (dynamic_cast<const SubjectB*>(s) != nullptr) ++count;
			break;
		case SUBJECT_TYPE::SUBJECT_C:
			if (dynamic_cast<const SubjectC*>(s) != nullptr) ++count;
			break;
		default:
			break;
		}
	}
	return count;
}