# THE HOLLOW

An original monochrome 2D atmospheric puzzle-platformer for Android.

> "Do not follow the light. Listen to the silence."

## Current build

**Version:** 0.3.0 — native campaign foundation  
**Package:** `com.hollow.game` (debug builds use `com.hollow.game.debug`)  
**Minimum Android:** 6.0 (API 23)  
**Orientation:** Landscape  
**Technology:** Kotlin, C++17, JNI, Android NDK, CMake, Canvas

### Implemented

- Monochrome 2D platformer with a silhouette protagonist
- Native C++ simulation for movement, gravity, collision and hazards
- Touch movement and jump controls
- Five chapters: forest, ruins, factory, underground and the void
- Chapter-specific platform geometry, platform heights and hazard layouts
- Checkpoints positioned for each chapter's platform geometry
- Pushable crate and pressure-plate gate puzzle
- Collectible fragments and a pursuing shadow
- Death/retry loop and local progress saving
- C++ tests for movement, jump, fragments, hazards, crate activation, save restore, checkpoints and chapter transitions

The five-chapter campaign framework and first-pass routes are implemented. Each chapter still needs bespoke puzzle sequences, polished original artwork, audio and narrative content; this is not yet the finished game.

## Controls

- Bottom-left button: move left
- Next button: move right
- Bottom-right button: jump
- Tap the title screen to start
- Tap after death to retry
- Tap the chapter-complete screen to continue

## Build the APK

Every push to `main`, pull request to `main`, or manual workflow dispatch runs **Android APK** in GitHub Actions.

1. Open the repository's **Actions** tab.
2. Select **Android APK** and open the latest successful run.
3. Download the `THE-HOLLOW-debug-apk` artifact.
4. Extract the ZIP and install `app-debug.apk` on an Android device.

The workflow uses JDK 17, Gradle 8.9, Android Gradle Plugin 8.7.3, Kotlin 2.0.21, Android SDK 35, NDK 27.0.12077973 and CMake 3.22.1.

## Project structure

```text
app/
  src/main/
    cpp/
      game_core.h
      game_core.cpp
      hollow_jni.cpp
      CMakeLists.txt
    java/com/hollow/game/
      MainActivity.kt
      HollowGameView.kt
      NativeGameEngine.kt
    res/values/styles.xml
    AndroidManifest.xml
tests/native/
  game_core_tests.cpp
.github/workflows/android.yml
docs/
  ROADMAP.md
  TECHNICAL_ARCHITECTURE.md
```

## Roadmap

See [docs/ROADMAP.md](docs/ROADMAP.md) for implementation status and next milestones.

## Creative direction

THE HOLLOW is an original project. It takes broad inspiration from the quiet tension and environmental storytelling of monochrome puzzle-platformers, but uses its own characters, puzzles, levels, artwork and sound.
