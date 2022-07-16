#include "ConsoleUtils.h"

// ----------------------------------------------------------------
// ConsoleUtils
// ----------------------------------------------------------------
namespace ConsoleUtils {
	// ----------------------------------------------------------------
	// SmartConsole
	// ----------------------------------------------------------------

	SmartConsole::SmartConsole(bool bAutoClose) {
		m_bAutoClose = bAutoClose;
		m_hWindow = GetConsoleWindow();
		m_pIn = nullptr;
		m_pOut = nullptr;
		m_hIn = GetStdHandle(STD_INPUT_HANDLE);
		m_hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		m_unOriginalMode = 0;
		m_nOriginalStyle = 0;
		m_nOriginalStyleEx = 0;
		if (m_hWindow) {
			setlocale(LC_ALL, "");

			if (m_hIn && (m_hIn != INVALID_HANDLE_VALUE)) {
				if (GetConsoleMode(m_hIn, &m_unOriginalMode)) {
					SetConsoleMode(m_hIn, m_unOriginalMode | ENABLE_INSERT_MODE);
				}
			}

			LONG nStyle = GetWindowLong(m_hWindow, GWL_STYLE);
			if (nStyle != 0) {
				m_nOriginalStyle = nStyle;
				SetWindowLong(m_hWindow, GWL_STYLE, nStyle & ~(WS_MAXIMIZEBOX | WS_MINIMIZEBOX));
			}

			LONG nStyleEx = GetWindowLong(m_hWindow, GWL_EXSTYLE);
			if (nStyleEx != 0) {
				m_nOriginalStyleEx = nStyleEx;
				SetWindowLong(m_hWindow, GWL_EXSTYLE, nStyleEx | WS_EX_LAYERED);
			}

			SetWindowPos(m_hWindow, nullptr, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOOWNERZORDER);
		}
	}

	SmartConsole::~SmartConsole() {
		if (m_bAutoClose) {
			Close();
		} else {
			if (m_nOriginalStyle != 0) {
				SetWindowLong(m_hWindow, GWL_STYLE, m_nOriginalStyle);
			}

			if (m_nOriginalStyleEx != 0) {
				SetWindowLong(m_hWindow, GWL_EXSTYLE, m_nOriginalStyleEx);
			}

			if (m_hIn && (m_hIn != INVALID_HANDLE_VALUE) && m_unOriginalMode) {
				SetConsoleMode(m_hIn, m_unOriginalMode);
			}
		}
	}

	bool SmartConsole::Open(bool bUpdateIO) {
		if (m_hWindow) {
			return true;
		}

		if (m_pIn) {
			if (_tfreopen_s(&m_pIn, _T("nul"), _T("r"), stdin)) {
				return false;
			}
			if (m_pIn) {
				fclose(m_pIn);
			}
			m_pIn = nullptr;
		}

		if (m_pOut) {
			if (_tfreopen_s(&m_pOut, _T("nul"), _T("w"), stdout)) {
				return false;
			}
			if (m_pOut) {
				fclose(m_pOut);
			}
			m_pOut = nullptr;
		}

		if (!AllocConsole()) {
			return false;
		}

		m_hWindow = GetConsoleWindow();
		if (!m_hWindow) {
			return false;
		}

		if (bUpdateIO) {
			if (_tfreopen_s(&m_pIn, _T("nul"), _T("r"), stdin)) {
				return false;
			}

			if (_tfreopen_s(&m_pOut, _T("nul"), _T("w"), stdout)) {
				return false;
			}
		}

		HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
		if (!hIn || (hIn == INVALID_HANDLE_VALUE)) {
			return false;
		}

		m_hIn = hIn;

		if (bUpdateIO) {
			int nInDescriptor = _open_osfhandle(reinterpret_cast<intptr_t>(hIn), _O_TEXT);
			if (nInDescriptor == -1) {
				return false;
			}

			FILE* pIn = _tfdopen(nInDescriptor, _T("r"));
			if (!pIn) {
				return false;
			}

			m_pIn = pIn;

			if (_dup2(_fileno(pIn), _fileno(stdin)) != 0) {
				return false;
			}

			if (setvbuf(stdin, nullptr, _IONBF, 0) != 0) {
				return false;
			}
		}

		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if (!hOut || (hOut == INVALID_HANDLE_VALUE)) {
			return false;
		}

		m_hOut = hOut;

		if (bUpdateIO) {
			int nOutDescriptor = _open_osfhandle(reinterpret_cast<intptr_t>(hOut), _O_TEXT);
			if (nOutDescriptor == -1) {
				return false;
			}

			FILE* pOut = _tfdopen(nOutDescriptor, _T("w"));
			if (!pOut) {
				return false;
			}

			m_pOut = pOut;

			if (_dup2(_fileno(pOut), _fileno(stdout)) != 0) {
				return false;
			}

			if (setvbuf(stdout, nullptr, _IONBF, 0) != 0) {
				return false;
			}
		}

		setlocale(LC_ALL, "");

		if (GetConsoleMode(hIn, &m_unOriginalMode)) {
			SetConsoleMode(hIn, m_unOriginalMode | ENABLE_INSERT_MODE);
		}

		LONG nStyle = GetWindowLong(m_hWindow, GWL_STYLE);
		if (nStyle != 0) {
			m_nOriginalStyle = nStyle;
			SetWindowLong(m_hWindow, GWL_STYLE, nStyle & ~(WS_MAXIMIZEBOX | WS_MINIMIZEBOX));
		}

		LONG nStyleEx = GetWindowLong(m_hWindow, GWL_EXSTYLE);
		if (nStyleEx != 0) {
			m_nOriginalStyleEx = nStyleEx;
			SetWindowLong(m_hWindow, GWL_EXSTYLE, nStyleEx | WS_EX_LAYERED);
		}

		SetWindowPos(m_hWindow, nullptr, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOOWNERZORDER);

		return true;
	}

