#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "Resource.h"

Resource::Resource(void)
	:
	type_(TYPE::NONE),
	path_(""),
	numX_(-1),
	numY_(-1),
	sizeX_(-1),
	sizeY_(-1),
	handleId_(-1),
	handleIds_(nullptr)
{
}

Resource::Resource(TYPE type, const std::string& path)
	:
	type_(type),
	path_(path),
	numX_(-1),
	numY_(-1),
	sizeX_(-1),
	sizeY_(-1),
	handleId_(-1),
	handleIds_(nullptr)
{
}

Resource::Resource(TYPE type, const std::string& path, int numX, int numY, int sizeX, int sizeY)
	:
	type_(type),
	path_(path),
	numX_(numX),
	numY_(numY),
	sizeX_(sizeX),
	sizeY_(sizeY),
	handleId_(-1),
	handleIds_(nullptr)
{
}

Resource::~Resource(void)
{
}

void Resource::Load(void)
{
	Release();

	switch (type_)
	{
	case Resource::TYPE::IMG:
		// âÊëú
		handleId_ = LoadGraph(path_.c_str());
		break;

	case Resource::TYPE::IMGS:
		// ï°êîâÊëú
		handleIds_ = new int[numX_ * numY_];
		LoadDivGraph(
			path_.c_str(),
			numX_ * numY_,
			numX_, numY_,
			sizeX_, sizeY_,
			&handleIds_[0]);
		break;

	case Resource::TYPE::MODEL:
		// ÉÇÉfÉã
		handleId_ = MV1LoadModel(path_.c_str());
		break;

	case Resource::TYPE::EFFEKSEER:
		handleId_ = LoadEffekseerEffect(path_.c_str());
		break;

	case Resource::TYPE::SOUND:
		// âπê∫
		handleId_ = LoadSoundMem(path_.c_str());
		break;

	case Resource::TYPE::NONE:
	default:
		break;
	}
}

void Resource::Release(void)
{
	switch (type_)
	{
	case Resource::TYPE::IMG:
		if (handleId_ != -1)
		{
			DeleteGraph(handleId_);
			handleId_ = -1;
		}
		break;

	case Resource::TYPE::IMGS:
		if (handleIds_ != nullptr)
		{
			int num = numX_ * numY_;
			for (int i = 0; i < num; i++)
			{
				DeleteGraph(handleIds_[i]);
			}
			delete[] handleIds_;
			handleIds_ = nullptr;
		}
		break;

	case Resource::TYPE::MODEL:
		if (handleId_ != -1)
		{
			MV1DeleteModel(handleId_);
			handleId_ = -1;
		}

		for (auto id : duplicateModelIds_)
		{
			MV1DeleteModel(id - 1);
		}
		duplicateModelIds_.clear();
		break;

	case Resource::TYPE::EFFEKSEER:
		if (handleId_ != -1)
		{
			DeleteEffekseerEffect(handleId_);
			handleId_ = -1;
		}
		break;

	case Resource::TYPE::SOUND:
		if (handleId_ != -1)
		{
			DeleteSoundMem(handleId_);
			handleId_ = -1;
		}
		break;

	case Resource::TYPE::NONE:
	default:
		break;
	}
}

void Resource::CopyHandle(int* imgs) const
{
	if (handleIds_ == nullptr)
	{
		return;
	}

	int num = numX_ * numY_;
	for (int i = 0; i < num; i++)
	{
		imgs[i] = handleIds_[i];
	}
}
