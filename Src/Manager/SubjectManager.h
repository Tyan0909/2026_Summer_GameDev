#pragma once
#include <vector>
#include <DxLib.h>
#include "ResourceManager.h"

class Subject;
class ColliderBase;

class SubjectManager
{
public:
	SubjectManager(void);
	~SubjectManager(void);

	void Init(void);
	void Update(void);
	void Draw(void);
	void Release(void);

	Subject* CreateSubject(ResourceManager::SRC modelSrc, const VECTOR& pos);
	Subject* CreateRandomSubject(ResourceManager::SRC modelSrc);
	void AddHitCollider(const ColliderBase* hitCollider);
	void SetMoveArea(const VECTOR& minPos, const VECTOR& maxPos);

	const std::vector<Subject*>& GetSubjects(void) const;

	// í«â¡: Subject ÇçÌèúÇ∑ÇÈÅiGameScene Ç©ÇÁåƒÇ‘Åj
	void RemoveSubject(Subject* subject);

private:
	static constexpr float SUBJECT_SPAWN_HEIGHT = 1000.0f;

	std::vector<Subject*> subjects_;
	std::vector<const ColliderBase*> hitColliders_;
	VECTOR moveAreaMin_;
	VECTOR moveAreaMax_;

	float GetRandomRange(float minValue, float maxValue) const;
};