#pragma once
#include "ResourceManager.h"

class SoundManager
{

public:

	static void CreateInstance(void);
	static SoundManager& GetInstance(void);

	void Init(void);
	void Release(void);
	void Destroy(void);

	void PlayBgm(ResourceManager::SRC src, bool isLoop = true);
	void StopBgm(void);

	void PlaySe(ResourceManager::SRC src);

	void SetBgmVolume(int volume);
	void SetSeVolume(int volume);

	int GetBgmVolume(void) const;
	int GetSeVolume(void) const;

private:

	static SoundManager* instance_;

	bool hasCurrentBgm_;
	int currentBgmHandle_;
	int bgmVolume_;
	int seVolume_;

	SoundManager(void);
	SoundManager(const SoundManager& manager) = default;
	~SoundManager(void) = default;

	int ClampVolume(int volume) const;

};