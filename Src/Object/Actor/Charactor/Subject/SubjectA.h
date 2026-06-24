#pragma once
#include "Subject.h"
class Player;

class SubjectA : public Subject
{
public:

	// コンストラクタ
	SubjectA(void);

	// デストラクタ
	~SubjectA(void);

protected:

	void InitPost(void) override;

	VECTOR GetInitPos(void) override;

	ResourceManager::SRC GetModelType() const override;

	void UpdateMove(void) override;

private:

	// プレイヤー取得
	Player* player_;

	

};

