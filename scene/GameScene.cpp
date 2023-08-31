#include "GameScene.h"
#include "TextureManager.h"
#include <cassert>
#include "AxisIndicator.h"
#include <fstream>

GameScene::GameScene() {}

GameScene::~GameScene() {
	// 弾の解放
	for (EnemyBullet* bullet : bullets_) {
		delete bullet;
	}
	delete model_;
	delete debugCamera_;
	delete player_;
	for (Enemy* enemy : enemy_) {
		delete enemy;
	}
	delete skydome_;
	delete modelSkydome_;
	delete railCamera_;
}

void GameScene::Initialize() {

	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();

	//テクスチャ
	textureHandle_ = TextureManager::Load("white1x1.png");

	//モデル
	model_ = Model::Create();

	// プレイヤー
	player_ = new Player();
	player_->Initialize(model_, textureHandle_, {0.0f, -5.0f, 15.0f});

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
	soundDataHandle_ = audio_->LoadWave("fanfare.wav");
	audio_->PlayWave(soundDataHandle_);
	voiceHandle_ = audio_->PlayWave(soundDataHandle_, true);

	//デバッグカメラ
	debugCamera_ = new DebugCamera(WinApp::kWindowWidth, WinApp::kWindowHeight);

	// 軸方向の表示
	AxisIndicator::GetInstance()->SetVisible(true);
	AxisIndicator::GetInstance()->SetTargetViewProjection(&viewProjection_);
}

void GameScene::Update() {

	// 更新
	player_->Update(viewProjection_);
	skydome_->Update();
	//railCamera_->Update();

	// 衝突判定
	CheckAllCollisions();

	//サウンド
	if (input_->TriggerKey(DIK_RETURN)) {
		audio_->StopWave(voiceHandle_);
	}

	enemy_.remove_if([](Enemy* enemy) {
		if (enemy->GetIsDead()) {
			delete enemy;
			return true;
		}
		return false;
	});

	// 敵発生
	LoadEnemyPopDate();
	UpdateEnemyPopCommands();

	// 敵の更新
	for (Enemy* enemy : enemy_) {
		enemy->Update();
	}

	// デスフラグの立った弾を削除
	bullets_.remove_if([](EnemyBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});

	// 弾の更新
	for (EnemyBullet* bullet : bullets_) {
		bullet->Update();
	}

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

	// 描画
	for (Enemy* enemy : enemy_) {
		enemy->Draw(viewProjection_);
	}
	for (EnemyBullet* bullet : bullets_) {
		bullet->Draw(viewProjection_);
	}

	player_->Draw(viewProjection_);
	skydome_->Draw(viewProjection_);

	// 3Dオブジェクト描画後処理
	Model::PostDraw();
#pragma endregion

#pragma region 前景スプライト描画
	// 前景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに前景スプライトの描画処理を追加できる
	/// </summary>

	player_->DrawUI();

	// スプライト描画後処理
	Sprite::PostDraw();

#pragma endregion
}

void GameScene::CheckAllCollisions() {
	Vector3 posA, posB;

	// 自弾リスト
	const std::list<PlayerBullet*>& playerBullets = player_->GetBullets();

	// 敵弾リスト
	const std::list<EnemyBullet*>& enemyBullets = GetBullets();

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
	for (Enemy* enemy : enemy_) {
		posA = enemy->GetWorldPosition();

		for (PlayerBullet* bullet : playerBullets) {
			posB = bullet->GetWorldPosition();

			float distance = (posB.x - posA.x) * (posB.x - posA.x) +
			                 (posB.y - posA.y) * (posB.y - posA.y) +
			                 (posB.z - posA.z) * (posB.z - posA.z);

			const float Radius = 20.0f;

			// 球と球の交差判定
			if (distance <= Radius) {
				enemy->OnCollision();
				bullet->OnCollision();
			}
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

void GameScene::LoadEnemyPopDate() {

	std::ifstream file;
	file.open("./Resources/enemyPop.csv");
	assert(file.is_open());

	enemyPopCommands << file.rdbuf();

	file.close();
}

void GameScene::UpdateEnemyPopCommands() {

	// 待機処理
	if (isWait_) {
		WaitTimer_--;
		if (WaitTimer_ <= 0) {
			isWait_ = false;
		}
		return;
	}

	std::string line;

	while (getline(enemyPopCommands, line)) {
		std::istringstream line_stream(line);

		std::string word;

		getline(line_stream, word, ',');

		if (word.find("//") == 0) {
			continue;
		}

		if (word.find("POP") == 0) {
			getline(line_stream, word, ',');
			float x = (float)std::atof(word.c_str());

			getline(line_stream, word, ',');
			float y = (float)std::atof(word.c_str());

			getline(line_stream, word, ',');
			float z = (float)std::atof(word.c_str());

			AddEnemy(Vector3(x, y, z));
		}

		else if (word.find("WAIT") == 0) {
			getline(line_stream, word, ',');

			int32_t waitTime = atoi(word.c_str());

			isWait_ = true;
			WaitTimer_ = waitTime;

			break;
		}
	}
}

void GameScene::AddEnemyBullet(EnemyBullet* enemyBullet) { bullets_.push_back(enemyBullet); }

void GameScene::AddEnemy(Vector3 pos) {
	// 敵
	Enemy* obj = new Enemy;
	obj->Initialize(model_, pos);
	obj->SetPlayer(player_);
	obj->SetGameScene(this);

	enemy_.push_back(obj);
}
