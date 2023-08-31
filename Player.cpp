#include "Player.h"
#include "ImGuiManager.h"
#include "assert.h"
#include "MathUtility.h"
#include "WinApp.h"

Player::~Player() {
	// 弾の解放
	for (PlayerBullet* bullet : bullets_) {
		delete bullet;
	}
	delete sprite2DReticle_;
}

void Player::Initialize(Model* model, uint32_t textureHandle, Vector3 position) {
	// NULLチェック
	assert(model);

	// 受け渡し
	model_ = model;
	textureHandle_ = textureHandle;
	worldTransform_.translation_ = position;

	// ワールド初期化
	worldTransform_.Initialize();

	// シングルインスタンスを取得
	input_ = Input::GetInstance();

	worldTransform3DReticle_.Initialize();
	reticle = Model::CreateFromOBJ("cube", true);

	uint32_t textureReticle = TextureManager::Load("target.png");
	sprite2DReticle_ = Sprite::Create(
	    textureReticle, {worldTransform_.translation_.x, worldTransform_.translation_.y},
	    {1, 1, 1, 1}, {0.5f, 0.5f});
}

void Player::Update(ViewProjection& viewProjection) {

	// 3Dレティクル
	const float kDistancePlayerTo3DReaticle = 50.0f;
	Vector3 offset{0.0f, 0.0f, 1.0f};
	offset = TransformNormal(offset, worldTransform_.matWorld_);
	offset = Multiply(Normalize(offset), kDistancePlayerTo3DReaticle);

	worldTransform3DReticle_.translation_.x = worldTransform_.translation_.x;
	worldTransform3DReticle_.translation_.y = worldTransform_.translation_.y;
	worldTransform3DReticle_.translation_.z =
	    worldTransform_.translation_.z + kDistancePlayerTo3DReaticle;

	// 行列更新
	worldTransform3DReticle_.matWorld_ = MakeAffineMatrix(
	    worldTransform3DReticle_.scale_, worldTransform3DReticle_.rotation_,
	    worldTransform3DReticle_.translation_);
	// 行列転送
	worldTransform3DReticle_.TransferMatrix();

	// 2Dレティクル
	Vector3 positionReticle = {
	    worldTransform3DReticle_.translation_.x, worldTransform3DReticle_.translation_.y,
	    worldTransform3DReticle_.translation_.z};
	Matrix4x4 matViewport =
	    MakeViewportMatrix(0, 0, WinApp::kWindowWidth, WinApp::kWindowHeight, 0, 1);
	Matrix4x4 matViewProjectionViewport =
	    viewProjection.matView * viewProjection.matProjection * matViewport;
	positionReticle = Transform(positionReticle, matViewProjectionViewport);
	sprite2DReticle_->SetPosition(Vector2(positionReticle.x, positionReticle.y));

	// デスフラグの立った弾を削除
	bullets_.remove_if([](PlayerBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});

	// 移動ベクトル
	Vector3 move = {0, 0, 0};
	// 移動の速さ
	const float kCharacterSpeed = 0.3f;

	// 上下左右の移動
	if (input_->PushKey(DIK_LEFT)) {
		move.x -= kCharacterSpeed;
	} else if (input_->PushKey(DIK_RIGHT)) {
		move.x += kCharacterSpeed;
	}
	if (input_->PushKey(DIK_DOWN)) {
		move.y -= kCharacterSpeed;
	} else if (input_->PushKey(DIK_UP)) {
		move.y += kCharacterSpeed;
	}

	// 旋回(回転)の速さ
	const float kRotSpeed = 0.02f;
	// 押した方向で移動ベクトルを変更
	if (input_->PushKey(DIK_A)) {
		worldTransform_.rotation_.y += kRotSpeed;
	} else if (input_->PushKey(DIK_D)) {
		worldTransform_.rotation_.y -= kRotSpeed;
	}

	// 攻撃処理
	Attack();

	// 弾の更新
	for (PlayerBullet* bullet : bullets_) {
		bullet->Update();
	}

	// 移動限界座標
	const float kMoveLimitX = 40.0f;
	const float kMoveLimitY = 25.0f;

	// 範囲を超えない処理
	worldTransform_.translation_.x = max(worldTransform_.translation_.x, -kMoveLimitX);
	worldTransform_.translation_.x = min(worldTransform_.translation_.x, +kMoveLimitX);
	worldTransform_.translation_.y = max(worldTransform_.translation_.y, -kMoveLimitY);
	worldTransform_.translation_.y = min(worldTransform_.translation_.y, +kMoveLimitY);

	// 座標移動
	worldTransform_.translation_.x += move.x;
	worldTransform_.translation_.y += move.y;
	worldTransform_.translation_.z += move.z;

	// 行列更新
	worldTransform_.matWorld_ = MakeAffineMatrix(
	    worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	// 行列転送
	worldTransform_.TransferMatrix();

	// ImGui
	// ウィンドウのサイズ、座標の指定
	ImGui::SetNextWindowPos({0, 0});
	ImGui::SetNextWindowSize({300, 100});

	ImGui::Begin("Debug");
	// 座標設定
	float sliderValue3[3] = {
	    worldTransform_.translation_.x, worldTransform_.translation_.y,
	    worldTransform_.translation_.z};
	ImGui::SliderFloat3("Player", sliderValue3, -20.0f, 20.0f);
	worldTransform_.translation_ = {sliderValue3[0], sliderValue3[1], sliderValue3[2]};

	ImGui::End();
}

void Player::Draw(ViewProjection& viewProjection) {
	// reticle->Draw(worldTransform3DReticle_, viewProjection);
	model_->Draw(worldTransform_, viewProjection, textureHandle_);

	// 弾の描画
	for (PlayerBullet* bullet : bullets_) {
		bullet->Draw(viewProjection);
	}
}

void Player::Attack() {
	if (input_->TriggerKey(DIK_SPACE)) {

		// 弾の速度
		const float kBulletSpeed = 2.0f;
		Vector3 velocity(0, 0, kBulletSpeed);

		// 速度ベクトルを自機の向きに合わせて回転させる
		velocity = TransformNormal(velocity, worldTransform_.matWorld_);

		velocity.x = worldTransform3DReticle_.translation_.x - GetWorldPosition().x;
		velocity.y = worldTransform3DReticle_.translation_.y - GetWorldPosition().y;
		velocity.z = worldTransform3DReticle_.translation_.z - GetWorldPosition().z;

		velocity = Multiply(Normalize(velocity), kBulletSpeed);

		// 弾の生成と初期化
		PlayerBullet* newBullet = new PlayerBullet;
		newBullet->Initialize(model_, worldTransform_.translation_, velocity);

		// 弾の登録
		bullets_.push_back(newBullet);
	}
}

void Player::OnCollision() {}

void Player::DrawUI() { sprite2DReticle_->Draw(); }

Vector3 Player::GetWorldPosition() {
	Vector3 worldPos;

	worldPos.x = worldTransform_.translation_.x;
	worldPos.y = worldTransform_.translation_.y;
	worldPos.z = worldTransform_.translation_.z;

	return worldPos;
}

void Player::SetParent(const WorldTransform* parent) {
	// 親子関係を結ぶ
	worldTransform_.parent_ = parent;
}
