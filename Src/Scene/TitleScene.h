#pragma once

#include "SceneBase.h"
#include <vector>

class Grid;

class TitleScene : public SceneBase

{

public:

	// コンストラクタ
	TitleScene(void);

	// デストラクタ
	~TitleScene(void) override;

	// ライフサイクル
	void Init(void) override;
	void Update(void) override;
	void Draw(void) override;
	void Release(void) override;

private:

	// 基本リソース
	Grid* debugGrid_;
	int logoHandle_;      // ロゴ画像
	int pointerHandle_;   // ポインタ画像
	int shutterSE_;       // シャッター音
	int pressHandle_;     // press space

	// ホラー演出用リソース
	int ambientSE_;       // 環境音ループ
	int whisperSE_;       // ささやき音
	int vignetteHandle_;  // ビネット画像
	int fogHandle_;       // 霧テクスチャ
	int bloodHandle_;     // （未使用だが残す可能性あり）
	int noiseHandle_;     // ノイズテクスチャ

	// 状態
	float shutterScale_;
	bool isShutter_;
	int flashAlpha_;
	int fadeAlpha_;
	float pointerAnim_;
	int logoAlpha_;
	VECTOR pointerPos_;
	VECTOR targetPos_;

	// ホラーパラメータ
	int whisperTimer_;        // 次のささやきまでのフレーム
	int nextWhisperDelay_;    // 次のささやきまでのランダム遅延（フレーム）
	int vignetteAlpha_;
	int fogAlpha_;
	int bloodAlpha_;
	float logoJitterX_;
	float logoJitterY_;
	bool forceRedText_;       // テキストを赤くするフラグ（短時間）

	// ノイズ / グリッチ（シェーダー風エフェクト用）
	int noiseAlpha_;
	float noiseOffsetX_;
	float noiseOffsetY_;
	float noiseTime_;        // 時間カウンタ
	float noiseSpeedX_;
	float noiseSpeedY_;

	// スキャンライン・グリッチ
	float scanlineAlpha_;
	bool isGlitching_;
	int glitchTimer_;
	int glitchDuration_;

};


