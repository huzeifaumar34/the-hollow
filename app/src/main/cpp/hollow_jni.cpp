#include <jni.h>
#include <cstdint>

#include "game_core.h"

namespace {
hollow::GameCore* from_handle(jlong handle) {
    return reinterpret_cast<hollow::GameCore*>(static_cast<std::uintptr_t>(handle));
}
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_hollow_game_NativeGameEngine_nativeCreate(JNIEnv*, jobject) {
    auto* game = new hollow::GameCore();
    return static_cast<jlong>(reinterpret_cast<std::uintptr_t>(game));
}

extern "C" JNIEXPORT void JNICALL
Java_com_hollow_game_NativeGameEngine_nativeDestroy(JNIEnv*, jobject, jlong handle) {
    delete from_handle(handle);
}

extern "C" JNIEXPORT void JNICALL
Java_com_hollow_game_NativeGameEngine_nativeStart(JNIEnv*, jobject, jlong handle) {
    if (auto* game = from_handle(handle)) game->start();
}

extern "C" JNIEXPORT void JNICALL
Java_com_hollow_game_NativeGameEngine_nativeSetInput(
    JNIEnv*, jobject, jlong handle, jboolean left, jboolean right, jboolean jump) {
    if (auto* game = from_handle(handle)) {
        game->set_input(left == JNI_TRUE, right == JNI_TRUE, jump == JNI_TRUE);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_hollow_game_NativeGameEngine_nativeUpdate(
    JNIEnv*, jobject, jlong handle, jfloat delta_seconds) {
    if (auto* game = from_handle(handle)) game->update(delta_seconds);
}

extern "C" JNIEXPORT void JNICALL
Java_com_hollow_game_NativeGameEngine_nativeRestart(
    JNIEnv*, jobject, jlong handle, jboolean from_complete) {
    if (auto* game = from_handle(handle)) {
        game->restart(from_complete == JNI_TRUE);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_hollow_game_NativeGameEngine_nativeNextChapter(JNIEnv*, jobject, jlong handle) {
    if (auto* game = from_handle(handle)) game->next_chapter();
}

extern "C" JNIEXPORT void JNICALL
Java_com_hollow_game_NativeGameEngine_nativeRestoreProgress(
    JNIEnv*, jobject, jlong handle, jfloat checkpoint_x, jint collected_mask,
    jboolean switch_active, jfloat crate_x, jint deaths, jint chapter_index) {
    if (auto* game = from_handle(handle)) {
        game->restore_progress(
            checkpoint_x,
            static_cast<int>(collected_mask),
            switch_active == JNI_TRUE,
            crate_x,
            static_cast<int>(deaths),
            static_cast<int>(chapter_index)
        );
    }
}

extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_hollow_game_NativeGameEngine_nativeSnapshot(
    JNIEnv* env, jobject, jlong handle) {
    jfloat values[16] = {};
    if (auto* game = from_handle(handle)) {
        const hollow::Snapshot s = game->snapshot();
        values[0] = s.player_x;
        values[1] = s.player_y;
        values[2] = s.velocity_x;
        values[3] = s.velocity_y;
        values[4] = s.grounded;
        values[5] = s.camera_x;
        values[6] = s.enemy_x;
        values[7] = s.switch_active;
        values[8] = s.collected_count;
        values[9] = s.game_state;
        values[10] = s.checkpoint_x;
        values[11] = s.deaths;
        values[12] = s.collected_mask;
        values[13] = s.crate_x;
        values[14] = s.crate_on_plate;
        values[15] = s.chapter_index;
    }
    jfloatArray result = env->NewFloatArray(16);
    if (result != nullptr) env->SetFloatArrayRegion(result, 0, 16, values);
    return result;
}
