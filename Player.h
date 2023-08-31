#pragma once

#include "Input.h"
#include "Model.h"
#include "ViewProjection.h"
#include "WorldTransform.h"
#include "PlayerBullet.h"
#include <list>
#include "Sprite.h"

/// <summary>
/// 自キャラ
/// </summary>
class Player {
public:
	/// <summary>
	/// デストラクタ
	/// </summary>
	~Player();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Model* model, uint32_t textureHandle, Vector3 position);

	/// <summary>
	/// 更新
	/// </summary>
	void Update(ViewProjection& viewProjection);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw(ViewProjection& viewProjection);

	/// <summary>
	/// 攻撃
	/// </summary>
	void Attack();

	// 衝突時コールバック関数
	void OnCollision();

	/// <summary>
	/// 親となるワールドトランスフォームをセット
	/// </summary>
	/// <param name="parent"> 親となるワールドトランスフォーム</param>
	void SetParent(const WorldTransform* parent);

	/// <summary>
	/// UI描画
	/// </summary>
	void DrawUI();

	// ワールド座標
	Vector3 GetWorldPosition();

	// 弾リスト取得
	const std::list<PlayerBullet*>& GetBullets() const { return bullets_; }

private:
	//ワールド
	WorldTransform worldTransform_;

	//モデル
	Model* model_ = nullptr;

	//テクスチャ
	uint32_t textureHandle_ = 0;

	//キーボード入力
	Input* input_ = nullptr;

	// 弾
	std::list<PlayerBullet*> bullets_;

	// 3Dレティクル
	WorldTransform worldTransform3DReticle_;
	// 2Dレティクル
	Sprite* sprite2DReticle_ = nullptr;

	Model* reticle = nullptr;
};
