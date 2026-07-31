// Dark Eden Updater - minimal cross-platform launcher.
//
// This is a first pass, not the full Windows MFC Updater (UpdaterPronto/):
// it does not yet download/verify patch files against a remote server -
// there is no live update endpoint for macOS yet. What it does today:
//   1. Launch the game client with a valid mode string (the same
//      convention window.bat/fullscreen.bat use for the Windows build:
//      "00000000" + one digit selecting window/fullscreen + resolution).
//
// Once a remote patch endpoint exists for macOS, the manifest-check and
// file-download step (mirroring UpdaterPronto/CodigoFonte/GGameCheck.cpp's
// PatchList.dat comparison) belongs right before the launch below.

#include <cstdio>
#include <cstring>

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

int main(int argc, char** argv)
{
	// Default: windowed, 1024x768 (matches window.bat's "0000000003").
	const char* mode = "0000000003";
	if (argc > 1 && argv[1] != NULL) {
		mode = argv[1];
	}

	printf("Dark Eden Updater starting (mode=%s)...\n", mode);

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
