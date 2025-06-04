#include "PakuScene.h"



void PakuScene::Initialize()
{
	BaseScene::Initialize();


	directionalLight = std::make_unique<DirectionalLight>();
	directionalLight->Initilaize();

	pointLight = std::make_unique<PointLight>();
	pointLight->Initilize();

	spotLight = std::make_unique<SpotLight>();
	spotLight->Initialize();

	skyDome_ = std::make_unique<SkyDome>();
	skyDome_->Initialize();

	ground_ = std::make_unique<Ground>();
	ground_->Initialize();

	// プレイヤーを生成
	player_ = std::make_unique<Player>();
	player_->Initialize();
	
	// LockOn

	lockOn_ = std::make_unique<LockOn>();
	lockOn_->Initialize();




	LoadEnemyPopData();
}

void PakuScene::Finalize()
{

	skyDome_.reset();

	ground_.reset();

	enemy_.reset();

	lockOn_.reset();
}

void PakuScene::Update()
{

#ifdef _DEBUG



#endif

	BaseScene::Update();

	// プレイヤーの更新
	player_->Update();

	//lockOn
	Vector3 playerPos = player_->GetPosition();
	Vector3 lockOnPos = { playerPos.x, playerPos.y, playerPos.z + 20.0f };
	/*lockOn_->SetPosition(lockOnPos);
	lockOn_->Update(enemies_);*/

	/*lockOn_->DetectEnemies(enemies_);*/

	skyDome_->Update();
	ground_->Update();
	//enemy_->Update();
	//
	UpdateEnemyPopCommands();
	for (const auto& enemy : enemies_) {
		enemy->Update();
	}

	/*lockOn_->DetectEnemies(enemies_);*/






}

void PakuScene::Draw()
{
	DrawBackgroundSprite();
	/// 背景スプライト描画




	DrawObject();
	/// オブジェクト描画	

	//========================================
	// プレイヤーの描画
	player_->Draw(Camera::GetInstance()->GetViewProjection(),
		*directionalLight.get(),
		*pointLight.get(),
		*spotLight.get());

	//skydome
	skyDome_->Draw(Camera::GetInstance()->GetViewProjection(),
		*directionalLight.get(),
		*pointLight.get(),
		*spotLight.get());
	//ground
	ground_->Draw(Camera::GetInstance()->GetViewProjection(),
		*directionalLight.get(),
		*pointLight.get(),
		*spotLight.get());

	for (const auto& enemy : enemies_) {
		enemy->Draw(Camera::GetInstance()->GetViewProjection(),
			*directionalLight.get(),
			*pointLight.get(),
			*spotLight.get());
	}

	//LockOn
	lockOn_->Draw(Camera::GetInstance()->GetViewProjection(),
		*directionalLight.get(),
		*pointLight.get(),
		*spotLight.get());

	DrawForegroundSprite();
	/// 前景スプライト描画	




}

void PakuScene::LoadEnemyPopData() {
	std::ifstream file;
	file.open("./Resources/enemyPop.csv");
	assert(file.is_open());

	enemyPopCommands << file.rdbuf();

	file.close();
}

void PakuScene::UpdateEnemyPopCommands() {
	if (isWaiting_) {
		--waitTimer_;
		if (--waitTimer_ <= 0) {
			isWaiting_ = false;
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

			SpawnEnemy(Vector3(x, y, z));
		} else if (word.find("WAIT") == 0) {

			getline(line_stream, word, ',');

			int32_t waitTime = atoi(word.c_str());

			isWaiting_ = true;
			waitTimer_ = waitTime;

			break;

		}


	}
}

void PakuScene::SpawnEnemy(const Vector3& position) {

	std::unique_ptr<Enemy> newEnemy = std::make_unique<Enemy>();
	newEnemy->Initialize();
	newEnemy->SetPosition(position);
	//newEnemy->SetPlayer(player_);
	enemies_.push_back(std::move(newEnemy));

}
