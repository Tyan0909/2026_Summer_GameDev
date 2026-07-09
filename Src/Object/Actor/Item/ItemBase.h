#pragma once

class Player;

class ItemBase
{
public:
    virtual ~ItemBase() = default;

    // プレイヤーがアイテムを取得したとき
    virtual void OnAcquire(Player* player) = 0;
};