	bool SmartConsole::Close() {
		if (!m_hWindow) {
			return false;
		}

		if (m_nOriginalStyle != 0) {
			SetWindowLong(m_hWindow, GWL_STYLE, m_nOriginalStyle);
		}

		if (m_nOriginalStyleEx != 0) {
			SetWindowLong(m_hWindow, GWL_EXSTYLE, m_nOriginalStyleEx);
		}

		if (m_hIn && (m_hIn != INVALID_HANDLE_VALUE) && m_unOriginalMode) {
			SetConsoleMode(m_hIn, m_unOriginalMode);
		}

		if (m_pIn) {
			if (_tfreopen_s(&m_pIn, _T("nul"), _T("r"), stdin)) {
				return false;
			}
			if (m_pIn) {
				fclose(m_pIn);
			}
			m_pIn = nullptr;
		}

		if (m_pOut) {
			if (_tfreopen_s(&m_pOut, _T("nul"), _T("w"), stdout)) {
				return false;
			}
			if (m_pOut) {
				fclose(m_pOut);
			}
			m_pOut = nullptr;
		}

		if (!FreeConsole()) {
			return false;
		}

		if (!DestroyWindow(m_hWindow)) {
			return false;
		}

		m_hWindow = nullptr;

		return true;
	}

	bool SmartConsole::Show() {
		if (!m_hWindow) {
			return false;
		}

		if (!ShowWindow(m_hWindow, SW_SHOW)) {
			return false;
		}

		return true;
	}

	bool SmartConsole::Hide() {
		if (!m_hWindow) {
			return false;
		}

		if (!ShowWindow(m_hWindow, SW_HIDE)) {
			return false;
		}

		return true;
	}

	bool SmartConsole::ReadA(char* const szBuffer, unsigned int unCount) {
		if (!m_hWindow) {
			return false;
		}

		if (!fgets(szBuffer, unCount, stdin)) {
			return false;
		}

		return true;
	}

	bool SmartConsole::ReadW(wchar_t* const szBuffer, unsigned int unCount) {
		if (!m_hWindow) {
			return false;
		}

		if (!fgetws(szBuffer, unCount, stdin)) {
			return false;
		}

		return true;
	}

#ifdef UNICODE
	bool SmartConsole::Read(wchar_t* const szBuffer, unsigned int unCount) {
		return ReadW(szBuffer, unCount);
	}
#else
	bool SmartConsole::Read(char* const szBuffer, unsigned int unCount) {
		return ReadA(szBuffer, unCount);
	}
#endif

	bool SmartConsole::WriteA(char const* const szBuffer) {
		if (!m_hWindow) {
			return false;
		}

		if (fputs(szBuffer, stdout) == EOF) {
			return false;
		}

		return true;
	}

	bool SmartConsole::WriteW(wchar_t const* const szBuffer) {
		if (!m_hWindow) {
			return false;
		}

		if (fputws(szBuffer, stdout) == EOF) {
			return false;
		}

		return true;
	}

#ifdef UNICODE
	bool SmartConsole::Write(wchar_t const* const szBuffer) {
		return WriteW(szBuffer);
	}
#else
	bool SmartConsole::Write(char const* const szBuffer) {
		return WriteA(szBuffer);
	}
#endif

	HWND SmartConsole::GetWindow() {
		return m_hWindow;
	}

