#pragma once
#include <string>
#include "SceneConstants.h"

class GameScene
{
public:
	GameScene() {}
	virtual ~GameScene() {}

	virtual void Initialize() = 0;
	virtual void Finalize() = 0;
	virtual void Update(float elapsedTime) = 0;
	virtual void Render() = 0;
	virtual void DrawGUI() = 0;

	bool IsReady() const { return ready; }
	void SetReady() { ready = true; }
	virtual  std::string GetName() const = 0;

private:
	bool    ready = false;
};
