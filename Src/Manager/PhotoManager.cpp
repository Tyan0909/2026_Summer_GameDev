#include "PhotoManager.h"
#include <DxLib.h>

PhotoManager& PhotoManager::GetInstance()
{
    static PhotoManager instance;
    return instance;
}

void PhotoManager::AddPhoto(const PhotoData& photo)
{
    photos_.push_back(photo);
}

void PhotoManager::Clear()
{
    for (auto& photo : photos_)
    {
        if (photo.graphHandle != -1)
        {
            DeleteGraph(photo.graphHandle);
            photo.graphHandle = -1;
        }
    }

    photos_.clear();
}

const PhotoData* PhotoManager::GetBestPhoto() const
{
    if (photos_.empty())
    {
        return nullptr;
    }

    const PhotoData* best = &photos_[0];

    for (const auto& photo : photos_)
    {
        if (photo.score > best->score)
        {
            best = &photo;
        }
    }

    return best;
}