	HANDLE SmartConsole::GetIn() {
		if (m_hIn == INVALID_HANDLE_VALUE) {
			return nullptr;
		}
		return m_hIn;
	}

	HANDLE SmartConsole::GetOut() {
		if (m_hOut == INVALID_HANDLE_VALUE) {
			return nullptr;
		}
		return m_hOut;
	}

	// ----------------------------------------------------------------
	// SmartConsoleUtils
	// ----------------------------------------------------------------

	SmartConsoleUtils::SmartConsoleUtils(bool bAutoClose, bool bAutoRestoreColors) : SmartConsole(bAutoClose) {
		HWND hWindow = GetWindow();
		HANDLE hOut = GetOut();

		m_bAutoRestoreColors = bAutoRestoreColors;
		
		if (bAutoRestoreColors && hWindow && hOut) {
			GetColor(&m_OriginalColorPair);
		}

		if (bAutoRestoreColors && hWindow && hOut) {
			GetCursorColor(&m_OriginalCursorColorPair);
		}

		for (unsigned char i = 0; i < 16; ++i) {
			m_OriginalColorTable[i] = 0x00000000;
		}

		if (bAutoRestoreColors && hWindow && hOut) {
			CONSOLE_SCREEN_BUFFER_INFOEX csbi;
			if (GetBufferInfo(&csbi)) {
				for (unsigned char i = 0; i < 16; ++i) {
					m_OriginalColorTable[i] = csbi.ColorTable[i];
				}
			}
		}
	}

	SmartConsoleUtils::~SmartConsoleUtils() {
		if (m_bAutoRestoreColors && GetWindow() && GetOut()) {
			CONSOLE_SCREEN_BUFFER_INFOEX csbi;
			if (GetBufferInfo(&csbi)) {
				for (unsigned char i = 0; i < 16; ++i) {
					csbi.ColorTable[i] = m_OriginalColorTable[i];
				}
				SetBufferInfo(csbi);
			}
			SetColor(m_OriginalColorPair);
			SetCursorColor(m_OriginalCursorColorPair);
		}
	}

	bool SmartConsoleUtils::Open(bool bUpdateIO) {
		if (!SmartConsole::Open(bUpdateIO)) {
			return false;
		}

		HWND hWindow = GetWindow();
		HANDLE hOut = GetOut();

		m_OriginalColorPair = COLOR_PAIR();
		if (m_bAutoRestoreColors && hWindow && hOut) {
			GetColor(&m_OriginalColorPair);
		}

		m_OriginalCursorColorPair = COLOR_PAIR();
		if (m_bAutoRestoreColors && hWindow && hOut) {
			GetCursorColor(&m_OriginalCursorColorPair);
		}

		m_PreviousColorPair = COLOR_PAIR();
		m_PreviousCursorColorPair = COLOR_PAIR();

		for (unsigned char i = 0; i < 16; ++i) {
			m_OriginalColorTable[i] = 0x00000000;
		}

		if (m_bAutoRestoreColors && hWindow && hOut) {
			CONSOLE_SCREEN_BUFFER_INFOEX csbi;
			if (GetBufferInfo(&csbi)) {
				for (unsigned char i = 0; i < 16; ++i) {
					m_OriginalColorTable[i] = csbi.ColorTable[i];
				}
			}
		}

		return true;
	}

	bool SmartConsoleUtils::Close() {
		if (m_bAutoRestoreColors && GetWindow() && GetOut()) {
			CONSOLE_SCREEN_BUFFER_INFOEX csbi;
			if (GetBufferInfo(&csbi)) {
				for (unsigned char i = 0; i < 16; ++i) {
					csbi.ColorTable[i] = m_OriginalColorTable[i];
				}
				SetBufferInfo(csbi);
			}
			SetColor(m_OriginalColorPair);
			SetCursorColor(m_OriginalCursorColorPair);
		}

		return SmartConsole::Close();
	}

	bool SmartConsoleUtils::GetBufferInfo(PCONSOLE_SCREEN_BUFFER_INFOEX pBufferInfo) {
		if (!pBufferInfo) {
			return false;
		}

		if (!GetWindow()) {
			return false;
		}

		HANDLE hOut = GetOut();
		if (!hOut) {
			return false;
		}

		memset(pBufferInfo, 0, sizeof(CONSOLE_SCREEN_BUFFER_INFOEX));
		pBufferInfo->cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);

		if (!GetConsoleScreenBufferInfoEx(hOut, pBufferInfo)) {
			return false;
		}

