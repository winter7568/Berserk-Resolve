#pragma once
#include <DirectXMath.h>
#include <vector>
#include <string>

// 攻撃データ構造体
struct AttackData
{
    int totalFrames;    // アニメーション総フレーム数
    int hitStart;       // ヒット判定開始フレーム
    int hitEnd;         // ヒット判定終了フレーム
    int cancelStart;    // キャンセル可能開始フレーム
    int cancelEnd;      // キャンセル可能終了フレーム
    bool hasCancel;     // キャンセル可能か 最後の攻撃以外はtrue
};

// 投射物攻撃データ構造体
struct ProjectileAttackData
{
    int spawnFrame;           // 投射物を生成するフレーム
    int projectileCount;      // 投射物の数
    std::string type;         // 投射物の種類
    std::vector<float> delays;// 複数投射物の間の遅延時間
    std::string spawnNode;    // 生成するボーン名
};