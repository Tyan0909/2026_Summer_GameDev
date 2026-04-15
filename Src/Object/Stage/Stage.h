#pragma once
#include <DxLib.h>
#include <vector>

// シンプルなブロック構造体
struct StageBlock
{
    VECTOR position;   // 座標
    VECTOR size;       // サイズ
    unsigned int color; // 色
};

class Stage
{
public:
    Stage();
    ~Stage();

    void Init();     // 初期化
    void Update();   // 更新
    void Draw();     // 描画

private:
    std::vector<StageBlock> blocks;

    // 内部処理
    void CreateStage();
};