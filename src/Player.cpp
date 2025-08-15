#include "Player.h"
#include "raylib.h"

static constexpr float kSpeed = 5.0f;

Player::Player() : rect_{0, 0, 50, 50}, name_("Player") {}

Player::Player(float x, float y, float w, float h, const std::string &name)
    : rect_{x, y, w, h}, name_(name) {}

void Player::Update() {
    if (IsKeyDown(KEY_D)) rect_.x += kSpeed;
    if (IsKeyDown(KEY_A)) rect_.x -= kSpeed;
    if (IsKeyDown(KEY_W)) rect_.y -= kSpeed;
    if (IsKeyDown(KEY_S)) rect_.y += kSpeed;
}

void Player::Draw() const {
    DrawRectangleRec(rect_, RED);
    const char *label = name_.c_str();
    int fontSize = 20;
    int textWidth = MeasureText(label, fontSize);
    DrawText(label,
             static_cast<int>(rect_.x + rect_.width / 2 - textWidth / 2),
             static_cast<int>(rect_.y - 25),
             fontSize,
             BLACK);
}

