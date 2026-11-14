#include "game_core.h"

#include <algorithm>
#include <cmath>

namespace hollow {

GameCore::GameCore()
    : platforms_{
          {0.0f, 430.0f, 690.0f},
          {770.0f, 430.0f, 1160.0f},
          {1245.0f, 390.0f, 1585.0f},
          {1660.0f, 430.0f, 2180.0f},
          {2260.0f, 430.0f, kWorldEnd},
      } {}

void GameCore::start() {
    if (state_ == GameState::Menu) {
        state_ = GameState::Playing;
    }
}

void GameCore::set_input(bool left, bool right, bool jump) {
    left_ = left;
    right_ = right;
    jump_ = jump;
}

void GameCore::clear_input() {
    left_ = false;
    right_ = false;
    jump_ = false;
    jump_latched_ = false;
}

void GameCore::die() {
    if (state_ != GameState::Playing) return;
    state_ = GameState::Dead;
    ++deaths_;
    clear_input();
}

void GameCore::restart(bool from_complete) {
    if (from_complete) {
        checkpoint_x_ = 90.0f;
        collected_.fill(false);
        switch_active_ = false;
        enemy_x_ = 1110.0f;
        crate_x_ = 1720.0f;
        deaths_ = 0;
    }

    player_x_ = checkpoint_x_;
    player_y_ = checkpoint_x_ > 1000.0f ? 336.0f : 365.0f;
    velocity_x_ = 0.0f;
    velocity_y_ = 0.0f;
    grounded_ = false;
    enemy_x_ = std::max(1110.0f, checkpoint_x_ - 160.0f);
    camera_x_ = std::max(0.0f, player_x_ - 330.0f);
    state_ = GameState::Playing;
    clear_input();
}

void GameCore::update(float delta_seconds) {
    if (state_ != GameState::Playing) return;
    const float dt = std::clamp(delta_seconds, 0.0f, 0.033f);
    if (dt <= 0.0f) return;

    update_physics(dt);
    if (state_ != GameState::Playing) return;
    update_world_objects(dt);
}

void GameCore::update_physics(float dt) {
    const float old_bottom = player_y_ + kPlayerHeight;
    const float old_player_x = player_x_;

    if (left_ && !right_) {
        velocity_x_ = -kMoveSpeed;
    } else if (right_ && !left_) {
        velocity_x_ = kMoveSpeed;
    } else {
        velocity_x_ = 0.0f;
    }

    if (jump_ && !jump_latched_ && grounded_) {
        velocity_y_ = kJumpSpeed;
        grounded_ = false;
    }
    jump_latched_ = jump_;

    player_x_ = std::max(0.0f, player_x_ + velocity_x_ * dt);
    player_y_ += velocity_y_ * dt;
    velocity_y_ += kGravity * dt;
    grounded_ = false;

    const float new_bottom = player_y_ + kPlayerHeight;
    if (velocity_y_ >= 0.0f) {
        for (const Platform& platform : platforms_) {
            const bool overlaps = player_x_ + kPlayerWidth > platform.left &&
                                  player_x_ < platform.right;
            if (overlaps && old_bottom <= platform.top + 8.0f &&
                new_bottom >= platform.top) {
                player_y_ = platform.top - kPlayerHeight;
                velocity_y_ = 0.0f;
                grounded_ = true;
                break;
            }
        }
    }

    if (player_x_ > 1735.0f && player_x_ < 1835.0f && grounded_) {
        switch_active_ = true;
    }

    constexpr float gate_x = 2115.0f;
    if (!switch_active_ && player_x_ + kPlayerWidth > gate_x &&
        player_x_ < gate_x + 42.0f) {
        player_x_ = gate_x - kPlayerWidth;
        velocity_x_ = 0.0f;
    }

    for (float hazard_x : hazards_) {
        const bool overlaps = player_x_ + kPlayerWidth > hazard_x &&
                              player_x_ < hazard_x + 46.0f;
        const float hazard_top = (hazard_x == 1500.0f) ? 350.0f : 390.0f;
        if (overlaps && player_y_ + kPlayerHeight > hazard_top &&
            player_y_ < hazard_top + 60.0f) {
            die();
            return;
        }
    }

    if (player_y_ > 650.0f) {
        die();
        return;
    }

    if (player_x_ > 1220.0f && checkpoint_x_ < 1200.0f) {
        checkpoint_x_ = 1270.0f;
    }

    if (player_x_ > 2680.0f) {
        state_ = GameState::Complete;
        clear_input();
    }

    camera_x_ = std::clamp(player_x_ - 330.0f, 0.0f, 1900.0f);
}

void GameCore::update_world_objects(float dt) {
    if (player_x_ > 1180.0f) {
        enemy_x_ += 205.0f * dt;
        if (player_x_ - enemy_x_ < 46.0f) {
            die();
            return;
        }
    }

    for (std::size_t i = 0; i < shards_.size(); ++i) {
        if (collected_[i]) continue;
        const float player_center_x = player_x_ + kPlayerWidth * 0.5f;
        const float player_center_y = player_y_ + kPlayerHeight * 0.5f;
        if (std::abs(player_center_x - shards_[i]) < 38.0f &&
            std::abs(player_center_y - 365.0f) < 100.0f) {
            collected_[i] = true;
        }
    }
}

bool GameCore::is_collected(int index) const {
    return index >= 0 && index < static_cast<int>(collected_.size()) &&
           collected_[static_cast<std::size_t>(index)];
}

int GameCore::collected_count() const {
    int count = 0;
    for (bool value : collected_) {
        if (value) ++count;
    }
    return count;
}

Snapshot GameCore::snapshot() const {
    int mask = 0;
    for (int i = 0; i < static_cast<int>(collected_.size()); ++i) {
        if (is_collected(i)) mask |= (1 << i);
    }

    Snapshot result;
    result.player_x = player_x_;
    result.player_y = player_y_;
    result.velocity_x = velocity_x_;
    result.velocity_y = velocity_y_;
    result.grounded = grounded_ ? 1.0f : 0.0f;
    result.camera_x = camera_x_;
    result.enemy_x = enemy_x_;
    result.switch_active = switch_active_ ? 1.0f : 0.0f;
    result.collected_count = static_cast<float>(collected_count());
    result.game_state = static_cast<float>(static_cast<int>(state_));
    result.checkpoint_x = checkpoint_x_;
    result.deaths = static_cast<float>(deaths_);
    result.collected_mask = static_cast<float>(mask);
    result.crate_x = crate_x_;
    result.crate_on_plate =
        (crate_x_ + 21.0f >= 1760.0f && crate_x_ + 21.0f <= 1815.0f) ? 1.0f : 0.0f;
    return result;
}

} // namespace hollow
