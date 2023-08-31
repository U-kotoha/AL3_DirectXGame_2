#include "Skydome.h"
#include "assert.h"

void Skydome::Initialize(Model* model) {

	assert(model);
	model_ = model;

	worldTransform_.Initialize();
	worldTransform_.translation_ = {0, 0, 0};
}

void Skydome::Update() { worldTransform_.UpdateMatrix(); }

void Skydome::Draw(ViewProjection& viewProjection_) {

	model_->Draw(worldTransform_, viewProjection_);
}
