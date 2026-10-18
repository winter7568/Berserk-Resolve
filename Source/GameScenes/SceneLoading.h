#pragma once
#include<thread>

#include "ShaderSystems/Sprite.h"
#include "Scene.h"
#include "Audio/Audio.h"

class SceneLoading : public GameScene
{
public:
	SceneLoading(GameScene* nextScene) : nextScene(nextScene) {}
	~SceneLoading() override {}

	void Initialize()override;
	void Finalize() override;
	void Update(float elapsedTime) override;
	void Render() override;
	void DrawGUI() override;

	std::string GetName() const override { return "SceneLoading"; }

private:
	static void LoadingThread(SceneLoading* scene);

private:
	std::unique_ptr<Sprite>   sprite;
	float angle = 0.0f;
	GameScene* nextScene = nullptr;
	std::thread* thread = nullptr;

	AudioSource* bgm = nullptr;
};