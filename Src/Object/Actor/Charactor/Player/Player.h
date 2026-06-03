#pragma once
#include <DxLib.h>
#include <vector>
#include <string>
#include "../../../../Manager/InputManager.h"
#include "../../ActorBase.h"
#include "../../../../Object/Collider/ColliderModel.h"
#include "../../../../Scene/BuySelect.h" // ITEM_TYPE を参照

class ColliderBase;
class ResourceManager;
class AnimationController;
class Camera;

class Player : public ActorBase
{
public:

	// 定数等（既存）
	static constexpr float GRAVITY = 0.5f;
	static constexpr float MOVE_SPEED = 250.f;

	static constexpr VECTOR COL_LINE_START_LOCAL_POS = { 0.0f, 80.0f, 0.0f };
	static constexpr VECTOR COL_LINE_END_LOCAL_POS = { 0.0f, -10.0f, 0.0f };

	static constexpr VECTOR COL_CAPSULE_TOP_LOCAL_POS = { 0.0f, 110.0f, 0.0f };
	static constexpr VECTOR COL_CAPSULE_DOWN_LOCAL_POS = { 0.0f, 30.0f, 0.0f };
	static constexpr float COL_CAPSULE_RADIUS = 20.0f;

	enum class COLLIDER_TYPE
	{
		MODEL,
		LINE,
		CAPSULE,
		MAX,
	};

	enum class ANIM_TYPE
	{
		IDLE,
		RUN,
		JUMP,
		CROUCHED,
		WALK,
		MAX,
	};

	enum class STATE
	{
		IDLE,
		WALK,
		RUN,
		JUMP,
		CROUCHED,
	};

	enum class INPUT_DEVICE
	{
		KEYBOARD,
		PAD,
		BOTH,
	};

	struct INPUT_CONFIG
	{
		INPUT_DEVICE device;
		int moveForwardKey;
		int moveBackKey;
		int moveLeftKey;
		int moveRightKey;
		int cameraUpKey;
		int cameraDownKey;
		int cameraLeftKey;
		int cameraRightKey;
		int crouchKey;
		int runKey;
		InputManager::JOYPAD_NO padNo;
		InputManager::JOYPAD_BTN runPadBtn;
		InputManager::JOYPAD_BTN crouchPadBtn;
	};

	static const INPUT_CONFIG KEYBOARD_INPUT_CONFIG;
	static const INPUT_CONFIG PAD1_INPUT_CONFIG;
	static const INPUT_CONFIG KEYBOARD_AND_PAD1_INPUT_CONFIG;
	static const INPUT_CONFIG PLAYER2_KEYBOARD_INPUT_CONFIG;

	Player(void);
	~Player(void);

	void Init(void);
	void Update(void) override;

	void SetPos(const VECTOR& pos);
	void SetInputEnabled(bool isEnabled);
	void SetInputConfig(const INPUT_CONFIG& config);

	const VECTOR& GetCameraAngles(void) const;
	void SetCameraAngles(const VECTOR& angles);
	VECTOR GetCameraWorldPos(void) const;
	VECTOR GetCameraForward(void) const;
	VECTOR GetHeadWorldPos(void) const;
	void ApplyCamera(Camera* camera) const;

	void TakeDamage(int damage);
	bool CanTakeDamage(void) const;
	bool IsDead(void) const;
	int GetHp(void) const;
	int GetHpMax(void) const;
	float GetHpRate(void) const;

	// 追加: アイテム付与 / 在庫参照 / 使用
	void AddItem(int itemType);

	int GetSpikeCount() const;
	int GetMineCount() const;
	int GetFragCount() const;

	bool UseSpikeTrap();
	bool UseExplosiveTrap();
	bool UseFragGrenade();

	// 追加: 使用アイテム選択（順送り） - 所持しているものだけ巡回する
	void CycleSelectedUsableItem(int dir); // dir = +1/-1, dir==0 => select first owned
	ITEM_TYPE GetSelectedUsableItemType() const;

	// 追加: ヘルメット残数取得
	int GetHelmetUses() const;

protected:
	void InitLoad(void) override;
	void InitTransform(void) override;
	void InitCollider(void) override;
	void InitAnimation(void) override;
	void InitPost(void) override;

private:
	static constexpr float GRAVITY_TERMINAL = -10.0f;
	static constexpr float GROUND_CHECK_DISTANCE = 500.0f;
	static constexpr float GROUND_OFFSET = 1.0f;
	static constexpr float WALL_CHECK_HEIGHT = 30.0f;
	static constexpr float WALL_PUSH_BACK = 2.0f;
	static constexpr float WALL_NORMAL_Y_MAX = 0.4f;

	static constexpr float CAMERA_ROT_SPEED = DX_PI_F / 180.0f;
	static constexpr float CAMERA_PITCH_MIN = -DX_PI_F * 0.45f;
	static constexpr float CAMERA_PITCH_MAX = DX_PI_F * 0.45f;
	static constexpr float TURN_SPEED = 10.0f;

	static constexpr VECTOR INIT_POS = { 300.0f, 100.0f, 100.0f };

	// TPS カメラのローカル位置（ヘッダに宣言、実体は cpp に定義）
	static const VECTOR TPS_CAMERA_LOCAL_POS;

	static constexpr int HP_MAX = 6;
	static constexpr int DAMAGE_COOLDOWN_MAX = 60;

	float gravityVelocity_;
	bool isInputEnabled_;
	VECTOR cameraAngles_;

	int hp_;
	int damageCooldownFrame_;

	AnimationController* animController_;
	INPUT_CONFIG inputConfig_;

	void UpdateMoveInput(void);
	void UpdateCameraInput(void);
	void ApplyGravity(void);
	bool CheckGround(VECTOR& hitPos) const;
	void ResolveWallCollision(void);
	void UpdateState(void);
	STATE GetNextState(void) const;
	void ChangeState(STATE newState, bool isForce = false);
	bool HasMoveInput(void) const;

	bool IsKeyboardInputEnabled(void) const;
	bool IsPadInputEnabled(void) const;
	bool IsRunInput(void) const;
	bool IsCrouchInput(void) const;
	VECTOR GetMoveInputVector(void) const;

	void UpdateIdle(void);
	void UpdateWalk(void);
	void UpdateRun(void);
	void UpdateJump(void);
	void UpdateCrouched(void);
	void OnEnterIdle(void);
	void OnEnterWalk(void);
	void OnEnterRun(void);
	void OnEnterJump(void);
	void OnEnterCrouched(void);

	STATE state_;

	// 追加メンバ: アイテム効果 / 在庫
	int helmetUsesRemaining_ = 0; // ヘルメット防御回数
	bool hasInsurance_ = false;
	bool hasZoomCamera_ = false;

	int spikeTrapCount_ = 0;
	int explosiveTrapCount_ = 0;
	int fragGrenadeCount_ = 0;

	std::vector<int> inventory_;

	// 使用アイテム選択用（固定順序）
	int selectedUsableIndex_ = -1; // 所持品が無ければ -1
	static const std::vector<ITEM_TYPE> usableOrder_;
};