		return true;
	}

	bool SmartConsoleUtils::SetBufferInfo(CONSOLE_SCREEN_BUFFER_INFOEX BufferInfo) {
		if (!GetWindow()) {
			return false;
		}

		HANDLE hOut = GetOut();
		if (!hOut) {
			return false;
		}

		++BufferInfo.srWindow.Bottom;
		++BufferInfo.srWindow.Right;

		if (!SetConsoleScreenBufferInfoEx(hOut, &BufferInfo)) {
			return false;
		}

		--BufferInfo.srWindow.Bottom;
		--BufferInfo.srWindow.Right;

		return true;
	}

	bool SmartConsoleUtils::GetAttributes(PWORD pAttributes) {
		if (!pAttributes) {
			return false;
		}

		CONSOLE_SCREEN_BUFFER_INFOEX csbi;
		if (!GetBufferInfo(&csbi)) {
			return false;
		}

		*pAttributes = csbi.wAttributes;

		return true;
	}

	bool SmartConsoleUtils::SetAttributes(WORD unAttributes) {
		if (!GetWindow()) {
			return false;
		}

		HANDLE hOut = GetOut();
		if (!hOut) {
			return false;
		}

		if (!SetConsoleTextAttribute(hOut, unAttributes)) {
			return false;
		}

		return true;
	}

	bool SmartConsoleUtils::ChangeColorPalette(COLOR Color, unsigned int unRGB) {
		if (static_cast<unsigned char>(Color) > 15) {
			return false;
		}

		CONSOLE_SCREEN_BUFFER_INFOEX csbi;
		if (!GetBufferInfo(&csbi)) {
			return false;
		}

		unsigned char unR = (unRGB >> 16) & 0xFF;
		unsigned char unG = (unRGB >> 8) & 0xFF;
		unsigned char unB = unRGB & 0xFF;

		csbi.ColorTable[static_cast<unsigned char>(Color)] = RGB(unR, unG, unB);

		if (!SetBufferInfo(csbi)) {
			return false;
		}

		if (!Flush()) {
			return false;
		}

		return true;
	}

	bool SmartConsoleUtils::ChangeColorPalette(COLOR Color, unsigned char unR, unsigned char unG, unsigned char unB) {
		if (static_cast<unsigned char>(Color) > 15) {
			return false;
		}

		CONSOLE_SCREEN_BUFFER_INFOEX csbi;
		if (!GetBufferInfo(&csbi)) {
			return false;
		}

		csbi.ColorTable[static_cast<unsigned char>(Color)] = RGB(unR, unG, unB);

		if (!SetBufferInfo(csbi)) {
			return false;
		}

		return true;
	}

	bool SmartConsoleUtils::Flush(bool bClear, bool bUpdateOriginalColorPair, bool bResetPreviousColorPair) {
		if (!GetWindow()) {
			return false;
		}

		HANDLE hOut = GetOut();
		if (!hOut) {
			return false;
		}

		CONSOLE_SCREEN_BUFFER_INFOEX csbi;
		if (!GetBufferInfo(&csbi)) {
			return false;
		}

		if (bClear) {
			SMALL_RECT Scroll;
			Scroll.Left = 0;
			Scroll.Top = 0;
			Scroll.Right = csbi.dwSize.X;
			Scroll.Bottom = csbi.dwSize.Y;

			COORD ScrollTarget;
			memset(&ScrollTarget, 0, sizeof(ScrollTarget));

			ScrollTarget.X = 0;
			ScrollTarget.Y = -csbi.dwSize.Y;

			CHAR_INFO Fill;
			memset(&Fill, 0, sizeof(Fill));
#ifdef UNICODE
			Fill.Char.UnicodeChar = L' ';
#else
			Fill.Char.AsciiChar = ' ';
#endif
			Fill.Attributes = csbi.wAttributes;

			if (!ScrollConsoleScreenBuffer(hOut, &Scroll, nullptr, ScrollTarget, &Fill)) {
				return false;
			}

			csbi.dwCursorPosition.X = 0;
			csbi.dwCursorPosition.Y = 0;

			if (!SetConsoleCursorPosition(hOut, csbi.dwCursorPosition)) {
				return false;
			}
		}

		COORD Coord;
		Coord.X = 0;
		Coord.Y = 0;
		DWORD unWrittenAttributes = 0;
		if (!FillConsoleOutputAttribute(hOut, csbi.wAttributes, csbi.dwSize.Y * csbi.dwSize.X, Coord, &unWrittenAttributes)) {
			return false;
		}

		if (bUpdateOriginalColorPair) {
			COLOR_PAIR CurrentColorPair;
			if (!GetColor(&CurrentColorPair)) {
				m_OriginalColorPair = CurrentColorPair;
			}
		}

		if (bResetPreviousColorPair) {
			m_PreviousColorPair = COLOR_PAIR();
		}

		return true;
	}

	bool SmartConsoleUtils::GetColor(PCOLOR_PAIR pColorPair) {
		if (!pColorPair) {
			return false;
		}

		WORD unAttributes = 0;
		if (!GetAttributes(&unAttributes)) {
			return false;
		}

		*pColorPair = COLOR_PAIR(static_cast<COLOR>((unAttributes & 0xF0) >> 4), static_cast<COLOR>(unAttributes & 0x0F));

		return true;
	}

	bool SmartConsoleUtils::SetColor(COLOR_PAIR ColorPair) {
		if (!GetWindow()) {
			return false;
		}

		HANDLE hOut = GetOut();
		if (!hOut) {
			return false;
		}

		WORD unAttributes = 0;
		if (!GetAttributes(&unAttributes)) {
			return false;
		}

		COLOR_PAIR CurrentColorPair(static_cast<COLOR>((unAttributes & 0xF0) >> 4), static_cast<COLOR>(unAttributes & 0x0F));

		if (ColorPair.ColorBackground != COLOR::COLOR_UNKNOWN) {
			unAttributes = static_cast<unsigned char>((static_cast<unsigned char>(ColorPair.ColorBackground) & 0x0F) << 4);
		} else {
			unAttributes = static_cast<unsigned char>(CurrentColorPair.ColorBackground);
		}

		if (ColorPair.ColorForeground != COLOR::COLOR_UNKNOWN) {
			unAttributes |= static_cast<unsigned char>((static_cast<unsigned char>(ColorPair.ColorForeground) & 0x0F));
		} else {
			unAttributes |= static_cast<unsigned char>(CurrentColorPair.ColorForeground);
		}

		m_PreviousColorPair = CurrentColorPair;

		CONSOLE_SCREEN_BUFFER_INFOEX csbi;
		if (!GetBufferInfo(&csbi)) {
			return false;
		}

		COORD Coord;
		Coord.X = 0;
		Coord.Y = 0;
		DWORD unWrittenAttributes = 0;
		if (!FillConsoleOutputAttribute(hOut, unAttributes, csbi.dwSize.Y * csbi.dwSize.X, Coord, &unWrittenAttributes)) {
			return false;
		}

		return true;
	}

	bool SmartConsoleUtils::RestoreColor(bool bRestorePrevious) {
		if (bRestorePrevious) {
			return SetColor(m_PreviousColorPair);
		}
		return SetColor(m_OriginalColorPair);
	}

	bool SmartConsoleUtils::GetCursorInfo(PCONSOLE_CURSOR_INFO pCursorInfo) {
		if (!pCursorInfo) {
			return false;
		}

		if (!GetWindow()) {
			return false;
		}

		HANDLE hOut = GetOut();
		if (!hOut) {
			return false;
		}

		memset(pCursorInfo, 0, sizeof(CONSOLE_CURSOR_INFO));

		if (!GetConsoleCursorInfo(hOut, pCursorInfo)) {
			return false;
		}

		return true;
	}

	bool SmartConsoleUtils::SetCursorInfo(CONSOLE_CURSOR_INFO CursorInfo) {
		if (!GetWindow()) {
			return false;
		}

		HANDLE hOut = GetOut();
		if (!hOut) {
			return false;
		}

		if (!SetConsoleCursorInfo(hOut, &CursorInfo)) {
			return false;
		}

		return true;
	}

	bool SmartConsoleUtils::GetCursorPosition(PCOORD pCursorPosition) {
		if (!pCursorPosition) {
			return false;
		}

		CONSOLE_SCREEN_BUFFER_INFOEX csbi;
		if (!GetBufferInfo(&csbi)) {
			return false;
		}

		pCursorPosition->X = csbi.dwCursorPosition.X;
		pCursorPosition->Y = csbi.dwCursorPosition.Y;

		return true;
	}

	bool SmartConsoleUtils::SetCursorPosition(COORD CursorPosition) {
		if (!GetWindow()) {
			return false;
		}

		HANDLE hOut = GetOut();
		if (!hOut) {
			return false;
		}

		if (!SetConsoleCursorPosition(hOut, CursorPosition)) {
			return false;
		}

		return true;
	}

	bool SmartConsoleUtils::ShowCursor() {
		CONSOLE_CURSOR_INFO cci;
		if (!GetCursorInfo(&cci)) {
			return false;
		}

		cci.bVisible = TRUE;

		if (!SetCursorInfo(cci)) {
			return false;
		}

		return true;
	}

	bool SmartConsoleUtils::HideCursor() {
		CONSOLE_CURSOR_INFO cci;
		if (!GetCursorInfo(&cci)) {
			return false;
		}

		cci.bVisible = FALSE;

		if (!SetCursorInfo(cci)) {
			return false;
		}

		return true;
	}

	bool SmartConsoleUtils::ToggleCursor() {
		CONSOLE_CURSOR_INFO cci;
		if (!GetCursorInfo(&cci)) {
			return false;
		}

		cci.bVisible = !static_cast<bool>(cci.bVisible & 1);

		if (!SetCursorInfo(cci)) {
			return false;
		}

		return true;
	}

	bool SmartConsoleUtils::GetCursorColor(PCOLOR_PAIR pColorPair) {
		if (!pColorPair) {
			return false;
		}

		WORD unAttributes = 0;
		if (!GetAttributes(&unAttributes)) {
			return false;
		}

		*pColorPair = COLOR_PAIR(static_cast<COLOR>((unAttributes & 0xF0) >> 4), static_cast<COLOR>(unAttributes & 0x0F));

		return true;
	}

	bool SmartConsoleUtils::SetCursorColor(COLOR_PAIR ColorPair) {
		WORD unAttributes = 0;
		if (!GetAttributes(&unAttributes)) {
			return false;
		}

		COLOR_PAIR CurrentColorPair(static_cast<COLOR>((unAttributes & 0xF0) >> 4), static_cast<COLOR>(unAttributes & 0x0F));

		if (ColorPair.ColorBackground != COLOR::COLOR_UNKNOWN) {
			unAttributes = static_cast<unsigned char>((static_cast<unsigned char>(ColorPair.ColorBackground) & 0x0F) << 4);
		} else {
			unAttributes = static_cast<unsigned char>(CurrentColorPair.ColorBackground);
		}

		if (ColorPair.ColorForeground != COLOR::COLOR_UNKNOWN) {
			unAttributes |= static_cast<unsigned char>((static_cast<unsigned char>(ColorPair.ColorForeground) & 0x0F));
		} else {
			unAttributes |= static_cast<unsigned char>(CurrentColorPair.ColorForeground);
		}

		m_PreviousCursorColorPair = CurrentColorPair;

		if (!SetAttributes(unAttributes)) {
			return false;
		}

		return true;
	}

	bool SmartConsoleUtils::RestoreCursorColor(bool bRestorePrevious) {
		if (bRestorePrevious) {
			return SetCursorColor(m_PreviousCursorColorPair);
		}
		return SetCursorColor(m_OriginalCursorColorPair);
	}

	bool SmartConsoleUtils::Erase(COORD CursorPosition, unsigned int unLength) {
		if (!GetWindow()) {
			return false;
		}

		HANDLE hOut = GetOut();
		if (!hOut) {
			return false;
		}

		DWORD unWrittenAttributes = 0;
		if (!FillConsoleOutputCharacter(hOut, _T(' '), unLength, CursorPosition, &unWrittenAttributes)) {
			return false;
		}

		WORD unAttributes = 0;
		if (!GetAttributes(&unAttributes)) {
			return false;
		}

		unWrittenAttributes = 0;
		if (!FillConsoleOutputAttribute(hOut, unAttributes, unLength, CursorPosition, &unWrittenAttributes)) {
			return false;
		}

		return true;
	}

	// ----------------------------------------------------------------
	// print/scan with format and color support
	// ----------------------------------------------------------------

	int clrvprintf(COLOR_PAIR ColorPair, char const* const _Format, va_list vargs) {
		char* szBuffer = new char[8192];
		if (!szBuffer) {
			return -1;
		}

		memset(szBuffer, 0, sizeof(szBuffer));

		SmartConsoleUtils SCU;
		
		if (!SCU.SetCursorColor(ColorPair)) {
			delete[] szBuffer;
			return -1;
		}

		int nLength = vsprintf_s(szBuffer, 8192, _Format, vargs);
		if (nLength == -1) {
			delete[] szBuffer;
			return -1;
		}

		szBuffer[8191] = 0;
		if (!SCU.WriteA(szBuffer)) {
			SCU.RestoreCursorColor(true);
			delete[] szBuffer;
			return -1;
		}

		if (!SCU.RestoreCursorColor(true)) {
			delete[] szBuffer;
			return -1;
		}

		delete[] szBuffer;
		return nLength;
	}

	int clrvprintf(COLOR unForegroundColor, char const* const _Format, va_list vargs) {
		return clrvprintf(COLOR_PAIR(unForegroundColor), _Format, vargs);
	}

	int clrprintf(COLOR_PAIR ColorPair, char const* const _Format, ...) {
		va_list vargs;
		va_start(vargs, _Format);
		int nLength = clrvprintf(ColorPair, _Format, vargs);
		va_end(vargs);
		return nLength;
	}

	int clrprintf(COLOR unForegroundColor, char const* const _Format, ...) {
		va_list vargs;
		va_start(vargs, _Format);
		int nLength = clrvprintf(unForegroundColor, _Format, vargs);
		va_end(vargs);
		return nLength;
	}

	int clrvwprintf(COLOR_PAIR ColorPair, wchar_t const* const _Format, va_list vargs) {
		wchar_t* szBuffer = new wchar_t[8192];
		if (!szBuffer) {
			return -1;
		}

		memset(szBuffer, 0, sizeof(szBuffer));

		SmartConsoleUtils SCU;

		if (!SCU.SetCursorColor(ColorPair)) {
			delete[] szBuffer;
			return -1;
		}

		int nLength = vswprintf_s(szBuffer, 8192, _Format, vargs);
		if (nLength == -1) {
			delete[] szBuffer;
			return -1;
		}

		szBuffer[8191] = 0;
		if (!SCU.WriteW(szBuffer)) {
			SCU.RestoreCursorColor(true);
			delete[] szBuffer;
			return -1;
		}

		if (!SCU.RestoreCursorColor(true)) {
			delete[] szBuffer;
			return -1;
		}

		delete[] szBuffer;
		return nLength;
	}

	int clrvwprintf(COLOR unForegroundColor, wchar_t const* const _Format, va_list vargs) {
		return clrvwprintf(COLOR_PAIR(unForegroundColor), _Format, vargs);
	}

	int clrwprintf(COLOR_PAIR ColorPair, wchar_t const* const _Format, ...) {
		va_list vargs;
		va_start(vargs, _Format);
		int nLength = clrvwprintf(ColorPair, _Format, vargs);
		va_end(vargs);
		return nLength;
	}

	int clrwprintf(COLOR unForegroundColor, wchar_t const* const _Format, ...) {
		va_list vargs;
		va_start(vargs, _Format);
		int nLength = clrvwprintf(unForegroundColor, _Format, vargs);
		va_end(vargs);
		return nLength;
	}

