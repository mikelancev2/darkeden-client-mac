// Dark Eden Updater - patch-checking logic, shared between the plain CLI
// build (main.cpp) and the macOS GUI build (main.mm).
//
// Mirrors UpdaterPronto/CodigoFonte/GGameCheck.cpp's PatchList.dat-based
// update flow (download manifest, compare size+MD5, fetch changed files)
// using libcurl instead of MFC/WinINet.

#ifndef DARKEDEN_UPDATER_LOGIC_H
#define DARKEDEN_UPDATER_LOGIC_H

#include <string>
#include <vector>
#include <functional>

struct PatchEntry {
	std::string version;
	std::string remotePath; // as listed in PatchList.dat, e.g. "Test_mainfold/DarkEden"
	std::string localPath;  // remotePath with the "Test_mainfold/" staging prefix stripped
	long size;
	std::string md5;
};

enum class UpdateStage {
	Connecting,
	ManifestLoaded,
	CheckingFile,
	Downloading,
	FileDone,
	Complete,
	ServerUnreachable,
	Error,
};

struct UpdateProgress {
	UpdateStage stage;
	std::string fileName;  // relevant to CheckingFile/Downloading/FileDone
	int fileIndex;         // 0-based index into the manifest
	int fileCount;         // total manifest entries
	std::string message;   // human-readable status line
};

typedef std::function<void(const UpdateProgress&)> ProgressCallback;

// Downloads PatchList.dat from baseUrl, compares each entry against the
// local file (by size then MD5, only hashing when the size already
// matches), and downloads whatever's missing or different. Reports
// progress via `cb` as it goes (safe to pass an empty std::function).
//
// Returns true if the client is ready to launch: either everything is
// up to date, or the update server couldn't be reached at all (treated
// as non-fatal - falls through to playing with whatever's on disk rather
// than blocking). Returns false only when the manifest loaded but a
// needed file failed to download.
bool RunUpdate(const std::string& baseUrl, const ProgressCallback& cb);

#endif // DARKEDEN_UPDATER_LOGIC_H
