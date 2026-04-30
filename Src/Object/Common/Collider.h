#pragma once

// Collider クラスの完全な宣言を提供
class Collider
{
public:
	// 衝突タイプ
	enum class TYPE
	{
		STAGE,
	};

	// コンストラクタ
	Collider(TYPE type, int modelId);

	// デストラクタ
	~Collider(void);

	// 衝突タイプ
	TYPE type_;	

	// モデルのハンドルID
	int modelId_;
};
