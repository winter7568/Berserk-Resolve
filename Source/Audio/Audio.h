#pragma once

#include <xaudio2.h>
#include "AudioSource.h"

// オーディオ管理クラス
class Audio
{
private:
    Audio() = default;
    ~Audio() = default;

public:
    // インスタンス取得
    static Audio& Instance()
    {
        static Audio instance;
        return instance;
    }

    // 初期化
    void Initialize();

    // 終了処理
    void Finalize();

    // オーディオソース読み込み
    AudioSource* LoadAudioSource(const char* filename);

private:
    IXAudio2* xaudio = nullptr;
    IXAudio2MasteringVoice* masteringVoice = nullptr;
};