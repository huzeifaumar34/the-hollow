#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace hollow {

enum class GameState : int {
    Menu = 0,
    Playing = 1,
    Dead = 2,
    Complete = 3
};

struct Platform {
    float left;
    float top;
    float right;
};

struct Snapshot {
    float player_x = 90.0f;
    float player_y = 365.0f;
    float velocity_x = 0.0f;
    float velocity_y = 0.0f;
    float grounded = 0.0f;
    float camera_x = 0.0f;
    float enemy_x = 1110.0f;
    float switch_active = 0.0f;
    float collected_count = 0.0f;
    float game_state = 0.0f;
    float checkpoint_x = 90.0f;
    float deaths = 0.0f;
    float collected_mask = 0.0f;
};

class GameCore {
public:
    GameCore();

    void start();
    void set_input(bool left, bool right, bool jump);
    void update(float delta_seconds);
    void restart(bool from_complete);
    Snapshot snapshot() const;

private:
    static constexpr float kPlayerWidth = 28.0f;
    static constexpr float kPlayerHeight = 54.0f;
    static constexpr float kGravity = 1500.0f;
    static constexpr float kMoveSpeed = 255.0f;
    static constexpr float kJumpSpeed = -610.0f;
    static constexpr float kWorldEnd = 2820.0f;

    std::vector<Platform> platforms_;
    std::array<float, 4> hazards_{{900.0f, 1040.0f, 1500.0f, 2385.0f}};
    std::array<float, 3> shards_{{555.0f, 1370.0f, 1905.0f}};
    std::array<bool, 3> collected_{{false, false, false}};

    GameState state_ = GameState::Menu;
    float player_x_ = 90.0f;
    float player_y_ = 365.0f;
    float velocity_x_ = 0.0f;
    float velocity_y_ = 0.0f;
    float camera_x_ = 0.0f;
    float enemy_x_ = 1110.0f;
    float checkpoint_x_ = 90.0f;
    int deaths_ = 0;
    bool grounded_ = false;
    bool left_ = false;
    bool right_ = false;
    bool jump_ = false;
    bool jump_latched_ = false;
    bool switch_active_ = false;

    void clear_input();
    void die();
    void update_physics(float dt);
    void update_world_objects(float dt);
    bool is_collected(int index) const;
    int collected_count() const;
};

} // namespace hollow
