#include "InsuranceCamera.h"
#include "../../Charactor/Player/Player.h"

void InsuranceCamera::OnAcquire(Player* player)
{
    player->SetInsuranceCamera(true);
}