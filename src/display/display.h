#ifndef LTTP_CLIENT_DISPLAY_H
#define LTTP_CLIENT_DISPLAY_H

#include <stdint.h>
#if !defined(_WIN32) && !defined(_WIN64)
#include <ncurses.h>
//http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/
#define NCURSES
#define DKEY_F2                 266
#define DKEY_PAGE_UP			339
#define DKEY_PAGE_DOWN			338
#define DKEY_DOWN_ARROW			KEY_DOWN
#define DKEY_UP_ARROW			KEY_UP
#define DKEY_LEFT_ARROW			KEY_LEFT
#define DKEY_RIGHT_ARROW		KEY_RIGHT
#define DKEY_CTRL_DOWN_ARROW	525
#define DKEY_CTRL_UP_ARROW		566
#define DKEY_CTRL_LEFT_ARROW	545
#define DKEY_CTRL_RIGHT_ARROW	560
#define DKEY_RETURN				10
#define DKEY_DELETE				8
#define DKEY_BACKSPACE			KEY_BACKSPACE
#else
// TODO:  Make these work
#include <stdio.h>
#include <wchar.h>
#include <uchar.h>
#include <windows.h>
#define ERR						-1
#define DKEY_PAGE_UP			457
#define DKEY_PAGE_DOWN			465
#define DKEY_DOWN_ARROW			464
#define DKEY_UP_ARROW			456
#define DKEY_LEFT_ARROW			459
#define DKEY_RIGHT_ARROW		461
#define DKEY_CTRL_DOWN_ARROW	-8
#define DKEY_CTRL_UP_ARROW		-9
#define DKEY_CTRL_LEFT_ARROW	499
#define DKEY_CTRL_RIGHT_ARROW	500
#define DKEY_RETURN				13
#define DKEY_DELETE				467
#define DKEY_BACKSPACE			8
#define DKEY_F2					60
static inline HWND cmdwin() { return GetStdHandle(STD_OUTPUT_HANDLE); }
static inline HWND cmdwinin() { return GetStdHandle(STD_INPUT_HANDLE); }
#endif

void display_init();
void display_quit();
void display_clear();
void display_move(int y, int x);
void display_refresh();
int display_get_char();
void display_get_yx(int* outY, int* outX);
void display_get_rows_cols(int* outRows, int* outCols);
void display_clear_to_line_end();
void display_add_char(uint16_t utf8Letter);
void display_insert_line();
void display_delete_line();
void display_delete_char();
void display_delete_char_at(int y, int x);

#ifdef NCURSES
#define DISPLAY_PRINT_STR(format, ...) printw((format), __VA_ARGS__)
#else
void DISPLAY_PRINT_STR(const char* format, ...);
#endif

#endif
