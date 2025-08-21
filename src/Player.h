#pragma once
#include "raylib.h"
#include <string>

class Player {
public:
    Player();
    Player(float x, float y, float w, float h, const std::string &name);

    void Update(float deltaTime);
    void Draw() const;

    const Rectangle &GetRect() const { return rect_; }
    void SetPosition(float x, float y) { rect_.x = x; rect_.y = y; }
    void SetName(const std::string &name) { name_ = name; }

private:
    Rectangle rect_{};
    std::string name_;
};