#ifdef UNICODE
	int tclrvprintf(COLOR_PAIR ColorPair, wchar_t const* const _Format, va_list vargs) {
		return clrvwprintf(ColorPair, _Format, vargs);
	}

	int tclrvprintf(COLOR unForegroundColor, wchar_t const* const _Format, va_list vargs) {
		return clrvwprintf(unForegroundColor, _Format, vargs);
	}

	int tclrprintf(COLOR_PAIR ColorPair, wchar_t const* const _Format, ...) {
		va_list vargs;
		va_start(vargs, _Format);
		int nLength = tclrvprintf(ColorPair, _Format, vargs);
		va_end(vargs);
		return nLength;
	}

	int tclrprintf(COLOR unForegroundColor, wchar_t const* const _Format, ...) {
		va_list vargs;
		va_start(vargs, _Format);
		int nLength = tclrvprintf(unForegroundColor, _Format, vargs);
		va_end(vargs);
		return nLength;
	}
#else
	int tclrvprintf(COLOR_PAIR ColorPair, char const* const _Format, va_list vargs) {
		return clrvprintf(ColorPair, _Format, vargs);
	}

	int tclrvprintf(COLOR unForegroundColor, char const* const _Format, va_list vargs) {
		return clrvprintf(unForegroundColor, _Format, vargs);
	}

	int tclrprintf(COLOR_PAIR ColorPair, char const* const _Format, ...) {
		va_list vargs;
		va_start(vargs, _Format);
		int nLength = tclrvprintf(ColorPair, _Format, vargs);
		va_end(vargs);
		return nLength;
	}

	int tclrprintf(COLOR unForegroundColor, char const* const _Format, ...) {
		va_list vargs;
		va_start(vargs, _Format);
		int nLength = tclrvprintf(unForegroundColor, _Format, vargs);
		va_end(vargs);
		return nLength;
	}
