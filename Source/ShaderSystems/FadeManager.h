#pragma once
class FadeManager
{
public:
    // フェードを開始する
    // fadeIn は true で黒から透明 false で透明から黒
    // duration はフェードの時間
    void StartFade(bool fadeIn, float duration = 0.5f)
    {
        isFading = true;
        this->fadeIn = fadeIn;
        this->duration = duration;
        timer = 0.0f;
        currentAlpha = fadeIn ? 1.0f : 0.0f;
    }

    // フェードの更新
    // deltaTime は前のフレームからの時間
    void Update(float deltaTime)
    {
        if (!isFading) return;

        timer += deltaTime;
        float progress = timer / duration;

        // 進行を 0 から 1 にする
        if (progress > 1.0f) progress = 1.0f;

        if (fadeIn)
        {
            // 黒から透明
            currentAlpha = 1.0f - progress;
        }
        else
        {
            // 透明から黒
            currentAlpha = progress;
        }

        // フェードが終わったか確認
        if (timer >= duration)
        {
            isFading = false;
            currentAlpha = fadeIn ? 0.0f : 1.0f;
        }
    }

    // 今のアルファ値を取得する
    // 0 は透明 1 は黒
    float GetAlpha() const { return currentAlpha; }

    // フェード中かどうか
    bool IsFading() const { return isFading; }

    // フェードを止めてリセット
    void Reset()
    {
        isFading = false;
        currentAlpha = 0.0f;
        timer = 0.0f;
    }

private:
    bool isFading = false;      // フェード中かどうか
    bool fadeIn = true;         // フェードの種類
    float duration = 0.5f;      // フェード時間
    float timer = 0.0f;         // 経過時間
    float currentAlpha = 0.0f;  // 現在の値
};