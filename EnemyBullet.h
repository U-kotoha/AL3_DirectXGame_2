#pragma once
#include "Model.h"
#include "Vector3.h"
#include "ViewProjection.h"

/// <summary>
/// 敵の弾
/// </summary>
class EnemyBullet {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="pos">初期座標</param>
	/// <param name="velocity">速度</param>
	void Initialize(Model* model, const Vector3& pos, const Vector3& velocity);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="view">ビュープロダクション</param>
	void Draw(ViewProjection& view);

	// メンバ関数
	bool IsDead() const { return isDead_; }

private:
	// ワールド変換
	WorldTransform world_;
	// モデル
	Model* model_;
	// テクスチャハンドル
	uint32_t texturehandle_;
	// 速度
	Vector3 velocity_;

	// 寿命<frm>
	static const int32_t kLifeTime = 60 * 5;
	// デスタイマー
	int32_t deathTimer_ = kLifeTime;
	// デスフラグ
	bool isDead_ = false;
};