#endif

	int clrvscanf(COLOR_PAIR ColorPair, char const* const _Format, va_list vargs) {
		char* szBuffer = new char[8192];
		if (!szBuffer) {
			return -1;
		}

		memset(szBuffer, 0, sizeof(szBuffer));

		SmartConsoleUtils SCU;

		if (!SCU.SetCursorColor(ColorPair)) {
			delete[] szBuffer;
			return -1;
		}

		if (!SCU.ReadA(szBuffer, 8191)) {
			SCU.RestoreCursorColor(true);
			delete[] szBuffer;
			return -1;
		}

		int nLength = vsscanf_s(szBuffer, _Format, vargs);
		if (nLength == -1) {
			delete[] szBuffer;
			return -1;
		}

		if (!SCU.RestoreCursorColor(true)) {
			delete[] szBuffer;
			return -1;
		}

		delete[] szBuffer;
		return nLength;
	}

	int clrvscanf(COLOR unForegroundColor, char const* const _Format, va_list vargs) {
		return clrvscanf(COLOR_PAIR(unForegroundColor), _Format, vargs);
	}

	int clrscanf(COLOR_PAIR ColorPair, char const* const _Format, ...) {
		va_list vargs;
		va_start(vargs, _Format);
		int nLength = clrvscanf(ColorPair, _Format, vargs);
		va_end(vargs);
		return nLength;
	}

	int clrscanf(COLOR unForegroundColor, char const* const _Format, ...) {
		va_list vargs;
		va_start(vargs, _Format);
		int nLength = clrvscanf(COLOR_PAIR(unForegroundColor), _Format, vargs);
		va_end(vargs);
		return nLength;
	}

	int clrvwscanf(COLOR_PAIR ColorPair, wchar_t const* const _Format, va_list vargs) {
		wchar_t* szBuffer = new wchar_t[8192];
		if (!szBuffer) {
			return -1;
		}

		memset(szBuffer, 0, sizeof(szBuffer));

		SmartConsoleUtils SCU;

		if (!SCU.SetCursorColor(ColorPair)) {
			delete[] szBuffer;
			return -1;
		}

		if (!SCU.ReadW(szBuffer, 8191)) {
			SCU.RestoreCursorColor(true);
			delete[] szBuffer;
			return -1;
		}

		int nLength = vswscanf_s(szBuffer, _Format, vargs);
		if (nLength == -1) {
			delete[] szBuffer;
			return -1;
		}

		if (!SCU.RestoreCursorColor(true)) {
			delete[] szBuffer;
			return -1;
		}

		delete[] szBuffer;
		return nLength;
	}

	int clrvwscanf(COLOR unForegroundColor, wchar_t const* const _Format, va_list vargs) {
		return clrvwscanf(COLOR_PAIR(unForegroundColor), _Format, vargs);
	}

	int clrwscanf(COLOR_PAIR ColorPair, wchar_t const* const _Format, ...) {
		va_list vargs;
		va_start(vargs, _Format);
		int nLength = clrvwscanf(ColorPair, _Format, vargs);
		va_end(vargs);
		return nLength;
	}

	int clrwscanf(COLOR unForegroundColor, wchar_t const* const _Format, ...) {
		va_list vargs;
		va_start(vargs, _Format);
		int nLength = clrvwscanf(COLOR_PAIR(unForegroundColor), _Format, vargs);
		va_end(vargs);
		return nLength;
	}

