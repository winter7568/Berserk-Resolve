#pragma once
#include "Camera.h"
#include <DirectXMath.h>

class CinematicCameraController
{
public:
    // シネマティック開始（プレイヤーと敵の位置をセット）
    void StartSequence(const DirectX::XMFLOAT3& playerPos,
        const DirectX::XMFLOAT3& enemyPos);

    // 毎フレーム更新
    void Update(float elapsedTime);

    // シーケンス終了チェック
    bool IsSequenceComplete() const { return isComplete; }

    // シーケンスが動作中かどうか
    bool IsActive() const { return isActive; }

    // 計算したカメラを実際のカメラに適用
    void ApplyToCamera(Camera& camera);

private:
    bool isActive = false;        // シネマティックが有効か
    bool isComplete = false;      // シーケンスが終わったか
    bool shakeTriggered = false;  // シェイクを発動したか
    float timer = 0.0f;           // 経過時間

    DirectX::XMFLOAT3 playerPosition = {}; // プレイヤー位置
    DirectX::XMFLOAT3 enemyPosition = {};  // 敵の位置

    DirectX::XMFLOAT3 currentEye = {};   // カメラ位置
    DirectX::XMFLOAT3 currentFocus = {}; // 注視点

    // キャラの高さ補正 
    static constexpr float PLAYER_HEAD_HEIGHT = 8.0f; // プレイヤー頭の高さ
    static constexpr float ENEMY_HEAD_HEIGHT = 8.0f;  // 敵の頭の高さ

    // 初期カメラ位置
    static constexpr float START_BACK_OFFSET = 12.0f;  // 後ろにどれだけ下げるか
    static constexpr float START_HEIGHT_OFFSET = 6.0f; // 上にどれだけ上げるか

    // カメラシェイク 
    static constexpr float SHAKE_TRIGGER_TIME = 0.3f; // 何秒後にシェイクするか
    static constexpr float SHAKE_TRAUMA = 1.8f;       // シェイク強さ

    // 各フェーズの時間 
    static constexpr float PHASE1_DURATION = 3.0f; // プレイヤー→敵へ移動
    static constexpr float PHASE2_DURATION = 3.0f; // 敵を映す
    static constexpr float PHASE3_DURATION = 2.0f; // プレイヤーに戻る
    static constexpr float TOTAL_SEQUENCE_TIME = 8.0f; // 合計時間

    // フェーズ1のオフセット 
    static constexpr float PHASE1_Y_OFFSET = 4.0f;       // 上方向
    static constexpr float PHASE1_Z_OFFSET = 12.0f;      // 後ろ方向
    static constexpr float PHASE1_ENEMY_Z_OFFSET = 5.0f; // 敵前方
    static constexpr float PHASE3_ENEMY_Y_OFFSET = 4.0f; // 戻る時の敵Y
    static constexpr float PHASE3_PLAYER_Y_OFFSET = 6.0f;// 戻る時のプレイヤーY
    static constexpr float PHASE3_ENEMY_Z_OFFSET = 5.0f; // 戻る時の敵Z

    // 敵の向き
    static constexpr float ENEMY_YAW_DEGREES = 80.0f; // 敵の回転角

    // 周回カメラ設定
    static constexpr float ORBIT_RADIUS = 34.0f;          // 回転半径
    static constexpr float ORBIT_SPEED_FACTOR = 0.25f;    // 回転速度
    static constexpr float ORBIT_HEIGHT_BASE = 6.0f;      // 基本高さ
    static constexpr float ORBIT_HEIGHT_VARIATION = 2.0f; // 高さ変化
    static constexpr float FORWARD_MOVE_FACTOR = 0.1f;    // 少し前に寄る量

    // 注視点の高さ
    static constexpr float FOCUS_HEIGHT_OFFSET = 1.0f; // プレイヤー注視高さ
    static constexpr float ENEMY_FOCUS_HEIGHT = 2.0f;  // 敵注視高さ
};