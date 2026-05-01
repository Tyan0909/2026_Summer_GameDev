#pragma once
#include <map>
#include <vector>
#include <DxLib.h>
#include <memory>
#include "../../ActorBase.h"

class AnimationController;
class Collider;
class Capsule;

class Player : public ActorBase
{
public:

	// 定数宣言
	// 速度

	// 回転

	// ジャンプ力

	// ジャンプ受付時間

	// モデルの状態
	enum class STATE
	{
		IDLE,	// 待機
		WALK,	// 歩き
		TAKE,	// 写真撮影
		JUMP,	// ジャンプ
	};

	// アニメーションの状態
	enum class ANIM_TYPE
	{
		IDLE,	// 待機
		WALK,	// 歩き
		TAKE,	// 写真撮影
		JUMP,	// ジャンプ
	};

	// コンストラクタ
	Player(void);

	// デストラクタ
	~Player(void);

	// 基本処理
	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;

	// 衝突判定に必要なコライダー制御
	void AddCollider(Collider* collider);
	void ClearCollider(void);

	// 衝突用カプセルを取得
	const Capsule* GetCapsule(void) const;

private:

	// アニメーション制御
	std::unique_ptr<AnimationController> animCtrl_;

	// 状態遷移
	STATE state_;

	// 移動スピード
	float speed_;

	// 移動方向
	VECTOR moveDir_;

	// 移動量
	VECTOR movePow_;

	// 移動後の位置
	VECTOR movedPos_;

	// 回転
	Quaternion playerRotY_;
	Quaternion goalQuaRot_;
	float stepRotTime_;

	// ジャンプ力
	VECTOR jumpPow_;

	// ジャンプ受付時間
	float stepJump_;

	// ジャンプ判定
	bool isJump_;

	// 衝突判定に必要なコライダー
	std::vector<Collider*> colliders_;
	std::unique_ptr<Capsule> capsule_;

	// 衝突判定
	VECTOR gravHitPosDown_;
	VECTOR gravHitPosUp_;

	// 丸影
	int imgShadow_;

	// アニメーションの初期化
	void InitAnimation(void);

	// 状態遷移
	void ChangeState(STATE state);
	void ChangeStateNone(void);		//	状態遷移なし
	void ChangeStateIdle(void);		//	待機状態
	void ChangeStateWalk(void);		//	歩き状態
	void ChangeStateTake(void);		//	写真撮影状態
	void ChangeStateJump(void);		//	ジャンプ状態

	// 更新
	void UpdateNone(void);		//	状態遷移なし
	void UpdateIdle(void);		//	待機状態
	void UpdateWalk(void);		//	歩き状態
	void UpdateTake(void);		//	写真撮影状態
	void UpdateJump(void);		//	ジャンプ状態

	// 描画
	void DrawShadow(void);		//	丸影の描画

	// 操作系
	void ProcessMove(void);		//	移動処理
	void ProcessJump(void);		//	ジャンプ処理

	// 回転
	void SetGoalRotateion(double rotRad);
	void Rotate(void);

	// 衝突判定
	void Collision(void);
	void CollisionGravity(void);	// 重力による衝突判定
	void CollisionCapsule(void);	// カプセルによる衝突判定

	// 移動量の計算
	void CalcGravityPow(void);	// 重力による移動量の計算
	
	// 着地終了
	bool IsEndlanding(void);
};