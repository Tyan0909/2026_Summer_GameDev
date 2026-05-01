#include "../Common/Transform.h"
#include "ColliderBase.h"
#include "ColliderSphere.h"

ColliderSphere::ColliderSphere(
	TAG tag, const Transform* follow,
	const VECTOR& localPos, float radius)
	:
	ColliderBase(SHAPE::SPHERE, tag, follow),
	localPos_(localPos),
	radius_(radius)
{
}

ColliderSphere::~ColliderSphere(void)
{
}

const VECTOR& ColliderSphere::GetLocalPos(void) const
{
	return localPos_;
}

void ColliderSphere::SetLocalPos(const VECTOR& localPos)
{
	localPos_ = localPos;
}

VECTOR ColliderSphere::GetPos(void) const
{
	return GetRotPos(localPos_);
}

float ColliderSphere::GetRadius(void) const
{
	return radius_;
}

void ColliderSphere::SetRadius(float radius)
{
	radius_ = radius;
}

VECTOR ColliderSphere::GetPosPushBackAlongNormal(const MV1_COLL_RESULT_POLY& hitColPoly, int maxTryCnt, float pushDistance) const
{
	// コピー生成
	Transform tmpTransform = *follow_;
	ColliderSphere tmpSphere = *this;
	tmpSphere.SetFollow(&tmpTransform);

	// 衝突補正処理
	int tryCnt = 0;
	while (tryCnt < maxTryCnt)
	{
		// カプセルと三角形の当たり判定
		if (!HitCheck_Sphere_Triangle(
			tmpSphere.GetPos(), tmpSphere.GetRadius(),
			hitColPoly.Position[0], hitColPoly.Position[1], hitColPoly.Position[2]))
		{
			break;
		}

		// 衝突していたら法線方向に押し戻し
		tmpTransform.pos =
			VAdd(tmpTransform.pos, VScale(hitColPoly.Normal, pushDistance));

		tryCnt++;
	}
	return tmpTransform.pos;
}

void ColliderSphere::DrawDebug(int color)
{
	DrawSphere3D(
		GetPos(), radius_, 16, color, color, false);
}
