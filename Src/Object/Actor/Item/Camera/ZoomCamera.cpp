#include "ZoomCamera.h"
#include "../../Charactor/Player/Player.h"

void ZoomCamera::OnAcquire(Player* player)
{
    player->EnableZoomCamera();
}