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
	void AddHitCollider(const ColliderBase* hitCollider);

	const std::vector<Subject*>& GetSubjects(void) const;

private:
	std::vector<Subject*> subjects_;
	std::vector<const ColliderBase*> hitColliders_;
};