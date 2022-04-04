#if defined(_WIN32) || defined(_WIN64)
#define _CRT_SECURE_NO_DEPRECATE
#include <conio.h>
//https://docs.microsoft.com/en-us/windows/console/console-functions
#endif

#include "display.h"

void display_init()
{
#ifdef NCURSES
	initscr();
	halfdelay(10);	/* 1 second */
	noecho();
	keypad(stdscr, TRUE);
#else
	/*--- Set the minimum buffer size --------------------------------------*/
	HANDLE win = cmdwin();
	SMALL_RECT r;
	r.Left = 0;
	r.Right = max(120, GetSystemMetrics(SM_CXMIN));
	r.Top = 0;
	r.Bottom = max(30, GetSystemMetrics(SM_CYMIN));
	COORD xy;
	xy.X = r.Right - r.Left;
	xy.Y = r.Bottom - r.Top;
	SetConsoleScreenBufferSize(win, xy);

	/*--- Get the window border size ---------------------------------------*/
	HWND window = GetConsoleWindow();
	RECT rc, rw;
	POINT ptDiff;
	GetClientRect(window, &rc);
	GetWindowRect(window, &rw);
	int winX = (rw.right - rw.left) - rc.right;
	int winY = (rw.bottom - rw.top) - rc.bottom;

	/*--- Get the font size ------------------------------------------------*/
	CONSOLE_FONT_INFO info;
	GetCurrentConsoleFont(win, FALSE, &info);
	COORD fontSize = GetConsoleFontSize(win, info.nFont);
	int w = xy.X * fontSize.X;
	int h = xy.Y * fontSize.Y;

	MoveWindow(window, rw.left, rw.top, w + winX, h + winY, TRUE);
#endif
}

void display_quit()
{
#ifdef NCURSES
	endwin();
#endif
}

void display_clear()
{
#ifdef NCURSES
	clear();
#else
	int rows, cols;
	display_get_rows_cols(&rows, &cols);
	DWORD c;
	rows++;
	FillConsoleOutputCharacter(cmdwin(), L' ', rows * cols, (COORD) { .X = 0, .Y = 0 }, &c);
#endif
}

void display_move(int y, int x)
{
#ifdef NCURSES
	move(y, x);
#else
	COORD xy;
	xy.X = x;
	xy.Y = y;
	SetConsoleCursorPosition(cmdwin(), xy);
#endif
}

void display_refresh()
{
#ifdef NCURSES
	refresh();
#endif
}

int display_get_char()
{
#ifdef NCURSES
	return getch();
#else
	int res = 0;
	while (_kbhit())
	{
		mbstate_t state;
		memset(&state, 0, sizeof state);
		wchar_t wChar = (wchar_t)_getch();
		wchar_t* pwChar = &wChar;
		unsigned char utf8Char = 0;
		wcsrtombs(&utf8Char, &pwChar, 1, &state);
		if (utf8Char == '\0')
			break;
		res <<= 1;
		res |= utf8Char;
	}
	return res > 0 ? res : ERR;
#endif
}

void display_get_yx(int* outY, int* outX)
{
#ifdef NCURSES
	getyx(stdscr, *outY, *outX);
#else
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(cmdwin(), &info);
	*outY = info.dwCursorPosition.Y;
	*outX = info.dwCursorPosition.X;
#endif
}

void display_get_rows_cols(int* outRows, int* outCols)
{
#ifdef NCURSES
	getmaxyx(stdscr, *outRows, *outCols);
#else
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(cmdwin(), &info);
	*outCols = info.srWindow.Right - info.srWindow.Left;
	*outRows = info.srWindow.Bottom - info.srWindow.Top + 1;
#endif
}

void display_clear_to_line_end()
{
#ifdef NCURSES
	clrtoeol();
#else
	int x, y, rows, cols;
	display_get_yx(&y, &x);
	display_get_rows_cols(&rows, &cols);
	int len = (cols - x) + 1;
	if (len == 0)
		return;
	DWORD c;
	FillConsoleOutputCharacter(cmdwin(), L' ', len, (COORD) { .X = x, .Y = y }, &c);
	display_move(y, x);
#endif
}

void display_add_char(uint16_t utf8Letter)
{
#ifdef NCURSES
	addch(utf8Letter);
#else
	wchar_t wLetter[1];
	mbstowcs(wLetter, &utf8Letter, 1);
	WriteConsole(cmdwin(), &wLetter, 1, NULL, NULL);
#endif
}

void display_insert_line()
{
#ifdef NCURSES
	insertln();
#else
	int x, y;
	int rows, cols;
	display_get_yx(&y, &x);
	display_get_rows_cols(&rows, &cols);
	x = 0;
	SMALL_RECT from = { .Left = x, .Top = y, .Right = cols, .Bottom = rows - y - 1 };
	COORD dest = { .X = 0, .Y = y + 1 };
	CHAR_INFO info = { .Char = L' ', .Attributes = 0 };
	ScrollConsoleScreenBuffer(cmdwin(), &from, NULL, dest, &info);
#endif
}

void display_delete_line()
{
#ifdef NCURSES
	deleteln();
#else
	int x, y;
	int rows, cols;
	display_get_yx(&y, &x);
	display_get_rows_cols(&rows, &cols);
	x = 0;
	y += 1;
	SMALL_RECT from = { .Left = x, .Top = y, .Right = cols, .Bottom = rows - y };
	COORD dest = { .X = 0, .Y = y - 1 };
	CHAR_INFO info = { .Char = L' ', .Attributes = 0 };
	ScrollConsoleScreenBuffer(cmdwin(), &from, NULL, dest, &info);
#endif
}

void display_delete_char()
{
#ifdef NCURSES
	delch();
#else
	int x, y;
	display_get_yx(&y, &x);
	WriteConsole(cmdwin(), L" ", 1, NULL, NULL);

	// TODO:  Move the entire buffer over
	int rows, cols;
	display_get_rows_cols(&rows, &cols);
	const DWORD txtLen = (cols * (rows - y)) - x;
	DWORD readLen = 0;
	wchar_t* scrTxt = malloc(sizeof(wchar_t) * txtLen);
	COORD xy;
	xy.X = x + 1;
	xy.Y = y;
	ReadConsoleOutputCharacter(cmdwin(), scrTxt, txtLen, xy, &readLen);
	display_move(y, x);
	WriteConsole(cmdwin(), scrTxt, readLen, NULL, NULL);
	display_move(y, x);
	free(scrTxt);
#endif
}

void display_delete_char_at(const int y, const int x)
{
#ifdef NCURSES
	mvdelch(y, x);
#else
	display_move(y, x);
	display_delete_char();
#endif
}

#ifndef NCURSES
void DISPLAY_PRINT_STR(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	int len = vsnprintf(NULL, 0, format, args);
	char* str = malloc(len + 1);
	vsnprintf(str, len + 1, format, args);
	va_end(args);
	char16_t* wStr;
	utf8_to_str16(str, &wStr);
	free(str);
	WriteConsole(cmdwin(), wStr, len, NULL, NULL);
	free(wStr);
}
#endif