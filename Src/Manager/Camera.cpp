#include "../Utility/AsoUtility.h"
#include "../Manager/InputManager.h"
#include "Camera.h"

Camera::Camera(void)
{
	// DxLibの初期設定では、
	// カメラの位置が x = 320.0f, y = 240.0f, z = (画面のサイズによって変化)、
	// 注視点の位置は x = 320.0f, y = 240.0f, z = 1.0f
	// カメラの上方向は x = 0.0f, y = 1.0f, z = 0.0f
	// 右上位置からZ軸のプラス方向を見るようなカメラ
}

Camera::~Camera(void)
{
}

void Camera::Init(void)
{
	// カメラ初期位置
	pos_ = DEFAULT_POS;

	// カメラの初期角度
	angles_ = DEFAULT_ANGLES;
}

void Camera::Update(void)
{
}

void Camera::SetBeforeDraw(void)
{
	// クリップ距離を設定する
	SetCameraNearFar(VIEW_NEAR, VIEW_FAR);

	switch (mode_)
	{

	case MODE::FIXED_POINT:
		SetBeforeDrawFixedPoint();
		break;

	case MODE::FREE:
		// 移動方向移動
		MoveXYZDirection();

		// カメラ固定時のみ使用
		/*SetBeforeDrawFree();*/
		break;
	}

	// カメラの設定
	SetCameraPositionAndAngle(
		pos_,
		angles_.x,
		angles_.y,
		angles_.z
	);
}

void Camera::SetBeforeDrawFixedPoint(void)
{
	// 何も処理しない
}

// カメラの方向移動
void Camera::MoveXYZDirection(void)
{
	auto& ins = InputManager::GetInstance();
	// 矢印キーでカメラの角度を変える
	float rotPow = 1.0f * DX_PI_F / 180.0f;
	if (ins.IsNew(KEY_INPUT_DOWN)) { angles_.x += rotPow; }
	if (ins.IsNew(KEY_INPUT_UP)) { angles_.x -= rotPow; }
	if (ins.IsNew(KEY_INPUT_RIGHT)) { angles_.y += rotPow; }
	if (ins.IsNew(KEY_INPUT_LEFT)) { angles_.y -= rotPow; }
	// WASDでカメラを移動させる
	const float movePow = 100.0f;
	VECTOR dir = AsoUtility::VECTOR_ZERO;
	if (ins.IsNew(KEY_INPUT_I)) { dir = { 0.0f, 0.0f, 1.0f }; }
	if (ins.IsNew(KEY_INPUT_J)) { dir = { -1.0f, 0.0f, 0.0f }; }
	if (ins.IsNew(KEY_INPUT_K)) { dir = { 0.0f, 0.0f, -1.0f }; }
	if (ins.IsNew(KEY_INPUT_L)) { dir = { 1.0f, 0.0f, 0.0f }; }
	if (!AsoUtility::EqualsVZero(dir))
	{
		// XYZの回転行列
		// XZ平面移動にする場合は、XZの回転を考慮しないようにする
		MATRIX mat = MGetIdent();
		mat = MMult(mat, MGetRotX(angles_.x));
		mat = MMult(mat, MGetRotY(angles_.y));
		//mat = MMult(mat, MGetRotZ(angles_.z));
		// 回転行列を使用して、ベクトルを回転させる
		VECTOR moveDir = VTransform(dir, mat);
		// 方向×スピードで移動量を作って、座標に足して移動
		pos_ = VAdd(pos_, VScale(moveDir, movePow));
	}
}

void Camera::SetBeforeDrawFree(void)
{
	// InputManagerのインスタンス取得
	auto& input = InputManager::GetInstance();

	// カメラ角度変更
	float rotPow = 1.f * DX_PI_F / 180.f;
	if (input.IsNew(KEY_INPUT_DOWN)) { angles_.x += rotPow; }
	if (input.IsNew(KEY_INPUT_UP)) { angles_.x -= rotPow; }
	if (input.IsNew(KEY_INPUT_LEFT)) { angles_.y += rotPow; }
	if (input.IsNew(KEY_INPUT_RIGHT)) { angles_.y -= rotPow; }

	// カメラ位置変更
	float movePow = 3.f;
	if (input.IsNew(KEY_INPUT_W)) { pos_.z += movePow; }
	if (input.IsNew(KEY_INPUT_A)) { pos_.x -= movePow; }
	if (input.IsNew(KEY_INPUT_S)) { pos_.z -= movePow; }
	if (input.IsNew(KEY_INPUT_D)) { pos_.x += movePow; }

	// 高さ変更
	if (input.IsNew(KEY_INPUT_Q)) { pos_.y += movePow; }
	if (input.IsNew(KEY_INPUT_E)) { pos_.y -= movePow; }

}

void Camera::DrawDebug(void)
{
	//#ifdef DEBUG
		// デバッグ用描画
	DrawFormatString(0, 600, GetColor(255, 0, 0),
		"Camera Pos:(%.1f, %.1f, %.1f)", pos_.x, pos_.y, pos_.z);

	// 角度はラジアン表示
	/*DrawFormatString(0, 620, GetColor(255, 255, 255),
		"Camera Angles:(%.2f, %.2f, %.2f)", AsoUtility::Rad2DegF(angles_.x), AsoUtility::Rad2DegF(angles_.y), AsoUtility::Rad2DegF(angles_.z));*/

	//#endif //DEBUG

}

void Camera::ChangeMode(MODE mode)
{
	mode_ = mode;

	// 変更時の初期化処理
	switch (mode_)
	{
	case Camera::MODE::FIXED_POINT:
		break;
	case Camera::MODE::FREE:
		break;
	}
}


void Camera::Release(void)
{

}

const VECTOR& Camera::GetPos(void) const
{
	return pos_;
}

const VECTOR& Camera::GetAngles(void) const
{
	return angles_;
}
