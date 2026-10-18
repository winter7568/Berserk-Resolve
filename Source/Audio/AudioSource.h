#pragma once

#include <memory>
#include <xaudio2.h>
#include "AudioResource.h"

// オーディオソース
class AudioSource
{
public:
	AudioSource(IXAudio2* xaudio, std::shared_ptr<AudioResource>& resource);
	~AudioSource();

	// 再生
	void Play(bool loop);

	// 停止
	void Stop();

	// 音量設定
	void SetVolume(float volume);

	// 再生速度設定 (0.5 = 半分の速度, 2.0 = 2倍速)
	void SetPlaybackSpeed(float speed);

private:
	IXAudio2SourceVoice*			sourceVoice = nullptr;
	std::shared_ptr<AudioResource>	resource;
};
