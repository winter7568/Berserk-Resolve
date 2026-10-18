#pragma once

#include "GameLib/RenderContext.h"
#include "GameLib/Model.h"

class Shader
{
public:
	Shader(){}
	virtual ~Shader(){}

	//描画開始
	virtual void Begin(const RenderContext& rc) = 0;

	//モデル画面
	virtual void Draw(const RenderContext& rc, const Model* model) = 0;

	//画面終了
	virtual void End(const RenderContext& rc) = 0;
};