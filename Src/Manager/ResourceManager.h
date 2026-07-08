#pragma once
#include <map>
#include <string>
#include "Resource.h"

class ResourceManager
{

public:

	// リソース名
	enum class SRC
	{
		TITLE,
		PUSH_SPACE,
		PIT_FALL_PLANET,
		SPHERE_PLANET,
		PLAYER,

		SUBJECT,
		MAIN_STAGE,
		MAIN_STAGE_FAR,
		SKY_DOME,
		PLAYER_SHADOW,
		ENEMY_RAT,
		ENEMY_ROBOT,
		VIEW_RANGE,

		BGM_TITLE,
		BGM_GAME,
		SE_DECIDE,
		SE_HIT,
		CAMERA_SHUTTER,

		SPIKE_MODEL,
		MINE_MODEL,
	};

	// 静的にインスタンスを生成
	static void CreateInstance(void);

	// 静的インスタンスの取得
	static ResourceManager& GetInstance(void);

	// 初期化
	void Init(void);

	// 解放(シーン切り替え時に一度だけ)
	void Release(void);

	// リソースの完全破棄
	void Destroy(void);

	// リソースのロード
	const Resource& Load(SRC src);

	// リソースの複製ロード(3Dモデル用)
	int LoadModelDuplicate(SRC src);

private:

	// 静的インスタンス
	static ResourceManager* instance_;

	// リソース管理対象
	std::map<SRC, Resource*> resourcesMap_;

	// 読み込み済みリソース
	std::map<SRC, bool> loadedMap_;

	Resource dummy_;

	ResourceManager(void);
	ResourceManager(const ResourceManager& manager) = default;
	~ResourceManager(void) = default;

	// 内部ロード
	Resource& _Load(SRC src);

};
