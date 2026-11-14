# THE HOLLOW

An original monochrome 2D atmospheric puzzle-platformer for Android.

> "Do not follow the light. Listen to the silence."

## Current build

**Version:** 0.1.0 — first playable prototype  
**Package:** `com.hollow.game` (debug builds use `com.hollow.game.debug`)  
**Minimum Android:** 6.0 (API 23)  
**Orientation:** Landscape  
**Technology:** Kotlin, Android Canvas, Gradle

The current prototype includes:
- A monochrome forest scene and silhouette protagonist
- Touch movement and jump controls
- Platforms, gaps, spikes, and basic landing collision
- Three collectible fragments
- A pressure plate that opens a gate
- A pursuing shadow, death/retry, and a checkpoint
- A simple first-section ending

All visuals are drawn with Android Canvas. No external game engine, paid SDK, or third-party game assets are required.

## Controls

- Bottom-left button: move left
- Next button: move right
- Bottom-right button: jump
- Tap the title screen to start
- Tap after death to retry

## Build the APK

Every push to `main`, pull request to `main`, or manual workflow dispatch runs **Android APK** in GitHub Actions.

1. Open the repository's **Actions** tab.
2. Select **Android APK** and open the latest successful run.
3. Download the `THE-HOLLOW-debug-apk` artifact.
4. Extract the ZIP and install `app-debug.apk` on an Android device.

The workflow uses JDK 17, Gradle 8.9, Android Gradle Plugin 8.7.3, Kotlin 2.0.21, and Android SDK 35.

## Project structure

```text
app/
  src/main/
    java/com/hollow/game/
      MainActivity.kt
      HollowGameView.kt
    res/values/styles.xml
    AndroidManifest.xml
.github/workflows/android.yml
docs/ROADMAP.md
```

## Roadmap

See [docs/ROADMAP.md](docs/ROADMAP.md) for planned milestones, from gameplay feel and original art/audio to a complete story and release candidate.

## Creative direction

THE HOLLOW is an original project. It takes broad inspiration from the quiet tension and environmental storytelling of monochrome puzzle-platformers, but will use its own characters, puzzles, levels, artwork, and sound.
