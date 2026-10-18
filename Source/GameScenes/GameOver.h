#pragma once
#include <memory>

#include "ShaderSystems/Sprite.h"
#include "Scene.h"

class GameOver : public GameScene
{
public:
	GameOver() {}
	~GameOver() override {}

	void Initialize()override;
	void Finalize() override;
	void Update(float elapsedTime) override;
	void Render() override;
	void DrawGUI() override;

	std::string GetName() const override { return "GameOver"; }

private:
	std::unique_ptr<Sprite>  sprite;

	float timer = 0.0f;
};