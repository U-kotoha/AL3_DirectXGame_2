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

	void SetPlayer(Player* player) { player_ = player; }
	// ワールド座標
	Vector3 GetWorldPosition();

	// 弾リスト取得
	const std::list<EnemyBullet*>& GetBullets() const { return bullets_; }

private:
	// ワールドトランスフォーム
	WorldTransform worldTransform_;

	// モデル
	Model* model_ = nullptr;

	// テクスチャハンドル
	uint32_t textureHandle_;

	// フェーズ
	Phase phase_ = Enemy::Phase::Approch;

	// 敵の弾
	std::list<EnemyBullet*> bullets_;

	// 発射タイマー
	int32_t fireTimer = 60;

	// プレイヤー
	Player* player_ = nullptr;

	int flag = false;
};
