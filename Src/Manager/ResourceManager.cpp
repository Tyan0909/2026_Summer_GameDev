#include <DxLib.h>
#include "../Application.h"
#include "Resource.h"
#include "ResourceManager.h"

ResourceManager* ResourceManager::instance_ = nullptr;

void ResourceManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new ResourceManager();
	}
	instance_->Init();
}

ResourceManager& ResourceManager::GetInstance(void)
{
	return *instance_;
}

void ResourceManager::Init(void)
{
	using RES = Resource;
	using RES_T = RES::TYPE;
	static std::string PATH_IMG = Application::PATH_IMAGE;
	static std::string PATH_MDL = Application::PATH_MODEL;
	static std::string PATH_EFF = Application::PATH_EFFECT;

	Resource* res;

	// ステージ(近景フル)
	res = new RES(RES_T::MODEL, PATH_MDL +
		"Stage/GameObject.mv1");
	resourcesMap_.emplace(SRC::MAIN_STAGE, res);

	// ステージ(遠景ローポリ)
	res = new RES(RES_T::MODEL, PATH_MDL +
		"Stage/GameObject_Low.mv1");
	resourcesMap_.emplace(SRC::MAIN_STAGE_FAR, res);

	// プレイヤー
	res = new RES(RES_T::MODEL, PATH_MDL +
		"Player/Player_1.mv1");
	resourcesMap_.emplace(SRC::PLAYER, res);

	// サブジェクト
	res = new RES(RES_T::MODEL, PATH_MDL +
		"Subject/Subject.mv1");
	resourcesMap_.emplace(SRC::SUBJECT, res);
}

void ResourceManager::Release(void)
{
	for (auto& p : loadedMap_)
	{
		p.second.Release();
	}

	loadedMap_.clear();
}

void ResourceManager::Destroy(void)
{
	Release();
	for (auto& res : resourcesMap_)
	{
		res.second->Release();
		delete res.second;
	}
	resourcesMap_.clear();
	delete instance_;
}

const Resource& ResourceManager::Load(SRC src)
{
	const Resource& res = _Load(src);
	if (res.type_ == Resource::TYPE::NONE)
	{
		return dummy_;
	}
	return res;
}

int ResourceManager::LoadModelDuplicate(SRC src)
{
	Resource& res = _Load(src);
	if (res.type_ == Resource::TYPE::NONE)
	{
		return -1;
	}

	int duId = MV1DuplicateModel(res.handleId_);
	res.duplicateModelIds_.push_back(duId);

	return duId;
}

ResourceManager::ResourceManager(void)
{
}

Resource& ResourceManager::_Load(SRC src)
{
	const auto& lPair = loadedMap_.find(src);
	if (lPair != loadedMap_.end())
	{
		return *resourcesMap_.find(src)->second;
	}

	const auto& rPair = resourcesMap_.find(src);
	if (rPair == resourcesMap_.end())
	{
		return dummy_;
	}

	rPair->second->Load();
	loadedMap_.emplace(src, *rPair->second);

	return *rPair->second;
}
