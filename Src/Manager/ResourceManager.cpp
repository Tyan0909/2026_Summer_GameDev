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
	if (!resourcesMap_.empty())
	{
		return;
	}

	using RES = Resource;
	using RES_T = RES::TYPE;
	static std::string PATH_IMG = Application::PATH_IMAGE;
	static std::string PATH_MDL = Application::PATH_MODEL;
	static std::string PATH_EFF = Application::PATH_EFFECT;
	static std::string PATH_SND = Application::PATH_SOUND;

	Resource* res;

	// ステージ(近景付き)
	res = new RES(RES_T::MODEL, PATH_MDL +
		"Stage/GameObject.mv1");
	resourcesMap_.emplace(SRC::MAIN_STAGE, res);

	// ステージ(簡易ローポリ)
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

	// タイトルロゴ
	res = new RES(RES_T::IMG, PATH_IMG +
		"Title/Title Logo.png");
	resourcesMap_.emplace(SRC::TITLE, res);

	// PRESS SPACE
	res = new RES(RES_T::IMG, PATH_IMG +
		"Title/press_space.png");
	resourcesMap_.emplace(SRC::PUSH_SPACE, res);

	// スカイドーム
	res = new RES(RES_T::MODEL, PATH_MDL +
		"SkyDome/SkyDome.mv1");
	resourcesMap_.emplace(SRC::SKY_DOME, res);

	//罠
	res = new RES(
		RES_T::MODEL,
		PATH_MDL + "Trap/Spike.mv1");
	resourcesMap_.emplace(SRC::SPIKE_MODEL,res);

	//地雷
	res = new RES(
		RES_T::MODEL,
		PATH_MDL + "Trap/Mine.mv1");
	resourcesMap_.emplace(SRC::MINE_MODEL,res);

	// BGM: ゲーム
	res = new RES(RES_T::SOUND, PATH_SND +
		"Bgm/game.wav");
	resourcesMap_.emplace(SRC::BGM_GAME, res);

	// SE: 決定
	res = new RES(RES_T::SOUND, PATH_SND +
		"Se/decide.wav");
	resourcesMap_.emplace(SRC::SE_DECIDE, res);

	// SE: ヒット
	res = new RES(RES_T::SOUND, PATH_SND +
		"Se/hit.wav");
	resourcesMap_.emplace(SRC::SE_HIT, res);

	// BGM: タイトル
	res = new RES(
		RES_T::SOUND,
		PATH_SND + "Bgm/Title.mp3");
	resourcesMap_.emplace(
		SRC::BGM_TITLE,
		res);

	// BGM: ゲーム
	res = new RES(
		RES_T::SOUND,
		PATH_SND + "Bgm/Game.wav");
	resourcesMap_.emplace(
		SRC::BGM_GAME,
		res);

	// SE: カメラシャッター
	res = new RES(
		RES_T::SOUND,
		PATH_SND + "shutter.mp3");

	resourcesMap_.emplace(
		SRC::CAMERA_SHUTTER,
		res);

	// SE: GAME CLEAR
	res = new RES(
		RES_T::SOUND,
		PATH_SND + "Result/GameClear.mp3");

	resourcesMap_.emplace(
		SRC::GAMECLEAR_SE,
		res);

	//SE:ロードディング
	res = new RES(
		RES_T::SOUND,
		PATH_SND + "Load/loading.wav");

	resourcesMap_.emplace(
		SRC::LOADING_SE,
		res);
}

void ResourceManager::Release(void)
{
	for (auto& p : loadedMap_)
	{
		if (!p.second)
		{
			continue;
		}

		auto resPair = resourcesMap_.find(p.first);
		if (resPair == resourcesMap_.end())
		{
			continue;
		}

		resPair->second->Release();
	}

	loadedMap_.clear();
}

void ResourceManager::Destroy(void)
{
	if (instance_ == nullptr)
	{
		return;
	}

	Release();

	for (auto& res : resourcesMap_)
	{
		delete res.second;
	}
	resourcesMap_.clear();

	delete instance_;
	instance_ = nullptr;
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
	if (res.type_ != Resource::TYPE::MODEL)
	{
		return -1;
	}

	int duId = MV1DuplicateModel(res.handleId_);

	if (duId != -1)
	{
		res.duplicateModelIds_.push_back(duId);
	}

	return duId;
}

ResourceManager::ResourceManager(void)
{
}

Resource& ResourceManager::_Load(SRC src)
{
	const auto lPair = loadedMap_.find(src);
	if (lPair != loadedMap_.end() && lPair->second)
	{
		return *resourcesMap_.find(src)->second;
	}

	const auto rPair = resourcesMap_.find(src);
	if (rPair == resourcesMap_.end())
	{
		return dummy_;
	}

	rPair->second->Load();
	loadedMap_[src] = true;

	return *rPair->second;
}
