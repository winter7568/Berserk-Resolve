#pragma once
#include <string>
#include "StateEnums.h"

// ステートの優先度
enum class StatePriority {
    INVALID = -1,
    MOVEMENT = 0,   // 移動 待機 歩き 走り
    ACTION = 1,     // 行動 ジャンプ ダッシュ
    ATTACK = 2,     // 攻撃
    BRAIN = 3,      // AI制御
    REACTION = 4,   // ダメージ状態
    OVERRIDE = 5    // 強制状態 イベントなど
};

template <typename Owner, typename StateID>
class State {
public:
    virtual ~State() = default;

    // ステート開始
    virtual void Enter(Owner& owner) = 0;

    // 更新
    virtual void Update(Owner& owner, float elapsedTime) = 0;

    // ステート終了
    virtual void Exit(Owner& owner) = 0;

    // 名前取得
    virtual std::string GetName() const = 0;

    // ID取得
    virtual StateID GetStateID() const = 0;

    // 優先度取得
    virtual StatePriority GetPriority() const = 0;

    // 他のステートを上書きできるか
    virtual bool CanOverride(const State& other) const {
        return GetPriority() > other.GetPriority();
    }

    // 他のステートと同時に使えるか
    virtual bool CanCombineWith(const State& other) const {
        return GetPriority() != other.GetPriority();
    }

    // 終了するかどうか
    virtual bool ShouldExit() const { return false; }

    // アニメーションブレンドするか
    virtual bool ShouldBlendWithLowerStates() const { return true; }

    // ブレンド時間
    virtual float GetAnimationBlendTime() const { return 0.1f; }
};