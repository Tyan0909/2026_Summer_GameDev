#include <cmath>
#include <DxLib.h>
#include "../Common/Transform.h"
#include "ColliderModel.h"
#include "ColliderCapsule.h"

ColliderCapsule::ColliderCapsule(
	TAG tag, const Transform* follow,
	const VECTOR& localPosTop, const VECTOR& localPosDown, float radius)
	:
	ColliderBase(SHAPE::CAPSULE, tag, follow),
	localPosTop_(localPosTop),
	localPosDown_(localPosDown),
	radius_(radius)
{
}

ColliderCapsule::~ColliderCapsule(void)
{
}

const VECTOR& ColliderCapsule::GetLocalPosTop(void) const
{
	return localPosTop_;
}

const VECTOR& ColliderCapsule::GetLocalPosDown(void) const
{
	return localPosDown_;
}

void ColliderCapsule::SetLocalPosTop(const VECTOR& pos)
{
	localPosTop_ = pos;
}

void ColliderCapsule::SetLocalPosDown(const VECTOR& pos)
{
	localPosDown_ = pos;
}

VECTOR ColliderCapsule::GetPosTop(void) const
{
	return GetRotPos(localPosTop_);
}

VECTOR ColliderCapsule::GetPosDown(void) const
{
	return GetRotPos(localPosDown_);
}

float ColliderCapsule::GetRadius(void) const
{
	return radius_;
}

void ColliderCapsule::SetRadius(float radius)
{
	radius_ = radius;
}

float ColliderCapsule::GetHeight(void) const
{
	return localPosTop_.y;
}

VECTOR ColliderCapsule::GetCenter(void) const
{
	VECTOR top = GetPosTop();
	VECTOR down = GetPosDown();
	VECTOR diff = VSub(top, down);
	return VAdd(down, VScale(diff, 0.5f));
}

VECTOR ColliderCapsule::GetPosPushBackAlongNormal(
	const MV1_COLL_RESULT_POLY& hitColPoly,
	int maxTryCnt, float pushDistance) const
{
	Transform tmpTransform = *follow_;
	ColliderCapsule tmpCapsule = *this;
	tmpCapsule.SetFollow(&tmpTransform);

	int tryCnt = 0;
	while (tryCnt < maxTryCnt)
	{
		if (!HitCheck_Capsule_Triangle(
			tmpCapsule.GetPosTop(), tmpCapsule.GetPosDown(),
			tmpCapsule.GetRadius(),
			hitColPoly.Position[0], hitColPoly.Position[1],
			hitColPoly.Position[2]))
		{
			break;
		}

		tmpTransform.pos =
			VAdd(tmpTransform.pos, VScale(hitColPoly.Normal, pushDistance));

		tryCnt++;
	}

	return tmpTransform.pos;
}

void ColliderCapsule::PushBackAlongNormal(
	const ColliderModel* colliderModel, Transform& transform,
	int maxTryCnt, float pushDistance,
	bool isExclude, bool isTarget, float normalYMax) const
{
	Transform tmpTransform = transform;
	ColliderCapsule tmpCapsule = *this;
	tmpCapsule.SetFollow(&tmpTransform);

	for (int solveCnt = 0; solveCnt < maxTryCnt; solveCnt++)
	{
		auto hits = MV1CollCheck_Capsule(
			colliderModel->GetFollow()->modelId, -1,
			tmpCapsule.GetPosTop(), tmpCapsule.GetPosDown(), tmpCapsule.GetRadius());

		bool isPushed = false;

		for (int i = 0; i < hits.HitNum; i++)
		{
			const auto& hitPoly = hits.Dim[i];

			if (isExclude && colliderModel->IsExcludeFrame(hitPoly.FrameIndex))
			{
				continue;
			}

			if (isTarget && !colliderModel->IsTargetFrame(hitPoly.FrameIndex))
			{
				continue;
			}
			if (std::fabs(hitPoly.Normal.y) > normalYMax)
			{
				continue;
			}

			if (!HitCheck_Capsule_Triangle(
				tmpCapsule.GetPosTop(), tmpCapsule.GetPosDown(), tmpCapsule.GetRadius(),
				hitPoly.Position[0], hitPoly.Position[1], hitPoly.Position[2]))
			{
				continue;
			}

			tmpTransform.pos =
				VAdd(tmpTransform.pos, VScale(hitPoly.Normal, pushDistance));

			isPushed = true;
		}

		MV1CollResultPolyDimTerminate(hits);

		if (!isPushed)
		{
			break;
		}
	}

	transform.pos = tmpTransform.pos;
}

bool ColliderCapsule::IsHit(const ColliderModel* colliderModel, bool isExclude, bool isTarget) const
{
	bool ret = false;

	auto hits = MV1CollCheck_Capsule(
		colliderModel->GetFollow()->modelId, -1,
		GetPosTop(), GetPosDown(), GetRadius());

	for (int i = 0; i < hits.HitNum; i++)
	{
		auto hitPoly = hits.Dim[i];

		if (isExclude && colliderModel->IsExcludeFrame(hitPoly.FrameIndex))
		{
			continue;
		}

		if (isTarget && !colliderModel->IsTargetFrame(hitPoly.FrameIndex))
		{
			continue;
		}

		ret = true;
		break;
	}

	MV1CollResultPolyDimTerminate(hits);

	return ret;
}

void ColliderCapsule::DrawDebug(int color)
{
	/*VECTOR pos1 = GetPosTop();
	DrawSphere3D(pos1, radius_, 5, color, color, false);

	VECTOR pos2 = GetPosDown();
	DrawSphere3D(pos2, radius_, 5, color, color, false);
	VECTOR dir;
	VECTOR s;
	VECTOR e;

	dir = follow_->GetRight();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, color);

	dir = follow_->GetLeft();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, color);

	dir = follow_->GetForward();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, color);

	dir = follow_->GetBack();
	s = VAdd(pos1, VScale(dir, radius_));
	e = VAdd(pos2, VScale(dir, radius_));
	DrawLine3D(s, e, color);

	DrawSphere3D(GetCenter(), 5.0f, 10, color, color, true);*/
}
