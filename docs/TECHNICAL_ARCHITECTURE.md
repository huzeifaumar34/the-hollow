# THE HOLLOW — Technical Architecture

## Runtime split

The application uses a small native-first gameplay architecture:

- **Kotlin / Android:** Activity lifecycle, touch input, screen scaling, Canvas rendering, and Android integration.
- **C++17 / Android NDK:** authoritative simulation state, movement, gravity, collision detection, hazards, enemy pressure, collectibles, checkpoint, and level completion.
- **JNI:** a narrow bridge that sends touch state and frame delta to C++, then returns a compact snapshot to the renderer.
- **CMake:** builds the shared native library `libhollow.so`.

Kotlin does not decide gameplay outcomes. It reads the native snapshot and draws what the native simulation reports.

## Native snapshot contract

The JNI snapshot contains 15 floats, in this order:

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
| 9 | Game state: menu 0, playing 1, dead 2, complete 3 |
| 10 | Checkpoint X |
| 11 | Death count |
| 12 | Fragment bit mask |
| 13 | Movable crate X |
| 14 | Crate positioned on pressure plate (0 or 1) |

The fixed-size snapshot keeps JNI traffic small and avoids passing game objects across the language boundary.

## Build targets

- Android application: `app/`
- Native shared library: `app/src/main/cpp/`
- Host-side C++ tests: `tests/native/`
- GitHub Actions: `.github/workflows/android.yml`

Android builds include `armeabi-v7a`, `arm64-v8a`, and `x86_64`. The 32-bit ARM target is retained for older devices.

## Design rules

1. Keep gameplay rules in C++.
2. Keep Android-specific APIs out of the native simulation.
3. Keep rendering separate from collision and state transitions.
4. Prefer deterministic, testable simulation functions.
5. Persist checkpoint, fragment, switch, crate, and death progress with Android SharedPreferences; restore that data into C++ at startup.
6. Add new level mechanics to the C++ core first, then expose only the presentation data needed by Kotlin.
