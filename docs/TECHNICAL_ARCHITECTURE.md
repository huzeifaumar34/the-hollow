# THE HOLLOW — Technical Architecture

## Runtime split

The application uses a native-first gameplay architecture:

- **Kotlin / Android:** Activity lifecycle, touch input, screen scaling, Canvas rendering, local save storage and Android integration.
- **C++17 / Android NDK:** authoritative simulation state, movement, gravity, collision detection, hazards, enemy pressure, crate puzzle, collectibles, checkpoints, chapter progression and level completion.
- **JNI:** a narrow bridge that sends touch state and frame delta to C++, then returns a compact snapshot to the renderer.
- **CMake:** builds the shared native library `libhollow.so`.

Kotlin does not decide gameplay outcomes. It reads the native snapshot and draws what the native simulation reports.

## Native snapshot contract

The JNI snapshot contains 16 floats, in this order:

| Index | Meaning |
|---:|---|
| 0 | Player X |
| 1 | Player Y |
| 2 | Horizontal velocity |
| 3 | Vertical velocity |
| 4 | Grounded (0 or 1) |
| 5 | Camera X |
| 6 | Shadow enemy X |
| 7 | Pressure switch active (0 or 1) |
| 8 | Collected fragment count |
| 9 | Game state: menu 0, playing 1, dead 2, complete 3, chapter complete 4 |
| 10 | Checkpoint X |
| 11 | Death count |
| 12 | Fragment bit mask |
| 13 | Movable crate X |
| 14 | Crate positioned on pressure plate (0 or 1) |
| 15 | Chapter index (0–4) |

The fixed-size snapshot keeps JNI traffic small and avoids passing game objects across the language boundary.

## Campaign

The C++ campaign state machine currently contains five chapters:

1. The Dead Forest
2. The Ruins
3. The Factory
4. The Underground
5. The Void

Each chapter has its own hazard layout and visual theme. Chapter geometry and puzzle sequences are still being expanded; the five-stage framework is not yet a finished five-level game.

## Persistence

Android SharedPreferences stores the C++ state needed to continue a run: chapter index, checkpoint, collectible bit mask, switch state, crate position and death count. The app restores these values into the native core during view initialization.

## Build targets

- Android application: `app/`
- Native shared library: `app/src/main/cpp/`
- Host-side C++ tests: `tests/native/`
- GitHub Actions: `.github/workflows/android.yml`

Android builds include `armeabi-v7a`, `arm64-v8a` and `x86_64`. The 32-bit ARM target is retained for older devices.

## Design rules

1. Keep gameplay rules in C++.
2. Keep Android-specific APIs out of the native simulation.
3. Keep rendering separate from collision and state transitions.
4. Prefer deterministic, testable simulation functions.
5. Persist player progress locally and restore it into the native core at startup.
6. Add new level mechanics to the C++ core first, then expose only the presentation data needed by Kotlin.
