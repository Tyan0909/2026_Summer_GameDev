#include "Player.h"
#include <DxLib.h>

Player::Player()
{
}

Player::~Player()
{
}

void Player::Init(void)
{
	// モデルの読み込み
	//modelId_ = LoadModel("Data/Model/Player.mqo");
	// 位置・角度・拡縮の初期化
	pos_ = VGet(0.0f, 0.0f, 0.0f);
	angle_ = VGet(0.0f, 0.0f, 0.0f);
	scale_ = VGet(1.0f, 1.0f, 1.0f);
}

void Player::Update(void)
{

}

void Player::Draw(void)
{

}

void Player::Release(void)
{

}