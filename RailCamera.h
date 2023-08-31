#pragma once
#include "ViewProjection.h"
#include "WorldTransform.h"

/// <summary>
/// レールカメラ
/// </summary>
class RailCamera {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Vector3 trans, Vector3 rot);

	ViewProjection& GetViewProjection() { return viewProjection_; }
	WorldTransform& GetWorldTransform() { return worldTransform_; }

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

private:
	// ワールド変換
	WorldTransform worldTransform_;
	// ビュープロジェクション
	ViewProjection viewProjection_;
};
