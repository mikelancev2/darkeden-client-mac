#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_BIN="${BUILD_BIN:-$ROOT_DIR/build/release/bin}"
PATCH_MAC="${PATCH_MAC:-$ROOT_DIR/Patch/mac}"
PACKAGE_DIR="${PACKAGE_DIR:-$ROOT_DIR/build/package/macos}"
APP_NAME="${APP_NAME:-DarkEden Classic.app}"
ZIP_NAME="${ZIP_NAME:-DarkEden-Classic-macOS-arm64.zip}"

APP_DIR="$PACKAGE_DIR/$APP_NAME"
CONTENTS_DIR="$APP_DIR/Contents"
MACOS_DIR="$CONTENTS_DIR/MacOS"
FRAMEWORKS_DIR="$CONTENTS_DIR/Frameworks"
RESOURCES_DIR="$CONTENTS_DIR/Resources"

require_file() {
	local path="$1"
	if [[ ! -f "$path" ]]; then
		echo "Missing required file: $path" >&2
		exit 1
	fi
}

copy_if_exists() {
	local src="$1"
	local dst="$2"
	if [[ -e "$src" ]]; then
		cp -R "$src" "$dst"
	fi
}

add_rpath_if_missing() {
	local binary="$1"
	if ! otool -l "$binary" | grep -q "@executable_path/../Frameworks"; then
		install_name_tool -add_rpath "@executable_path/../Frameworks" "$binary" 2>/dev/null || true
	fi
}

is_external_dylib() {
	local dep="$1"
	[[ "$dep" == /opt/homebrew/* || "$dep" == /usr/local/* ]]
}

COPIED_DYLIBS="|"

has_copied_dylib() {
	local base="$1"
	[[ "$COPIED_DYLIBS" == *"|$base|"* ]]
}

mark_copied_dylib() {
	local base="$1"
	COPIED_DYLIBS="${COPIED_DYLIBS}${base}|"
}

copy_dylib_tree() {
	local binary="$1"
	local deps
	deps="$(otool -L "$binary" | awk 'NR > 1 { print $1 }')"

	while IFS= read -r dep; do
		[[ -z "$dep" ]] && continue
		if ! is_external_dylib "$dep"; then
			continue
		fi

		local base
		base="$(basename "$dep")"
		local dst="$FRAMEWORKS_DIR/$base"

		if ! has_copied_dylib "$base"; then
			echo "Bundling dylib: $dep"
			cp -L "$dep" "$dst"
			chmod u+w "$dst"
			install_name_tool -id "@rpath/$base" "$dst" 2>/dev/null || true
			add_rpath_if_missing "$dst"
			mark_copied_dylib "$base"
			copy_dylib_tree "$dst"
		fi

		install_name_tool -change "$dep" "@rpath/$base" "$binary" 2>/dev/null || true
	done <<< "$deps"
}

echo "Packaging DarkEden Classic macOS app"
echo "Root:      $ROOT_DIR"
echo "Build bin: $BUILD_BIN"
echo "Patch mac: $PATCH_MAC"
echo "Output:    $PACKAGE_DIR"

require_file "$BUILD_BIN/DarkEden"
require_file "$BUILD_BIN/Updater"

rm -rf "$PACKAGE_DIR"
mkdir -p "$MACOS_DIR" "$FRAMEWORKS_DIR" "$RESOURCES_DIR"

cp "$BUILD_BIN/DarkEden" "$MACOS_DIR/DarkEden"
cp "$BUILD_BIN/Updater" "$MACOS_DIR/Updater"
chmod +x "$MACOS_DIR/DarkEden" "$MACOS_DIR/Updater"

if [[ -d "$BUILD_BIN/ui" ]]; then
	cp -R "$BUILD_BIN/ui" "$MACOS_DIR/ui"
else
	cp -R "$PATCH_MAC/ui" "$MACOS_DIR/ui"
fi

copy_if_exists "$PATCH_MAC/dk2.cfg" "$MACOS_DIR/"
copy_if_exists "$PATCH_MAC/ttdk2.cfg" "$MACOS_DIR/"
copy_if_exists "$PATCH_MAC/version.dat" "$MACOS_DIR/"
copy_if_exists "$PATCH_MAC/Data" "$MACOS_DIR/"

cat > "$CONTENTS_DIR/Info.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
 "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleDevelopmentRegion</key>
	<string>en</string>
	<key>CFBundleDisplayName</key>
	<string>DarkEden Classic</string>
	<key>CFBundleExecutable</key>
	<string>Updater</string>
	<key>CFBundleIdentifier</key>
	<string>com.darkedenclassic.launcher</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundleName</key>
	<string>DarkEden Classic</string>
	<key>CFBundlePackageType</key>
	<string>APPL</string>
	<key>CFBundleShortVersionString</key>
	<string>1.0.0</string>
	<key>CFBundleVersion</key>
	<string>1</string>
	<key>LSMinimumSystemVersion</key>
	<string>12.0</string>
	<key>NSHighResolutionCapable</key>
	<true/>
</dict>
</plist>
PLIST

add_rpath_if_missing "$MACOS_DIR/DarkEden"
add_rpath_if_missing "$MACOS_DIR/Updater"
copy_dylib_tree "$MACOS_DIR/DarkEden"
copy_dylib_tree "$MACOS_DIR/Updater"

if [[ ! -d "$MACOS_DIR/Data" ]]; then
	cat > "$PACKAGE_DIR/README-FIRST.txt" <<'README'
DarkEden Classic.app foi gerado sem a pasta Data.

Para jogar, baixe Data-mac.zip da release data-mac-v1, extraia e coloque a
pasta Data dentro de:

DarkEden Classic.app/Contents/MacOS/Data

Depois abra DarkEden Classic.app normalmente.
README
fi

if command -v codesign >/dev/null 2>&1; then
	echo "Applying ad-hoc code signature..."
	codesign --force --deep --sign - "$APP_DIR" || true
fi

(
	cd "$PACKAGE_DIR"
	ditto -c -k --sequesterRsrc --keepParent "$APP_NAME" "$ZIP_NAME"
)

echo "Created: $PACKAGE_DIR/$ZIP_NAME"
