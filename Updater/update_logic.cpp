#include "update_logic.h"
#include "md5.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <sys/stat.h>
#include <curl/curl.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

namespace {

size_t WriteToString(void* contents, size_t size, size_t nmemb, void* userp)
{
	std::string* out = static_cast<std::string*>(userp);
	out->append(static_cast<char*>(contents), size * nmemb);
	return size * nmemb;
}

size_t WriteToFile(void* contents, size_t size, size_t nmemb, void* userp)
{
	FILE* f = static_cast<FILE*>(userp);
	return fwrite(contents, size, nmemb, f);
}

bool HttpGetToString(const std::string& url, std::string& out)
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

bool HttpDownloadToFile(const std::string& url, const std::string& destPath)
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
	remove(destPath.c_str());
	return rename(tmpPath.c_str(), destPath.c_str()) == 0;
}

// Creates every missing directory component of a file path (like
// `mkdir -p $(dirname path)`).
void MakeDirsForFile(const std::string& filePath)
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

std::vector<PatchEntry> ParsePatchList(const std::string& text)
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
		e.localPath = (normalized.compare(0, prefix.size(), prefix) == 0)
			? normalized.substr(prefix.size())
			: normalized;

		entries.push_back(e);
	}
	return entries;
}

bool FileNeedsUpdate(const PatchEntry& e)
{
	struct stat st;
	if (stat(e.localPath.c_str(), &st) != 0) return true; // missing
	if ((long)st.st_size != e.size) return true;          // size mismatch

	char digest[33];
	if (!Md5File(e.localPath.c_str(), digest)) return true; // be safe, redownload

	std::string local(digest);
	std::string remote = e.md5;
	for (size_t i = 0; i < local.size(); i++) local[i] = (char)tolower((unsigned char)local[i]);
	for (size_t i = 0; i < remote.size(); i++) remote[i] = (char)tolower((unsigned char)remote[i]);
	return local != remote;
}

void Report(const ProgressCallback& cb, UpdateStage stage, const std::string& fileName,
	int fileIndex, int fileCount, const std::string& message)
{
	if (!cb) return;
	UpdateProgress p;
	p.stage = stage;
	p.fileName = fileName;
	p.fileIndex = fileIndex;
	p.fileCount = fileCount;
	p.message = message;
	cb(p);
}

} // namespace

bool RunUpdate(const std::string& baseUrl, const ProgressCallback& cb)
{
	Report(cb, UpdateStage::Connecting, "", 0, 0, "Connecting to update server...");

	std::string manifestText;
	std::string manifestUrl = baseUrl + "/PatchList.dat";
	if (!HttpGetToString(manifestUrl, manifestText)) {
		Report(cb, UpdateStage::ServerUnreachable, "", 0, 0,
			"Could not reach the update server - playing with local files.");
		return true;
	}

	std::vector<PatchEntry> entries = ParsePatchList(manifestText);
	int fileCount = (int)entries.size();
	Report(cb, UpdateStage::ManifestLoaded, "", 0, fileCount,
		"Checking " + std::to_string(fileCount) + " files...");

	int failed = 0;
	for (int i = 0; i < fileCount; i++) {
		const PatchEntry& e = entries[i];
		Report(cb, UpdateStage::CheckingFile, e.localPath, i, fileCount, "Checking " + e.localPath);

		if (!FileNeedsUpdate(e)) continue;

		Report(cb, UpdateStage::Downloading, e.localPath, i, fileCount, "Downloading " + e.localPath);
		MakeDirsForFile(e.localPath);
		std::string fileUrl = baseUrl + "/" + e.remotePath;
		if (HttpDownloadToFile(fileUrl, e.localPath)) {
			Report(cb, UpdateStage::FileDone, e.localPath, i, fileCount, "Updated " + e.localPath);
		} else {
			failed++;
			Report(cb, UpdateStage::Error, e.localPath, i, fileCount, "Failed to download " + e.localPath);
		}
	}

	if (failed == 0) {
		Report(cb, UpdateStage::Complete, "", fileCount, fileCount, "Up to date.");
	}
	return failed == 0;
}
