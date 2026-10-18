#include "Effect.h"
#include "EffectManager.h"
#include "GameLib/Graphics.h"

Effect::Effect(const char* filename)
{
	std::lock_guard<std::mutex> lock(Graphics::Instance().GetMutex());

	char16_t utff16Filename[256];
	Effekseer::ConvertUtf8ToUtf16(utff16Filename, 256, filename);

	Effekseer::ManagerRef effeckseerManager = EffectManager::Instance().GetEffekseerManager();

	effeckseerEffect = Effekseer::Effect::Create(effeckseerManager, (EFK_CHAR*)utff16Filename);
}

Effekseer::Handle Effect::Play(const DirectX::XMFLOAT3& position, float scale)
{
	Effekseer::ManagerRef effeckseerManager = EffectManager::Instance().GetEffekseerManager();
	Effekseer::Handle handle = effeckseerManager->Play(effeckseerEffect, position.x,
		position.y, position.z);

	if (handle >= 0) {
		effeckseerManager->SetScale(handle, scale, scale, scale);
	}

	return handle;
}

void Effect::Stop(Effekseer::Handle handle)
{
	Effekseer::ManagerRef effeckseerManager = EffectManager::Instance().GetEffekseerManager();
	effeckseerManager->StopEffect(handle);
}

void Effect::SetPosition(Effekseer::Handle handle, const DirectX::XMFLOAT3& position)
{
	Effekseer::ManagerRef effeckseerManager = EffectManager::Instance().GetEffekseerManager();
	effeckseerManager->SetLocation(handle, position.x, position.y, position.z);

}

void Effect::SetScale(Effekseer::Handle handle, const DirectX::XMFLOAT3& scale)
{
	Effekseer::ManagerRef effeckseerManager = EffectManager::Instance().GetEffekseerManager();
	effeckseerManager->SetScale(handle, scale.x, scale.y, scale.z);

}

void Effect::SetRotation(Effekseer::Handle handle, const DirectX::XMFLOAT3& rotation)
{
	Effekseer::ManagerRef effeckseerManager = EffectManager::Instance().GetEffekseerManager();
	effeckseerManager->SetRotation(handle, rotation.x, rotation.y, rotation.z);
}

