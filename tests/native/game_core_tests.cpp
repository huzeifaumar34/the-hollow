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
    game.restore_progress(1400.0f, 5, true, 1780.0f, 4, 2);
    const auto restored = game.snapshot();
    assert(std::abs(restored.checkpoint_x - 1400.0f) < 0.01f);
    assert(std::abs(restored.player_x - 1400.0f) < 0.01f);
    assert(restored.collected_count == 2.0f);
    assert(restored.switch_active > 0.5f);
    assert(std::abs(restored.crate_x - 1780.0f) < 0.01f);
    assert(restored.deaths == 4.0f);
    assert(restored.chapter_index == 2.0f);
}

void test_box_activates_pressure_plate() {
    hollow::GameCore game;
    game.start();
    game.debug_set_player(1660.0f, 376.0f, true);
    game.set_input(false, true, false);
    tick(game, 10, 0.033f);
    const auto result = game.snapshot();
    assert(result.crate_x > 1730.0f);
    assert(result.crate_on_plate > 0.5f);
    assert(result.switch_active > 0.5f);
}

void test_factory_spike_kills_player() {
    hollow::GameCore game;
    game.start();
    game.debug_set_player(1490.0f, 336.0f, true);
    tick(game, 1, 1.0f / 60.0f);
    const auto result = game.snapshot();
    assert(static_cast<int>(result.game_state) == 2);
    assert(result.deaths == 1.0f);
}

void test_five_chapter_campaign() {
    hollow::GameCore game;
    game.start();

    for (int chapter = 0; chapter < 4; ++chapter) {
        game.debug_set_player(2681.0f, 376.0f, true);
        tick(game, 1, 1.0f / 60.0f);
        const auto chapter_end = game.snapshot();
        assert(static_cast<int>(chapter_end.game_state) == 4);
        assert(chapter_end.chapter_index == static_cast<float>(chapter));

        game.next_chapter();
        const auto next = game.snapshot();
        assert(static_cast<int>(next.game_state) == 1);
        assert(next.chapter_index == static_cast<float>(chapter + 1));
        assert(std::abs(next.player_x - 90.0f) < 0.01f);
    }

    game.debug_set_player(2681.0f, 376.0f, true);
    tick(game, 1, 1.0f / 60.0f);
    const auto final = game.snapshot();
    assert(static_cast<int>(final.game_state) == 3);
    assert(final.chapter_index == 4.0f);
}

void test_chapter_checkpoints_match_platform_heights() {
    const float expected_x[5] = {1270.0f, 1180.0f, 1400.0f, 1200.0f, 1420.0f};
    const float expected_y[5] = {336.0f, 376.0f, 376.0f, 376.0f, 316.0f};

    hollow::GameCore game;
    game.start();
    for (int chapter = 0; chapter < 5; ++chapter) {
        game.debug_set_player(1700.0f, 376.0f, true);
        tick(game, 1, 1.0f / 60.0f);
        game.restart(false);
        const auto respawn = game.snapshot();
        assert(std::abs(respawn.checkpoint_x - expected_x[chapter]) < 0.01f);
        assert(std::abs(respawn.player_x - expected_x[chapter]) < 0.01f);
        assert(std::abs(respawn.player_y - expected_y[chapter]) < 0.01f);

        if (chapter < 4) {
            game.debug_set_player(2681.0f, 376.0f, true);
            tick(game, 1, 1.0f / 60.0f);
            game.next_chapter();
        }
    }
}

} // namespace

int main() {
    test_menu_and_start();
    test_landing_and_jump();
    test_horizontal_movement();
    test_fragment_collection();
    test_restart_from_checkpoint();
    test_progress_restore();
    test_box_activates_pressure_plate();
    test_factory_spike_kills_player();
    test_five_chapter_campaign();
    test_chapter_checkpoints_match_platform_heights();
    std::cout << "THE HOLLOW native core tests passed\n";
    return 0;
}
