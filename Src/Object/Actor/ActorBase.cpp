#include "../Manager/ResourceManager.h"
#include "../Manager/SceneManager.h"
#include "ActorBase.h"

ActorBase::ActorBase(void)
	: resMng_(ResourceManager::GetInstance()),
	scnMng_(SceneManager::GetInstance())
{
}

ActorBase::~ActorBase(void)
{
	transform_.Release();
}

const Transform& ActorBase::GetTransform(void) const
{
	return transform_;
}
