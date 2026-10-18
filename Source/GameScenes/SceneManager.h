#pragma once
#include "Scene.h"

class SceneManager
{
private:
	SceneManager() {}
	~SceneManager() {}

public:

	static SceneManager& Instance()
	{
		static SceneManager instance;
		return instance;
	}

	void Update(float elapsedTime);
	void Render();
	void DrawGUI();
	void Clear();
	void ChangeScene(GameScene* scene);

	GameScene* GetCurrentScene() const { return currentScene; }

private:
	GameScene* currentScene = nullptr;
	GameScene* nextScene = nullptr;
};