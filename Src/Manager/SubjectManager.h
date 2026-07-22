#pragma once
#include <vector>
#include <DxLib.h>
#include "ResourceManager.h"

class Subject;
class ColliderBase;

class SubjectManager
{
public:


	enum class SUBJECT_TYPE
	{
		SUBJECT_A,
		SUBJECT_B,
		SUBJECT_C,
		SUBJECT_D,
		MAX,
	};

	SubjectManager(void);
	~SubjectManager(void);

	void Init(void);
	void Update(void);
	void Draw(void);
	void Release(void);

	Subject* CreateSubject(SUBJECT_TYPE type, const VECTOR& pos);
	Subject* CreateRandomSubject();
	void SetSpawnArea(const VECTOR& minPos, const VECTOR& maxPos);
	void AddHitCollider(const ColliderBase* hitCollider);
	void SetMoveArea(const VECTOR& minPos, const VECTOR& maxPos);

	const std::vector<Subject*>& GetSubjects(void) const;

	// 追加: Subject を削除する（GameScene から呼ぶ）
	void RemoveSubject(Subject* subject);

	// 新規: 出現比率と最大同時出現数を設定
	void SetSpawnWeight(SUBJECT_TYPE type, float weight);
	void SetMaxConcurrent(SUBJECT_TYPE type, int maxCount);

private:
	static constexpr float SUBJECT_SPAWN_HEIGHT = 1000.0f;

	std::vector<Subject*> subjects_;
	std::vector<const ColliderBase*> hitColliders_;
	VECTOR moveAreaMin_;
	VECTOR moveAreaMax_;

	VECTOR spawnAreaMin_;
	VECTOR spawnAreaMax_;

	float GetRandomRange(float minValue, float maxValue) const;

	// 出現制御
	std::vector<float> spawnWeights_; // 各種の重み（0で出現無効）
	std::vector<int> maxConcurrent_;  // 各種の最大同時出現数（-1で無制限）

	// 型ごとの現在数を数える内部ユーティリティ
	int CountSubjectsOfType(SUBJECT_TYPE type) const;
};