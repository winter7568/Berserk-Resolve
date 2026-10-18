#include "SceneManager.h"

void SceneManager::Update(float elapsedTime)
{
	// 次のシーンがあれば切り替え
	if (nextScene != nullptr)
	{
		// 今のシーンを削除
		Clear();

		// 新しいシーンに変更
		currentScene = nextScene;
		nextScene = nullptr;

		// 初期化
		if (!currentScene->IsReady())
		{
			currentScene->Initialize();
		}
	}

	// 現在のシーン更新
	if (currentScene != nullptr)
	{
		currentScene->Update(elapsedTime);
	}
}

void SceneManager::Render()
{
	// 現在のシーン描画
	if (currentScene != nullptr)
	{
		currentScene->Render();
	}
}

void SceneManager::DrawGUI()
{
	// GUI描画
	if (currentScene != nullptr)
	{
		currentScene->DrawGUI();
	}
}

void SceneManager::Clear()
{
	// シーン削除
	if (currentScene != nullptr)
	{
		currentScene->Finalize();
		delete currentScene;
		currentScene = nullptr;
	}
}

void SceneManager::ChangeScene(GameScene* scene)
{
	// 次のシーンをセット
	nextScene = scene;
}