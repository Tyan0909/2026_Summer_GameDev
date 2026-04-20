#pragma once
#include <DxLib.h>
class Transform;

class ColliderBase
{

public:

	// 形状
	enum class SHAPE
	{
		NONE,
		LINE,
		SPHERE,
		CAPSULE,
		MODEL,
	};

	// 衝突種別
	enum class TAG
	{
		STAGE,
		PLAYER,
		CAMERA,
		ENEMY,
		VIEW_RANGE,
	};

	// コンストラクタ
	ColliderBase(SHAPE shape, TAG tag, const Transform* follow);

	// デストラクタ
	virtual ~ColliderBase(void);

	// 描画
	void Draw(void);

	// 追従先の取得
	const Transform* GetFollow(void) const { return follow_; };

	// 追従先の再設定
	void SetFollow(Transform* follow);

	// 形状
	SHAPE GetShape(void) const { return shape_; }

	// 衝突種別
	TAG GetTag(void) const { return tag_; }

	// 指定された回数と距離で三角形の法線方向に押し戻した座標を取得
	virtual VECTOR GetPosPushBackAlongNormal(
		const MV1_COLL_RESULT_POLY& hitColPoly,
		int maxTryCnt,
		float pushDistance) const = 0;

protected:

	// デバッグ表示の色
	static constexpr int COLOR_VALID = 0xff0000;
	static constexpr int COLOR_INVALID = 0xaaaaaa;

	// 形状
	SHAPE shape_;

	// 衝突種別
	TAG tag_;

	// 追従先
	const Transform* follow_;

	// 有効フラグ
	bool isValid_;

	// ローカル座標をワールド座標に変換
	VECTOR GetRotPos(const VECTOR& localPos) const;

	// デバッグ用描画
	virtual void DrawDebug(int color) = 0;

};