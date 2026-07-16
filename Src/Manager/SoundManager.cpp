#include <DxLib.h>
#include "SoundManager.h"

SoundManager* SoundManager::instance_ = nullptr;

void SoundManager::CreateInstance(void)
{
	if (instance_ == nullptr)
	{
		instance_ = new SoundManager();
	}
	instance_->Init();
}

SoundManager& SoundManager::GetInstance(void)
{
	return *instance_;
}

void SoundManager::Init(void)
{
	bgmVolume_ = 255;
	seVolume_ = 255;
	hasCurrentBgm_ = false;
	currentBgmHandle_ = -1;
}

void SoundManager::Release(void)
{
	StopBgm();
}

void SoundManager::Destroy(void)
{
	if (instance_ == nullptr)
	{
		return;
	}

	Release();

	delete instance_;
	instance_ = nullptr;
}

void SoundManager::PlayBgm(ResourceManager::SRC src, bool isLoop)
{
	const Resource& res = ResourceManager::GetInstance().Load(src);
	if (res.type_ != Resource::TYPE::SOUND || res.handleId_ == -1)
	{
		return;
	}

	if (hasCurrentBgm_ && currentBgmHandle_ == res.handleId_)
	{
		return;
	}

	StopBgm();

	ChangeVolumeSoundMem(bgmVolume_, res.handleId_);
	PlaySoundMem(res.handleId_, isLoop ? DX_PLAYTYPE_LOOP : DX_PLAYTYPE_BACK, TRUE);

	currentBgmHandle_ = res.handleId_;
	hasCurrentBgm_ = true;
}

void SoundManager::StopBgm(void)
{
	if (!hasCurrentBgm_)
	{
		return;
	}

	if (currentBgmHandle_ != -1)
	{
		StopSoundMem(currentBgmHandle_);
	}

	currentBgmHandle_ = -1;
	hasCurrentBgm_ = false;
}

void SoundManager::PlaySe(ResourceManager::SRC src)
{
	const Resource& res = ResourceManager::GetInstance().Load(src);
	/*printfDx("SE Handle = %d\n", res.handleId_);*/
	if (res.type_ != Resource::TYPE::SOUND || res.handleId_ == -1)
	{
		return;
	}

	ChangeVolumeSoundMem(seVolume_, res.handleId_);
	PlaySoundMem(res.handleId_, DX_PLAYTYPE_BACK, TRUE);
}

void SoundManager::SetBgmVolume(int volume)
{
	bgmVolume_ = ClampVolume(volume);

	if (hasCurrentBgm_ && currentBgmHandle_ != -1)
	{
		ChangeVolumeSoundMem(bgmVolume_, currentBgmHandle_);
	}
}

void SoundManager::SetSeVolume(int volume)
{
	seVolume_ = ClampVolume(volume);
}

int SoundManager::GetBgmVolume(void) const
{
	return bgmVolume_;
}

int SoundManager::GetSeVolume(void) const
{
	return seVolume_;
}

SoundManager::SoundManager(void)
	:
	hasCurrentBgm_(false),
	currentBgmHandle_(-1),
	bgmVolume_(255),
	seVolume_(255)
{
}

int SoundManager::ClampVolume(int volume) const
{
	if (volume < 0)
	{
		return 0;
	}

	if (volume > 255)
	{
		return 255;
	}

	return volume;
}