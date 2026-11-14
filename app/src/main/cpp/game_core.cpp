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
      } {
    configure_chapter();
}

void GameCore::configure_chapter() {
    switch (chapter_index_) {
        case 0:
            platforms_ = {
                {0.0f, 430.0f, 690.0f},
                {770.0f, 430.0f, 1160.0f},
                {1245.0f, 390.0f, 1585.0f},
                {1660.0f, 430.0f, 2180.0f},
                {2260.0f, 430.0f, kWorldEnd},
            };
            hazards_ = {{900.0f, 1040.0f, 1500.0f, 2385.0f}};
            break;
        case 1:
            platforms_ = {
                {0.0f, 430.0f, 560.0f},
                {640.0f, 400.0f, 1050.0f},
                {1140.0f, 430.0f, 1510.0f},
                {1600.0f, 430.0f, 2180.0f},
                {2260.0f, 430.0f, kWorldEnd},
            };
            hazards_ = {{850.0f, 980.0f, 1380.0f, 2370.0f}};
            break;
        case 2:
            platforms_ = {
                {0.0f, 430.0f, 760.0f},
                {850.0f, 390.0f, 1230.0f},
                {1320.0f, 430.0f, 1590.0f},
                {1660.0f, 430.0f, 2180.0f},
                {2260.0f, 430.0f, kWorldEnd},
            };
            hazards_ = {{930.0f, 1110.0f, 1515.0f, 2450.0f}};
            break;
        case 3:
            platforms_ = {
                {0.0f, 430.0f, 520.0f},
                {610.0f, 390.0f, 980.0f},
                {1070.0f, 430.0f, 1420.0f},
                {1510.0f, 390.0f, 1600.0f},
                {1660.0f, 430.0f, 2180.0f},
                {2260.0f, 430.0f, kWorldEnd},
            };
            hazards_ = {{820.0f, 910.0f, 1210.0f, 2300.0f}};
            break;
        default:
            platforms_ = {
                {0.0f, 430.0f, 450.0f},
                {540.0f, 370.0f, 820.0f},
                {900.0f, 430.0f, 1220.0f},
                {1300.0f, 370.0f, 1585.0f},
                {1660.0f, 430.0f, 2180.0f},
                {2260.0f, 430.0f, kWorldEnd},
            };
            hazards_ = {{600.0f, 740.0f, 1430.0f, 2490.0f}};
            break;
    }
}

float GameCore::platform_top_at(float x) const {
    for (const Platform& platform : platforms_) {
        if (x >= platform.left && x < platform.right) {
            return platform.top;
        }
    }
    return 430.0f;
}

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
        chapter_index_ = 0;
        checkpoint_x_ = 90.0f;
        collected_.fill(false);
        switch_active_ = false;
        enemy_x_ = 1110.0f;
        crate_x_ = 1720.0f;
        deaths_ = 0;
        configure_chapter();
    }

    player_x_ = checkpoint_x_;
    player_y_ = platform_top_at(checkpoint_x_) - kPlayerHeight;
    velocity_x_ = 0.0f;
    velocity_y_ = 0.0f;
    grounded_ = false;
    enemy_x_ = std::max(1110.0f, checkpoint_x_ - 160.0f);
    camera_x_ = std::max(0.0f, player_x_ - 330.0f);
    state_ = GameState::Playing;
    clear_input();
}

void GameCore::next_chapter() {
    if (state_ != GameState::ChapterComplete || chapter_index_ >= 4) return;

    ++chapter_index_;
    checkpoint_x_ = 90.0f;
    player_x_ = 90.0f;
    player_y_ = 365.0f;
    velocity_x_ = 0.0f;
    velocity_y_ = 0.0f;
    camera_x_ = 0.0f;
    enemy_x_ = 1110.0f;
    crate_x_ = 1720.0f;
    switch_active_ = false;
    collected_.fill(false);
    grounded_ = false;
    state_ = GameState::Playing;
    configure_chapter();
    clear_input();
}

void GameCore::restore_progress(
    float checkpoint_x,
    int collected_mask,
    bool switch_active,
    float crate_x,
    int deaths,
    int chapter_index
) {
    chapter_index_ = std::clamp(chapter_index, 0, 4);
    configure_chapter();
    checkpoint_x_ = std::clamp(checkpoint_x, 90.0f, 2700.0f);
    crate_x_ = std::clamp(crate_x, 1720.0f, 2070.0f);
    switch_active_ = switch_active;
    deaths_ = std::max(0, deaths);

    for (int i = 0; i < static_cast<int>(collected_.size()); ++i) {
        collected_[static_cast<std::size_t>(i)] = (collected_mask & (1 << i)) != 0;
    }

    player_x_ = checkpoint_x_;
    player_y_ = checkpoint_x_ > 1000.0f ? 336.0f : 365.0f;
    velocity_x_ = 0.0f;
    velocity_y_ = 0.0f;
    grounded_ = false;
    camera_x_ = std::clamp(player_x_ - 330.0f, 0.0f, 1900.0f);
    enemy_x_ = std::max(1110.0f, checkpoint_x_ - 160.0f);
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

    // Push the crate while grounded. The plate is activated by the crate,
    // not by the player walking across it.
    constexpr float kCrateWidth = 42.0f;
    if (!switch_active_ && grounded_ && velocity_x_ > 0.0f &&
        old_player_x + kPlayerWidth <= crate_x_ + 2.0f &&
        player_x_ + kPlayerWidth > crate_x_) {
        const float push_distance = player_x_ + kPlayerWidth - crate_x_;
        crate_x_ = std::min(crate_x_ + push_distance, 2070.0f);
        player_x_ = crate_x_ - kPlayerWidth;
    }

    const bool crate_over_plate =
        crate_x_ + kCrateWidth * 0.5f >= 1760.0f &&
        crate_x_ + kCrateWidth * 0.5f <= 1815.0f;
    if (crate_over_plate) {
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
        const float hazard_top = platform_top_at(hazard_x) - 40.0f;
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
        switch (chapter_index_) {
            case 0: checkpoint_x_ = 1270.0f; break;
            case 1: checkpoint_x_ = 1180.0f; break;
            case 2: checkpoint_x_ = 1400.0f; break;
            case 3: checkpoint_x_ = 1200.0f; break;
            default: checkpoint_x_ = 1420.0f; break;
        }
    }

    if (player_x_ > 2680.0f) {
        state_ = chapter_index_ < 4 ? GameState::ChapterComplete : GameState::Complete;
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

#ifdef HOLLOW_TESTING
void GameCore::debug_set_player(float x, float y, bool grounded) {
    player_x_ = x;
    player_y_ = y;
    grounded_ = grounded;
    velocity_x_ = 0.0f;
    velocity_y_ = 0.0f;
}
#endif

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
    result.chapter_index = static_cast<float>(chapter_index_);
    return result;
}

} // namespace hollow
