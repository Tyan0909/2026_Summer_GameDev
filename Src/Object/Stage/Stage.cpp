#include "Stage.h"

// コンストラクタ
Stage::Stage()
{
}

// デストラクタ
Stage::~Stage()
{
}

// 初期化
void Stage::Init()
{
    CreateStage();
}

// 更新（今回は特になし）
void Stage::Update()
{
}

// 描画
void Stage::Draw()
{
    for (const auto& block : blocks)
    {
        VECTOR min = VGet(
            block.position.x - block.size.x / 2,
            block.position.y,
            block.position.z - block.size.z / 2);

        VECTOR max = VGet(
            block.position.x + block.size.x / 2,
            block.position.y + block.size.y,
            block.position.z + block.size.z / 2);

        DrawCube3D(min, max, block.color, block.color, TRUE);
    }
}

// ステージ生成
void Stage::CreateStage()
{
    blocks.clear();

    // 地面
    blocks.push_back({
        VGet(0.0f, -1.0f, 0.0f),
        VGet(100.0f, 1.0f, 100.0f),
        GetColor(50, 50, 50)
        });

    // 建物（商店街っぽく）
    for (int i = -5; i <= 5; i++)
    {
        // 左側の建物
        blocks.push_back({
            VGet(-10.0f, 0.0f, i * 10.0f),
            VGet(5.0f, 10.0f, 5.0f),
            GetColor(100, 100, 255)
            });

        // 右側の建物
        blocks.push_back({
            VGet(10.0f, 0.0f, i * 10.0f),
            VGet(5.0f, 10.0f, 5.0f),
            GetColor(255, 100, 100)
            });
    }

    // 障害物（瓦礫）
    blocks.push_back({
        VGet(0.0f, 0.0f, 10.0f),
        VGet(3.0f, 2.0f, 3.0f),
        GetColor(150, 150, 150)
        });

    blocks.push_back({
        VGet(2.0f, 0.0f, -5.0f),
        VGet(2.0f, 1.5f, 2.0f),
        GetColor(120, 120, 120)
        });
}