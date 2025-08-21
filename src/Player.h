#pragma once
#include "raylib.h"
#include <string>

struct InputState {
    bool moveUp = false;
    bool moveDown = false;
    bool moveLeft = false;
    bool moveRight = false;
};

class Player {
public:
    Player();
    Player(float x, float y, float w, float h, const std::string &name);

    void UpdateWithInput(float deltaTime, const InputState& input);
    void Draw() const;

    [[nodiscard]] InputState GetCurrentInput() const;

    [[nodiscard]] const Rectangle &GetRect() const { return rect_; }
    void SetPosition(float x, float y) { rect_.x = x; rect_.y = y; }
    void SetName(const std::string &name) { name_ = name; }

private:
    Rectangle rect_{};
    std::string name_;
};
