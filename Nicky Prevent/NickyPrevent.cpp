#include <iostream>
#include <fstream>
#include <windows.h>

// default settings
char keyToBlock = 'W';
DWORD debounceTime = 50;

// syncing
DWORD lastKeyPress = 0;
bool wasKeyDown = false;

// deserilization of key to prevent double clicks from with set MS debounce time
void LoadSettings()
{
	std::ifstream infile("nicky.txt");
	if (infile.is_open())
	{
		infile >> keyToBlock;
		infile >> debounceTime;
		infile.close();
	}
	else
	{
		std::ofstream outfile("nicky.txt");
		if (outfile.is_open())
		{
			outfile << "W\n50\n";
			outfile.close();
		}
		else
		{
			std::cerr << "Could not create default settings file." << std::endl;
		}
	}
}

// key hook listener + debounce logic
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	KBDLLHOOKSTRUCT* kbdStruct = (KBDLLHOOKSTRUCT*)lParam;
	DWORD currentTime = GetTickCount64();
	DWORD deltaTime = currentTime - lastKeyPress;

	if (nCode == HC_ACTION)
	{
		if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
		{
			if (kbdStruct->vkCode == keyToBlock && !wasKeyDown)
			{
				std::cout << "Detected " << keyToBlock << " key with delta time: " << deltaTime << " ms" << std::endl;

				if (deltaTime < debounceTime)
				{
					std::cout << keyToBlock << " key blocked" << std::endl;
					wasKeyDown = true;
					return 1;
				}

				lastKeyPress = currentTime;
				wasKeyDown = true;
			}
		}
		else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
		{
			if (kbdStruct->vkCode == keyToBlock)
			{
				wasKeyDown = false;
			}
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// load settings and alloc the console and set the key hook
int main()
{
	LoadSettings();

	AllocConsole();
	FILE* stream;
	freopen_s(&stream, "CONOUT$", "w", stdout);

	HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);

	if (keyboardHook == NULL)
	{
		std::cerr << "Failed to set hook" << std::endl;
		return 1;
	}

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnhookWindowsHookEx(keyboardHook);
	return 0;
}
