#include "SceneBase.h"
#include "../Manager/ResourceManager.h"

SceneBase::SceneBase(void)
{
}

SceneBase::~SceneBase(void)
{
}

void SceneBase::Init(void)
{
	ResourceManager::GetInstance().Init();
}

void SceneBase::Update(void)
{
}

void SceneBase::Draw(void)
{
}

void SceneBase::Release(void)
{
}
