#include "Player.h"
#include "raylib.h"

static constexpr float kSpeed = 300.0f;

Player::Player() : rect_{0, 0, 50, 50}, name_("Player") {}

Player::Player(float x, float y, float w, float h, const std::string &name)
    : rect_{x, y, w, h}, name_(name) {}

void Player::UpdateWithInput(float deltaTime, const InputState& input) {
    float movement = kSpeed * deltaTime;

    if (input.moveRight) rect_.x += movement;
    if (input.moveLeft) rect_.x -= movement;
    if (input.moveUp) rect_.y -= movement;
    if (input.moveDown) rect_.y += movement;
}

InputState Player::GetCurrentInput() const {
    InputState input;
    input.moveRight = IsKeyDown(KEY_D);
    input.moveLeft = IsKeyDown(KEY_A);
    input.moveUp = IsKeyDown(KEY_W);
    input.moveDown = IsKeyDown(KEY_S);
    return input;
}

void Player::Draw() const {
    DrawRectangleRec(rect_, RED);
    const char *label = name_.c_str();
    int fontSize = 20;
    int textWidth = MeasureText(label, fontSize);
    DrawText(label,
             static_cast<int>(rect_.x + rect_.width / 2.0f - static_cast<float>(textWidth) / 2.0f),
             static_cast<int>(rect_.y - 25),
             fontSize,
             BLACK);
}