#ifdef UNICODE
	int tclrvscanf(COLOR_PAIR ColorPair, wchar_t const* const _Format, va_list vargs) {
		return clrvwscanf(ColorPair, _Format, vargs);
	}

	int tclrvscanf(COLOR unForegroundColor, wchar_t const* const _Format, va_list vargs) {
		return clrvwscanf(unForegroundColor, _Format, vargs);
	}

	int tclrscanf(COLOR_PAIR ColorPair, wchar_t const* const _Format, ...) {
		va_list vargs;
		va_start(vargs, _Format);
		int nLength = tclrvscanf(ColorPair, _Format, vargs);
		va_end(vargs);
		return nLength;
	}

	int tclrscanf(COLOR unForegroundColor, wchar_t const* const _Format, ...) {
		va_list vargs;
		va_start(vargs, _Format);
		int nLength = tclrvscanf(unForegroundColor, _Format, vargs);
		va_end(vargs);
		return nLength;
	}
#else
	int tclrvscanf(COLOR_PAIR ColorPair, char const* const _Format, va_list vargs) {
		return clrvscanf(ColorPair, _Format, vargs);
	}

	int tclrvscanf(COLOR unForegroundColor, char const* const _Format, va_list vargs) {
		return clrvscanf(unForegroundColor, _Format, vargs);
	}

	int tclrscanf(COLOR_PAIR ColorPair, char const* const _Format, ...) {
		va_list vargs;
		va_start(vargs, _Format);
		int nLength = tclrvscanf(ColorPair, _Format, vargs);
		va_end(vargs);
		return nLength;
	}

	int tclrscanf(COLOR unForegroundColor, char const* const _Format, ...) {
		va_list vargs;
		va_start(vargs, _Format);
		int nLength = tclrvscanf(unForegroundColor, _Format, vargs);
		va_end(vargs);
		return nLength;
	}
#endif
}
