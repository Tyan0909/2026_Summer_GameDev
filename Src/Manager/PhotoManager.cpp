#include "PhotoManager.h"

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
    photos_.clear();
}