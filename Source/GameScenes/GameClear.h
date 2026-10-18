#pragma once
#include <memory>

#include "ShaderSystems/Sprite.h"
#include "Scene.h"

class GameClear : public GameScene
{
public:
	GameClear() {}
	~GameClear() override {}

	void Initialize()override;
	void Finalize() override;
	void Update(float elapsedTime) override;
	void Render() override;
	void DrawGUI() override;

	std::string GetName() const override { return "GameClear"; }

private:
	std::unique_ptr<Sprite>  sprite;

	// Timer variables
	float timer = 0.0f;
};