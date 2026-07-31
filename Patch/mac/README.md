# Dark Eden - macOS Test Build

This is a first test build for macOS (Apple Silicon / arm64). It is **not** the
final distribution package - see "Known limitations" below.

## Setup

1. **Clone with git rather than downloading a ZIP from the GitHub web UI.**
   Files downloaded through a browser get macOS's "quarantine" flag, which
   makes Gatekeeper block `DarkEden`/`Updater` from running on first launch
   ("cannot be opened because the developer cannot be verified"). A `git
   clone` doesn't set that flag, so this sidesteps the warning entirely:
   ```
   git clone git@github.com:mikelancev2/darkeden-client-mac.git
   cd darkeden-client-mac/Patch/mac
   ```
   (If you do end up with a quarantined copy some other way, `xattr -d
   com.apple.quarantine DarkEden Updater` clears it - one-time, no
   Apple Developer account needed. A fully "just works" download for
   random players would need Apple notarization, which isn't set up yet.)

2. **Install SDL2** (the game links against it dynamically, same as the Windows
   build's `SDL2.dll`/`SDL2_image.dll`/`SDL2_ttf.dll`):
   ```
   brew install sdl2 sdl2_image sdl2_ttf
   ```
   (If you don't have Homebrew: https://brew.sh)

3. **Download the game data.** This folder only has the program itself - the
   maps/sprites/sounds/fonts (~1.7GB, same files the Windows client uses) are
   too big for git, so they're attached to this repo's Releases instead:
   https://github.com/mikelancev2/darkeden-client-mac/releases/tag/data-mac-v1

   Download `Data-mac.zip` from there, unzip it, and put the resulting `Data`
   folder in this same folder (next to `DarkEden` and `Updater`), so you have:
   ```
   mac/
     DarkEden
     Updater
     ui/            <- Updater's window (HTML/CSS), loaded relative to it - keep together
     Data/
     dk2.cfg
     ttdk2.cfg
     version.dat
   ```

4. **Run it:**
   ```
   ./Updater
   ```
   This opens a window (background/logo/news matching the site), checks
   `https://darkedenclassic.com/patch2/PatchList.dat` for anything new,
   downloads whatever changed, then Play launches the game. If the update
   server is unreachable it just plays with whatever's on disk instead of
   blocking you.

   (Launching `./DarkEden` directly also still works, no Updater required -
   there's no "must go through the Updater" restriction yet, that's still
   being decided.)

## Known limitations (this is a first pass, not the final thing)

- **Requires Homebrew SDL2**, since the binary isn't self-contained yet
  (the Windows build ships `SDL2.dll` etc. right next to `darkeden.exe`;
  the macOS equivalent - bundling `.dylib`s so end users don't need
  Homebrew - hasn't been done yet).
- **Not a signed/notarized `.app`** - see the Gatekeeper note in step 1.
  Fine for testing, not for handing to a random player yet.
- Built for **Apple Silicon (arm64)**. An Intel Mac would need its own build
  (change the GitHub Actions runner, e.g. `macos-13`, and rebuild).
- Updater's nav buttons only link to pages that exist today (Home, Ranking,
  Create Account, Downloads) - Shop and Discord are left out until there's
  a real shop page and the Discord invite in the site's config is filled in.

## If something crashes or won't start

Check `Log/darkeden.log` and `Log/ui_debug.log` (created next to the binary
after the first run) and share them - same log files the Windows build
writes, same format.
