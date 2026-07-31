# Dark Eden - macOS Test Build

This is a first test build for macOS (Apple Silicon / arm64). It is **not** the
final distribution package - see "Known limitations" below.

## Setup

1. **Install SDL2** (the game links against it dynamically, same as the Windows
   build's `SDL2.dll`/`SDL2_image.dll`/`SDL2_ttf.dll`):
   ```
   brew install sdl2 sdl2_image sdl2_ttf
   ```
   (If you don't have Homebrew: https://brew.sh)

2. **Download the game data.** This folder only has the program itself - the
   maps/sprites/sounds/fonts (~1.7GB, same files the Windows client uses) are
   too big for git, so they're attached to this repo's Releases instead:
   https://github.com/mikelancev2/darkeden-client-mac/releases/tag/data-mac-v1

   Download `Data-mac.zip` from there, unzip it, and put the resulting `Data`
   folder in this same folder (next to `DarkEden` and `Updater`), so you have:
   ```
   mac/
     DarkEden
     Updater
     Data/
     dk2.cfg
     ttdk2.cfg
     version.dat
   ```

3. **Make the binaries executable** (git doesn't always preserve the
   executable bit through zip/download):
   ```
   chmod +x DarkEden Updater
   ```

4. **Run it:**
   ```
   ./Updater
   ```
   (Launching `./DarkEden` directly also works for now - there's no
   "must go through the Updater" restriction yet, that's still being decided.)

## Known limitations (this is a first pass, not the final thing)

- **No update checking yet.** The Updater just launches the client - it
  doesn't download/verify patches from a server yet, unlike the real Windows
  Updater (`UpdaterPronto/`). That comes once there's a live update endpoint
  for macOS.
- **Requires Homebrew SDL2**, since the binary isn't self-contained yet
  (the Windows build ships `SDL2.dll` etc. right next to `darkeden.exe`;
  the macOS equivalent - bundling `.dylib`s so end users don't need
  Homebrew - hasn't been done yet).
- Built for **Apple Silicon (arm64)**. An Intel Mac would need its own build
  (change the GitHub Actions runner, e.g. `macos-13`, and rebuild).

## If something crashes or won't start

Check `Log/darkeden.log` and `Log/ui_debug.log` (created next to the binary
after the first run) and share them - same log files the Windows build
writes, same format.
