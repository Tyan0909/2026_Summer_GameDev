#pragma once
#include <vector>

struct PhotoData
{
    int graphHandle = -1;

    int score = 0;

    int playerIndex = 0;
};

class PhotoManager
{
public:

    static PhotoManager& GetInstance();

    void AddPhoto(const PhotoData& photo);

    const std::vector<PhotoData>& GetPhotos() const
    {
        return photos_;
    }

    void Clear();

private:

    std::vector<PhotoData> photos_;
};