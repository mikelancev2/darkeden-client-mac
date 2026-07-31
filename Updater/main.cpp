// Dark Eden Updater - cross-platform launcher + patch checker.
//
// Mirrors UpdaterPronto/CodigoFonte/GGameCheck.cpp's PatchList.dat-based
// update flow (download manifest, compare size+MD5, fetch changed files)
// but implemented with libcurl instead of MFC/WinINet, so it also builds
// for macOS. Windows keeps using the real MFC Updater (UpdaterPronto/)
// for now - this one is for macOS.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <curl/curl.h>

#include "md5.h"

#ifdef _WIN32
#include <process.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

// Base URL for the patch server. The domain has a real cert and works
// cleanly over HTTPS; if DNS/the domain ever misbehaves, swap to the IP
// directly - same vhost serves both (matches the IP the game itself
// connects to in Data/Info/GameClient.inf).
static const char* PATCH_BASE_URL = "https://darkedenclassic.com/patch2";
// static const char* PATCH_BASE_URL_FALLBACK = "https://89.117.75.68/patch2";

struct PatchEntry {
	std::string version;
	std::string remotePath; // as listed in PatchList.dat, e.g. "Test_mainfold/DarkEden"
	std::string localPath;  // remotePath with the "Test_mainfold/" staging prefix stripped
	long size;
	std::string md5;
};

static size_t WriteToString(void* contents, size_t size, size_t nmemb, void* userp)
{
	std::string* out = static_cast<std::string*>(userp);
	out->append(static_cast<char*>(contents), size * nmemb);
	return size * nmemb;
}

static size_t WriteToFile(void* contents, size_t size, size_t nmemb, void* userp)
{
	FILE* f = static_cast<FILE*>(userp);
	return fwrite(contents, size, nmemb, f);
}

static bool HttpGetToString(const std::string& url, std::string& out)
{
	CURL* curl = curl_easy_init();
	if (!curl) return false;
	out.clear();
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToString);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "DarkEdenMacUpdater/1.0");
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
	CURLcode res = curl_easy_perform(curl);
	long httpCode = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
	curl_easy_cleanup(curl);
	return res == CURLE_OK && httpCode >= 200 && httpCode < 300;
}

static bool HttpDownloadToFile(const std::string& url, const std::string& destPath)
{
	std::string tmpPath = destPath + ".part";
	FILE* f = fopen(tmpPath.c_str(), "wb");
	if (!f) return false;

	CURL* curl = curl_easy_init();
	if (!curl) { fclose(f); remove(tmpPath.c_str()); return false; }

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToFile);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "DarkEdenMacUpdater/1.0");
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);
	CURLcode res = curl_easy_perform(curl);
	long httpCode = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
	curl_easy_cleanup(curl);
	fclose(f);

	bool ok = (res == CURLE_OK) && (httpCode >= 200 && httpCode < 300);
	if (!ok) {
		remove(tmpPath.c_str());
		return false;
	}
	// Swap in atomically-ish - avoids leaving a half-written file at
	// destPath if the process gets killed mid-download.
	remove(destPath.c_str());
	return rename(tmpPath.c_str(), destPath.c_str()) == 0;
}

// Creates every missing directory component of a file path (like
// `mkdir -p $(dirname path)`).
static void MakeDirsForFile(const std::string& filePath)
{
	size_t pos = 0;
	while ((pos = filePath.find('/', pos + 1)) != std::string::npos) {
		std::string dir = filePath.substr(0, pos);
#ifdef _WIN32
		_mkdir(dir.c_str());
#else
		mkdir(dir.c_str(), 0755);
#endif
	}
}

