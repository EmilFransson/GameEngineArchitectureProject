#include "pch.h"
#include "Application.h"
const bool InitializeConsole();

int CALLBACK WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
#if defined (DEBUG) | defined (_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	if (!InitializeConsole())
	{
		std::cout << "Error: Failed to initialize debug console";
		return -1;
	}

	Application app;
	app.Run();
	app.CleanUp();
	return 0;
}

const bool InitializeConsole()
{
#pragma warning(disable : 4996)
	if (AllocConsole() == FALSE)
		return false;
	if (freopen("CONIN$", "r", stdin) == nullptr)
		return false;
	if (freopen("CONOUT$", "w", stdout) == nullptr)
		return false;
	if (freopen("CONOUT$", "w", stderr) == nullptr)
		return false;
	return true;
}