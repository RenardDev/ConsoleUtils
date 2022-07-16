
// Default
#include <Windows.h>
#include <tchar.h>

// ConsoleUtils
#include "ConsoleUtils.h"

using namespace ConsoleUtils;

int _tmain() {
	SmartConsoleUtils SCU;

	if (SCU.Open()) {
		tclrprintf(COLOR::COLOR_CYAN, _T("Hello, ")); tclrprintf(COLOR_PAIR(COLOR::COLOR_GREEN, COLOR::COLOR_RED), _T("World!\n"));

		TCHAR szName[32];
		memset(szName, 0, sizeof(szName));
		tclrprintf(COLOR::COLOR_WHITE, _T("Enter name: "));
		tclrscanf(COLOR_PAIR(COLOR::COLOR_WHITE, COLOR::COLOR_GREEN), _T("%s"), szName, sizeof(szName) / sizeof(TCHAR) - 1);

		tclrprintf(COLOR::COLOR_YELLOW, _T("Your name: "));
		tclrprintf(COLOR::COLOR_BLUE, _T("%s\n"), szName);
	}

	return 0;
}