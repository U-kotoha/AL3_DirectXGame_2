#pragma once

#include "Model.h"
#include "ViewProjection.h"
#include "WorldTransform.h"
#include "EnemyBullet.h"
#include <list>

/// <summary>
/// 敵
/// </summary>
class Player;
class GameScene;

class Enemy {
public:
	/// <summary>
	/// デストラクタ
	/// </summary>
	~Enemy();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Model* model, const Vector3& pos);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw(ViewProjection& viewProjection);

	/// <summary>
	/// 弾発射
	/// </summary>
	void Fire();

	// 衝突時コールバック関数
	void OnCollision();

	// 弾の発射間隔
	static const int kFireInterval = 120;
	// 接近フェーズ初期化
	void Approch_();

	// 行動フェーズ
	enum class Phase {
		Approch, // 接近する
		Leave,   // 離脱する
	};

	// ワールド座標
	Vector3 GetWorldPosition();

	bool GetIsDead() { return isDead_; }

	void SetPlayer(Player* player) { player_ = player; }
	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }

private:
	// ワールドトランスフォーム
	WorldTransform worldTransform_;

	// モデル
	Model* model_ = nullptr;

	// テクスチャハンドル
	uint32_t textureHandle_;

	// フェーズ
	Phase phase_ = Enemy::Phase::Approch;

	// 発射タイマー
	int32_t fireTimer = 60;
	// デスタイマー
	int32_t deathTimer_ = 120;
	// デスフラグ
	bool isDead_ = false;

	// プレイヤー
	Player* player_ = nullptr;
	// ゲームシーン
	GameScene* gameScene_ = nullptr;
};
