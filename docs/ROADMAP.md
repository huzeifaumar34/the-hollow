# THE HOLLOW — Roadmap

## Current milestone: 0.3.0 — Native campaign foundation

- [x] Native Android project and landscape activity
- [x] Kotlin Canvas renderer and touch controls
- [x] C++17 gameplay core built with Android NDK and CMake
- [x] JNI bridge with compact native state snapshots
- [x] Platforms, gaps, spikes, gravity and landing collision
- [x] Three collectible fragments per chapter
- [x] Pushable crate and pressure-plate gate puzzle
- [x] Pursuing shadow enemy
- [x] Checkpoint and death/retry loop
- [x] Save and restore chapter, checkpoint, fragments, puzzle state and death count
- [x] Five-chapter campaign state machine and chapter transitions
- [x] Per-chapter atmosphere and hazard layouts
- [x] Host-side C++ tests for movement, jump, pickups, spike collision, crate puzzle, save restore and campaign progression
- [x] GitHub Actions workflow: native tests first, then multi-ABI debug APK

## Milestone 0.4 — Distinct level design

- [ ] Give each chapter bespoke platform geometry and puzzle sequences
- [ ] Add moving platforms, lifts and timed machinery
- [ ] Add chapter-specific enemy behaviours and encounters
- [ ] Add hidden routes and optional memory fragments
- [ ] Add checkpoints at deliberate narrative beats

## Milestone 0.5 — Art, animation and sound

- [ ] Replace geometric placeholders with original silhouette artwork
- [ ] Add character animation states and smoother movement
- [ ] Add layered parallax, fog, particles and lighting
- [ ] Compose original ambient music and sound effects
- [ ] Add haptics, accessibility and graphics settings

## Milestone 0.6 — Narrative and menus

- [ ] Add title, pause, settings and credits screens
- [ ] Tell the story through environmental details and short visual sequences
- [ ] Add chapter introductions and endings
- [ ] Add a proper save-slot and progress screen

## Milestone 1.0 — Release candidate

- [ ] Complete and balance all five chapters
- [ ] Full playthrough and regression testing
- [ ] Test on ARMv7 and arm64 physical Android devices
- [ ] Profile memory, battery use and frame pacing
- [ ] Produce a signed release APK and release notes

All art, sound, levels and code will be original. The project may take broad atmospheric inspiration from monochrome puzzle-platformers, but will not reuse another game's characters, levels, assets or audio.
