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

	static constexpr VECTOR INIT_POS = { 0.0f, 1000.0f, 0.0f };

protected:

	void InitPost(void) override;
	void InitLoad(void) override;
	void InitTransform(void) override;

	VECTOR GetInitPos(void) override;

	ResourceManager::SRC GetModelType() const override;

	void UpdateMove(void) override;

	// 飛行体は重力無視でホバリングするため ApplyGravity をオーバーライド
	void ApplyGravity(void) override;

private:

	// プレイヤー取得（未使用だが宣言は残す）
	Player* player_;

	// ホバリング用パラメータ
	float hoverBaseY_;
	float hoverAmplitude_;
	float hoverSpeed_;
	float hoverAngle_;

};