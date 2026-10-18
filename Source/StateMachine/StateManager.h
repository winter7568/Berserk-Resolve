#pragma once
#include <vector>
#include <string>
#include "StateMachine/State.h"

template <typename Owner, typename StateID>
class StateManager {
public:
    explicit StateManager(Owner& owner) : owner(owner) {}

    // 新しいステートをスタックに追加
    void PushState(State<Owner, StateID>* newState) {
        if (!newState) return;

        // スタックにステートがあり 組み合わせできない場合は上のステートを削除
        if (!stateStack.empty() && !newState->CanCombineWith(*stateStack.back())) {
            stateStack.back()->Exit(owner);
            stateStack.pop_back();
        }
        // 新しいステートを開始して追加
        newState->Enter(owner);
        stateStack.push_back(newState);
    }

    // 一番上のステートを削除
    void PopState() {
        if (!stateStack.empty()) {
            stateStack.back()->Exit(owner);
            stateStack.pop_back();
        }
    }

    // IDでステートを削除
    bool RemoveState(StateID stateID) {
        for (auto it = stateStack.begin(); it != stateStack.end(); ) {
            if ((*it)->GetStateID() == stateID) {
                (*it)->Exit(owner);
                it = stateStack.erase(it);
                return true;
            }
            else {
                ++it;
            }
        }
        return false;
    }

    // 名前でステートを削除
    bool RemoveState(const std::string& stateName) {
        for (auto it = stateStack.begin(); it != stateStack.end(); ) {
            if ((*it)->GetName() == stateName) {
                (*it)->Exit(owner);
                it = stateStack.erase(it);
                return true;
            }
            else {
                ++it;
            }
        }
        return false;
    }

    // すべてのステートを削除
    void ClearStates() {
        for (auto& state : stateStack) {
            state->Exit(owner);
        }
        stateStack.clear();
    }

    // 一番上のステートを更新
    void Update(float elapsedTime) {
        if (!stateStack.empty()) {
            State<Owner, StateID>* currentState = stateStack.back();
            currentState->Update(owner, elapsedTime);

            // 終了が必要なら削除
            if (currentState->ShouldExit()) {
                if (!stateStack.empty() && stateStack.back() == currentState) {
                    currentState->Exit(owner);
                    stateStack.pop_back();
                }
            }
        }
    }

    // 指定したIDのステートがあるか確認
    bool IsInState(StateID stateID) const {
        for (const auto& state : stateStack) {
            if (state->GetStateID() == stateID)
                return true;
        }
        return false;
    }

    // 一番上のステートが指定IDか確認
    bool IsTopState(StateID stateID) const {
        if (stateStack.empty()) return false;
        return stateStack.back()->GetStateID() == stateID;
    }

    // 現在のステートIDを取得
    StateID GetCurrentStateID() const {
        if (stateStack.empty()) {
            return static_cast<StateID>(0);
        }
        return stateStack.back()->GetStateID();
    }

    // 複数のステートの中にあるか確認
    bool IsInAnyState(const std::initializer_list<StateID>& stateIDs) const {
        for (const auto& state : stateStack) {
            StateID currentID = state->GetStateID();
            for (StateID id : stateIDs) {
                if (currentID == id) return true;
            }
        }
        return false;
    }

    // 名前でステートを確認
    bool IsInState(const std::string& stateName) const {
        for (const auto& state : stateStack) {
            if (state->GetName() == stateName)
                return true;
        }
        return false;
    }

    // 現在のステートを取得
    State<Owner, StateID>* GetCurrentState() const {
        return stateStack.empty() ? nullptr : stateStack.back();
    }

    // 現在の優先度を取得
    StatePriority GetCurrentPriority() const {
        return stateStack.empty() ? StatePriority::INVALID : stateStack.back()->GetPriority();
    }

    // 移動系ステートか確認
    bool IsInMovementState() const {
        return GetCurrentPriority() <= StatePriority::ACTION;
    }

    // 現在のステート名を取得
    std::string GetCurrentStateName() const {
        if (stateStack.empty()) {
            return "None";
        }
        return stateStack.back()->GetName();
    }

    // スタックの内容をデバッグ表示
    void PrintStateStack() {
        std::string output = "Active states: " + std::to_string(stateStack.size()) + "\n";
        for (auto& state : stateStack) {
            output += "- " + state->GetName() + "\n";
        }

        OutputDebugStringA(output.c_str());
    }

    // スタックの数を取得
    size_t GetStackSize() const { return stateStack.size(); }

private:
    Owner& owner;
    std::vector<State<Owner, StateID>*> stateStack;
};