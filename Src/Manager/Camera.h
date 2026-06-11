#pragma once
#include <DxLib.h>

class Camera
{

public:

	// カメラの初期座標
	static constexpr VECTOR DEFAULT_POS = {0.0f,0.0f,0.0f};

	// カメラの初期角度
	static constexpr VECTOR DEFAULT_ANGLES = {
		30.f * DX_PI_F / 180.f, 0.f, 0.f
	};

	// カメラのクリップ範囲
	static constexpr float VIEW_NEAR = 10.0f;		// ニアクリップ
	static constexpr float VIEW_FAR = 5000.0f;	// ファークリップ

	// カメラモード
	enum class MODE
	{
		NONE,
		FIXED_POINT,	// 定点カメラ
		FREE,			// 自由カメラ
	};

	// コンストラクタ
	Camera(void);

	// デストラクタ
	~Camera(void);

	// 初期化
	void Init(void);

	// 更新
	void Update(void);

	// 描画前のカメラ設定
	void SetBeforeDraw(void);
	void SetBeforeDrawFixedPoint(void);
	void SetBeforeDrawFree(void);

	// デバッグ用描画
	void DrawDebug(void);

	// 解放
	void Release(void);

	// 座標の取得
	const VECTOR& GetPos(void) const;

	// 角度の取得
	const VECTOR& GetAngles(void) const;

	// カメラモードの変更
	void ChangeMode(MODE mode);
	void SetPos(const VECTOR& pos);
	void SetAngles(const VECTOR& angles);

	void MoveXYZDirection(void);

private:

	// カメラモード
	MODE mode_;

	// カメラの位置
	VECTOR pos_;

	// カメラの角度
	VECTOR angles_;

};