static std::vector<PatchEntry> ParsePatchList(const std::string& text)
{
	std::vector<PatchEntry> entries;
	size_t pos = 0;
	while (pos < text.size()) {
		size_t eol = text.find('\n', pos);
		if (eol == std::string::npos) eol = text.size();
		std::string line = text.substr(pos, eol - pos);
		pos = eol + 1;
		if (!line.empty() && line.back() == '\r') line.pop_back();
		if (line.empty() || line[0] == '[') continue;

		// version,path,size,md5
		size_t c1 = line.find(',');
		size_t c2 = (c1 == std::string::npos) ? std::string::npos : line.find(',', c1 + 1);
		size_t c3 = (c2 == std::string::npos) ? std::string::npos : line.find(',', c2 + 1);
		if (c1 == std::string::npos || c2 == std::string::npos || c3 == std::string::npos) continue;

		PatchEntry e;
		e.version = line.substr(0, c1);
		e.remotePath = line.substr(c1 + 1, c2 - c1 - 1);
		e.size = atol(line.substr(c2 + 1, c3 - c2 - 1).c_str());
		e.md5 = line.substr(c3 + 1);

		// Manifest paths are staged under a "Test_mainfold/" prefix on the
		// server (an artifact of how the patch is packaged there) that
		// doesn't exist in the actual install layout - DarkEden sits
		// directly at the client root, not in a Test_mainfold subfolder.
		std::string normalized = e.remotePath;
		for (size_t i = 0; i < normalized.size(); i++) {
			if (normalized[i] == '\\') normalized[i] = '/';
		}
		const std::string prefix = "Test_mainfold/";
		if (normalized.compare(0, prefix.size(), prefix) == 0) {
			e.localPath = normalized.substr(prefix.size());
		} else {
			e.localPath = normalized;
		}

		entries.push_back(e);
	}
	return entries;
}

static bool FileNeedsUpdate(const PatchEntry& e)
{
	struct stat st;
	if (stat(e.localPath.c_str(), &st) != 0) {
		return true; // missing
	}
	if ((long)st.st_size != e.size) {
		return true; // size mismatch
	}

	char digest[33];
	if (!Md5File(e.localPath.c_str(), digest)) {
		return true; // couldn't hash it, be safe and redownload
	}

	std::string local(digest);
	std::string remote = e.md5;
	for (size_t i = 0; i < local.size(); i++) local[i] = (char)tolower((unsigned char)local[i]);
	for (size_t i = 0; i < remote.size(); i++) remote[i] = (char)tolower((unsigned char)remote[i]);
	return local != remote;
}

// Returns false only on a real failure (e.g. a file that needed updating
// couldn't be downloaded). An unreachable update server is not fatal -
// falls through to launching with whatever's on disk.
static bool RunUpdate()
{
	printf("Checking for updates at %s ...\n", PATCH_BASE_URL);

	std::string manifestText;
	std::string manifestUrl = std::string(PATCH_BASE_URL) + "/PatchList.dat";
	if (!HttpGetToString(manifestUrl, manifestText)) {
		fprintf(stderr, "WARNING: could not reach update server (%s) - launching with local files as-is.\n", manifestUrl.c_str());
		return true;
	}

	std::vector<PatchEntry> entries = ParsePatchList(manifestText);
	printf("Manifest has %d entries.\n", (int)entries.size());

	int updated = 0, failed = 0;
	for (size_t i = 0; i < entries.size(); i++) {
		const PatchEntry& e = entries[i];
		if (!FileNeedsUpdate(e)) continue;

		printf("Updating %s ...\n", e.localPath.c_str());
		MakeDirsForFile(e.localPath);
		std::string fileUrl = std::string(PATCH_BASE_URL) + "/" + e.remotePath;
		if (HttpDownloadToFile(fileUrl, e.localPath)) {
			updated++;
		} else {
			fprintf(stderr, "ERROR: failed to download %s\n", fileUrl.c_str());
			failed++;
		}
	}

	printf("Update check complete: %d updated, %d failed, %d already current.\n",
		updated, failed, (int)entries.size() - updated - failed);
	return failed == 0;
}

int main(int argc, char** argv)
{
	const char* mode = "0000000003"; // matches window.bat: windowed, 1024x768
	if (argc > 1 && argv[1] != NULL) {
		mode = argv[1];
	}

	curl_global_init(CURL_GLOBAL_DEFAULT);
	printf("Dark Eden Updater starting (mode=%s)...\n", mode);

	RunUpdate();

	curl_global_cleanup();

#ifdef _WIN32
	const char* clientPath = "darkeden.exe";
	_execl(clientPath, clientPath, mode, (char*)NULL);
#else
	const char* clientPath = "./DarkEden";
	execl(clientPath, "DarkEden", mode, (char*)NULL);
#endif

	// Only reached if exec failed to replace this process.
	fprintf(stderr, "ERROR: failed to launch client at %s\n", clientPath);
	return 1;
}
