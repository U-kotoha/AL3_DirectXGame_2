#pragma once

#include "Input.h"
#include "Model.h"
#include "ViewProjection.h"
#include "WorldTransform.h"

/// <summary>
/// 自キャラ
/// </summary>
class Player {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Model* model, uint32_t textureHandle);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw(ViewProjection& viewProjection);

private:
	//ワールド
	WorldTransform worldTransform_;

	//モデル
	Model* model_ = nullptr;

	//テクスチャ
	uint32_t textureHandle_ = 0;

	//キーボード入力
	Input* input_ = nullptr;
};
