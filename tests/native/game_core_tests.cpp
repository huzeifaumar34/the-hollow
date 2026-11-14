#include "game_core.h"

#include <cassert>
#include <cmath>
#include <iostream>

namespace {
void tick(hollow::GameCore& game, int frames, float dt = 1.0f / 60.0f) {
    for (int i = 0; i < frames; ++i) game.update(dt);
}

void test_menu_and_start() {
    hollow::GameCore game;
    assert(static_cast<int>(game.snapshot().game_state) == 0);
    game.start();
    assert(static_cast<int>(game.snapshot().game_state) == 1);
}

void test_landing_and_jump() {
    hollow::GameCore game;
    game.start();
    tick(game, 20);
    auto grounded = game.snapshot();
    assert(grounded.grounded > 0.5f);
    const float before_jump = grounded.player_y;

    game.set_input(false, false, true);
    tick(game, 1);
    game.set_input(false, false, false);
    const auto airborne = game.snapshot();
    assert(airborne.velocity_y < 0.0f);
    assert(airborne.player_y < before_jump);
}

void test_horizontal_movement() {
    hollow::GameCore game;
    game.start();
    tick(game, 20);
    const float initial_x = game.snapshot().player_x;
    game.set_input(false, true, false);
    tick(game, 20);
    game.set_input(false, false, false);
    assert(game.snapshot().player_x > initial_x + 50.0f);
}

void test_fragment_collection() {
    hollow::GameCore game;
    game.start();
    tick(game, 20);
    game.set_input(false, true, false);
    tick(game, 68, 0.033f);
    game.set_input(false, false, false);
    const auto result = game.snapshot();
    assert(result.collected_count >= 1.0f);
    assert((static_cast<int>(result.collected_mask) & 1) != 0);
}

void test_restart_from_checkpoint() {
    hollow::GameCore game;
    game.start();
    game.restart(false);
    const auto result = game.snapshot();
    assert(static_cast<int>(result.game_state) == 1);
    assert(std::abs(result.player_x - 90.0f) < 0.01f);
}

void test_progress_restore() {
    hollow::GameCore game;
    game.restore_progress(1270.0f, 5, true, 1780.0f, 4);
    const auto restored = game.snapshot();
    assert(std::abs(restored.checkpoint_x - 1270.0f) < 0.01f);
    assert(std::abs(restored.player_x - 1270.0f) < 0.01f);
    assert(restored.collected_count == 2.0f);
    assert(restored.switch_active > 0.5f);
    assert(std::abs(restored.crate_x - 1780.0f) < 0.01f);
    assert(restored.deaths == 4.0f);
}

} // namespace

int main() {
    test_menu_and_start();
    test_landing_and_jump();
    test_horizontal_movement();
    test_fragment_collection();
    test_restart_from_checkpoint();
    test_progress_restore();
    std::cout << "THE HOLLOW native core tests passed\n";
    return 0;
}
