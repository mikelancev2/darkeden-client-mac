// Dark Eden Updater - plain console launcher (non-GUI fallback).
//
// On macOS this is superseded by main.mm (AppKit + WKWebView GUI, built
// automatically instead of this file when compiling on Apple - see
// CMakeLists.txt). This console version stays as a lightweight
// fallback/reference for other platforms and quick command-line testing.

#include <cstdio>
#include "update_logic.h"

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

static const char* PATCH_BASE_URL = "https://darkedenclassic.com/patch2";

static void PrintProgress(const UpdateProgress& p)
{
	printf("%s\n", p.message.c_str());
}

int main(int argc, char** argv)
{
	const char* mode = "0000000003"; // matches window.bat: windowed, 1024x768
	if (argc > 1 && argv[1] != NULL) {
		mode = argv[1];
	}

	printf("Dark Eden Updater starting (mode=%s)...\n", mode);
	RunUpdate(PATCH_BASE_URL, PrintProgress);

#ifdef _WIN32
	const char* clientPath = "darkeden.exe";
	_execl(clientPath, clientPath, mode, (char*)NULL);
#else
	const char* clientPath = "./DarkEden";
	execl(clientPath, "DarkEden", mode, (char*)NULL);
#endif

	fprintf(stderr, "ERROR: failed to launch client at %s\n", "client");
	return 1;
}
