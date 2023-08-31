#include "GameScene.h"
#include "TextureManager.h"
#include <cassert>
#include "AxisIndicator.h"
#include <fstream>

GameScene::GameScene() {}

GameScene::~GameScene() {

	delete model_;
	delete debugCamera_;
	delete player_;
	delete enemy_;
	delete skydome_;
	delete modelSkydome_;
	delete railCamera_;
}

void GameScene::Initialize() {

	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();

	// テクスチャ
	textureHandle_ = TextureManager::Load("player.png");

	// モデル
	model_ = Model::Create();

	// プレイヤー
	player_ = new Player();
	player_->Initialize(model_, textureHandle_, {0.0f, -5.0f, 15.0f});

	// 敵
	enemy_ = new Enemy();
	enemy_->Initialize(model_, pos_);
	enemy_->SetPlayer(player_);

	// 天球
	skydome_ = new Skydome();
	modelSkydome_ = Model::CreateFromOBJ("skydome", true);
	skydome_->Initialize(modelSkydome_);

	// レールカメラ
	railCamera_ = new RailCamera();
	railCamera_->Initialize({0.0f, 0.0f, 0.0f}, {0.0f, 0.001f, 0.0f});

	// レティクル
	TextureManager::Load("target.png");

	// ビュープロジェクション
	viewProjection_.farZ = 2000.0f;
	viewProjection_.Initialize();

	//サウンド
	soundDataHandle_ = audio_->LoadWave("stage.wav");
	audio_->PlayWave(soundDataHandle_, true, 0.5);
	voiceHandle_ = audio_->PlayWave(soundDataHandle_, true);

	//デバッグカメラ
	debugCamera_ = new DebugCamera(WinApp::kWindowWidth, WinApp::kWindowHeight);

	// 軸方向の表示
	/*AxisIndicator::GetInstance()->SetVisible(true);
	AxisIndicator::GetInstance()->SetTargetViewProjection(&viewProjection_);*/
}

void GameScene::Update() {

	switch (Nowmode) {
	case title:
		if (input_->TriggerKey(DIK_SPACE)) {
			Nowmode = stage;
		}
		break;

	case stage:
		// 更新
		player_->Update(viewProjection_);
		enemy_->Update();
		skydome_->Update();
		// railCamera_->Update();

		// 衝突判定
		CheckAllCollisions();

#ifdef _DEBUG
		if (input_->TriggerKey(DIK_BACKSPACE)) {
			isDebugCameraActive_ = true;
		}
#endif //_DEBUG

		// デバッグカメラ
		if (isDebugCameraActive_) {
			debugCamera_->Update();
			viewProjection_.matView = debugCamera_->GetViewProjection().matView;
			viewProjection_.matProjection = debugCamera_->GetViewProjection().matProjection;
			viewProjection_.TransferMatrix();
		} else {
			viewProjection_.matView = railCamera_->GetViewProjection().matView;
			viewProjection_.matProjection = railCamera_->GetViewProjection().matProjection;
			viewProjection_.TransferMatrix();
		}

		break;

	case gameclear:
		audio_->StopWave(voiceHandle_);

		break;
	}

}

void GameScene::Draw() {

	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
	// 背景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに背景スプライトの描画処理を追加できる
	/// </summary>

	// スプライト描画後処理
	Sprite::PostDraw();
	// 深度バッファクリア
	dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
	// 3Dオブジェクト描画前処理
	Model::PreDraw(commandList);

	/// <summary>
	/// ここに3Dオブジェクトの描画処理を追加できる
	/// </summary>

	switch (Nowmode) {
	case title:
		skydome_->Draw(viewProjection_);

		break;
	
	case stage:
		// 描画
		player_->Draw(viewProjection_);
		enemy_->Draw(viewProjection_);
		skydome_->Draw(viewProjection_);

		break;

	case gameclear:

		break;
	}
	// 3Dオブジェクト描画後処理
	Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
	// 前景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに前景スプライトの描画処理を追加できる
	/// </summary>
	switch (Nowmode) {
	case stage:
		player_->DrawUI();
		break;
	}
	// スプライト描画後処理
	Sprite::PostDraw();

#pragma endregion
}

void GameScene::CheckAllCollisions() {
	Vector3 posA, posB;

	// 自弾リスト
	const std::list<PlayerBullet*>& playerBullets = player_->GetBullets();

	// 敵弾リスト
	const std::list<EnemyBullet*>& enemyBullets = enemy_->GetBullets();

#pragma region 自キャラと敵弾の当たり判定
	posA = player_->GetWorldPosition();

	for (EnemyBullet* bullet : enemyBullets) {
		posB = bullet->GetWorldPosition();

		float distance = (posB.x - posA.x) * (posB.x - posA.x) +
		                 (posB.y - posA.y) * (posB.y - posA.y) +
		                 (posB.z - posA.z) * (posB.z - posA.z);

		const float Radius = 20.0f;

		// 球と球の交差判定
		if (distance <= Radius) {
			player_->OnCollision();
			bullet->OnCollision();
		}
	}
#pragma endregion

#pragma region 自弾と敵キャラの当たり判定
	posA = enemy_->GetWorldPosition();

	for (PlayerBullet* bullet : playerBullets) {
		posB = bullet->GetWorldPosition();

		float distance = (posB.x - posA.x) * (posB.x - posA.x) +
		                 (posB.y - posA.y) * (posB.y - posA.y) +
		                 (posB.z - posA.z) * (posB.z - posA.z);

		const float Radius = 20.0f;

		// 球と球の交差判定
		if (distance <= Radius) {
			Nowmode = gameclear;
			bullet->OnCollision();
		}
	}
#pragma endregion

#pragma region 自弾と敵弾の当たり判定
	for (PlayerBullet* bullet_ : playerBullets) {
		posA = bullet_->GetWorldPosition();

		for (EnemyBullet* bullet : enemyBullets) {
			posB = bullet->GetWorldPosition();

			float distance = (posB.x - posA.x) * (posB.x - posA.x) +
			                 (posB.y - posA.y) * (posB.y - posA.y) +
			                 (posB.z - posA.z) * (posB.z - posA.z);

			const float Radius = 20.0f;

			// 球と球の交差判定
			if (distance <= Radius) {
				bullet_->OnCollision();
				bullet->OnCollision();
			}
		}
	}
#pragma endregion
}
