#include "Enemy.h"
#include "assert.h"
#include "MathUtility.h"
#include "Player.h"
#include "GameScene.h"

Enemy::~Enemy() {
	// 弾の解放
	for (EnemyBullet* bullet : bullets_) {
		delete bullet;
	}
}

void Enemy::Initialize(Model* model, const Vector3& pos) {
	// NULLチェック
	assert(model);

	// 受け渡し
	model_ = model;

	// テクスチャ読み込み
	textureHandle_ = TextureManager::Load("enemy.png");

	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();

	// 初期座標の設定
	worldTransform_.translation_ = pos;

	// 接近フェーズ初期化
	Approch_();
}

void Enemy::Update() {

	// デスフラグの立った弾を削除
	bullets_.remove_if([](EnemyBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});

	switch (phase_) {
	case Enemy::Phase::Approch:
		// 移動(ベクトル加算)
		worldTransform_.translation_.x -= 0.10f;

		// 発射タイマーカウントダウン
		fireTimer--;

		// 指定時間に達した
		if (fireTimer <= 0) {
			Fire();
			// 発射タイマーを初期化
			fireTimer = kFireInterval;
		}

		// 一定の位置になったら行動フェーズが変わる
		if (worldTransform_.translation_.x < -40.0f) {
			phase_ = Enemy::Phase::Leave;
		}
		break;

	case Enemy::Phase::Leave:
		worldTransform_.translation_.x = 40.f;
		phase_ = Enemy::Phase::Approch;

		break;
	}

	// 弾の更新
	for (EnemyBullet* bullet : bullets_) {
		bullet->Update();
	}

	// 行列更新
	worldTransform_.matWorld_ = MakeAffineMatrix(
	    worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);
	// 行列転送
	worldTransform_.TransferMatrix();
}

void Enemy::Draw(ViewProjection& viewProjection) {
	// 3Dモデルの描画
	model_->Draw(worldTransform_, viewProjection, textureHandle_);

	// 弾の描画
	for (EnemyBullet* bullet : bullets_) {
		bullet->Draw(viewProjection);
	}
}

void Enemy::Fire() {
	assert(player_);

	// 弾の速度
	const float kBulletSpeed = 0.4f;

	// ワールド座標
	Vector3 playerV = player_->GetWorldPosition();
	Vector3 enemyV = Enemy::GetWorldPosition();

	// ベクトル
	Vector3 playerV_;
	playerV_.x = enemyV.x - playerV.x;
	playerV_.y = enemyV.y - playerV.y;
	playerV_.z = enemyV.z - playerV.z;

	// 正規化
	float length =
	    sqrt(playerV_.x * playerV_.x + playerV_.y + playerV_.y + playerV_.z * playerV_.z);

	Vector3 normal;
	normal.x = playerV_.x / length;
	normal.y = playerV_.y / length;
	normal.z = playerV_.z / length;

	normal.x *= kBulletSpeed;
	normal.y *= kBulletSpeed;
	normal.z *= kBulletSpeed;

	// 弾の生成と初期化
	EnemyBullet* newBullet = new EnemyBullet;
	newBullet->Initialize(model_, worldTransform_.translation_, normal);

	// 弾の登録
	bullets_.push_back(newBullet);
}

void Enemy::OnCollision() {}

void Enemy::Approch_() {
	// 発射タイマーを初期化
	fireTimer = kFireInterval;
}

Vector3 Enemy::GetWorldPosition() {
	Vector3 worldPos;

	worldPos.x = worldTransform_.translation_.x;
	worldPos.y = worldTransform_.translation_.y;
	worldPos.z = worldTransform_.translation_.z;

	return worldPos;
}
