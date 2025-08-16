#pragma once

class Scene {
public:
    virtual ~Scene() = default;

    virtual void Initialize() = 0;
    virtual void Update() = 0;
    virtual void Draw() const = 0;
    virtual void Unload() = 0;
};
