#pragma once
#include <DxLib.h>
#include "ColliderBase.h"
class Transform;
class ColliderModel;

class ColliderLine : public ColliderBase
{

public:

	// コンストラクタ
	ColliderLine(
		TAG tag, const Transform* follow,
		const VECTOR& localPosStart, const VECTOR& localPosEnd);

	// デストラクタ
	~ColliderLine(void) override;

	// ローカル座標での設定
	void SetLocalPosStart(const VECTOR& pos);
	void SetLocalPosEnd(const VECTOR& pos);

	// ローカル座標の取得
	const VECTOR& GetLocalPosStart(void) const;
	const VECTOR& GetLocalPosEnd(void) const;

	// ワールド座標の取得
	VECTOR GetPosStart(void) const;
	VECTOR GetPosEnd(void) const;

	// 指定された回数と距離で三角形の法線方向に押し戻した座標を取得
	VECTOR GetPosPushBackAlongNormal(
		const MV1_COLL_RESULT_POLY& hitColPoly,
		int maxTryCnt,
		float pushDistance) const override
	{
		return VECTOR();
	}

	// 衝突したポリゴンの上に押し戻す
	bool PushBackUp(
		const ColliderModel* colliderModel, Transform& transform,
		float pushDistance,
		bool isExclude = false, bool isTarget = false) const;

protected:

	// デバッグ用描画
	void DrawDebug(int color) override;

private:

	// デバッグ表示の球体半径
	static constexpr float RADIUS = 5.0f;

	// デバッグ表示の球体ポリゴン分割数
	static constexpr int DIV_NUM = 6;

	// 線分の開始座標(ローカル)
	VECTOR localPosStart_;

	// 線分の終了座標(ローカル)
	VECTOR localPosEnd_;

};