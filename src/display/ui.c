#include "ui.h"
#include "input.h"
#include <string.h>
#include <assert.h>
#include "display.h"
#include <libc/string.h>

/************************************************************************/
/************************************************************************/
/* Pagination                                                           */
/************************************************************************/
/************************************************************************/
typedef struct PageBuffer PageBuffer;
struct PageBuffer {
	char* buffer;
	PageBuffer* next;
	PageBuffer* prev;
};

typedef struct {
	PageBuffer* head;
	PageBuffer* tail;
	PageBuffer* currentPage;
	int32_t currentPageIndex;
	int32_t pages;
	int32_t writeIndex;
} PageBook;

static void local_clear_book(PageBook* book)
{
	PageBuffer* pb = book->head;
	while (pb != NULL)
	{
		free(pb->buffer);
		PageBuffer* c = pb;
		pb = pb->next;
		free(c);
	}
	book->head = NULL;
	book->tail = NULL;
	book->currentPage = NULL;
}

static void local_print_page(PageBuffer* page)
{
	int fromX, fromY;
	display_get_yx(&fromY, &fromX);
	display_move(0, 0);
	DISPLAY_PRINT_STR("%s", page->buffer);
	display_move(fromY, fromX);
}

static void local_add_page_to_book(PageBook* book, const int32_t size)
{
	book->tail->buffer[book->writeIndex] = '\0';
	book->writeIndex = 0;
	book->tail->next = calloc(1, sizeof(PageBuffer));
	book->tail->next->buffer = malloc(size);
	book->tail->next->prev = book->tail;
	book->tail = book->tail->next;
	book->pages++;
}

static void local_add_to_book(PageBook* book, const char* text,
	const int32_t rows, const int32_t cols)
{
	int currentRows = 0;
	const int32_t printLen = (int32_t)strlen(text);
	int32_t nextSpace = stridxof(text, " \0", 0);
	int32_t lineOffset = book->writeIndex;
	const int32_t bufferSize = rows * cols;
	for (int32_t i = 0, pi = 0; i < printLen; ++i, ++pi)
	{
		char c = text[i];
		if (c == '\n')
		{
			lineOffset = i + 1;
			currentRows++;
		}
		if (i == nextSpace)
		{
			int32_t lastSpace = nextSpace;
			nextSpace = stridxof(text, " \0", i + 1);
			if (nextSpace > 0 && nextSpace - lineOffset > cols)
			{
				lineOffset = lastSpace + 1;
				currentRows++;
				c = '\n';
			}
		}
		if (pi == bufferSize - 1 || currentRows == rows)
		{
			local_add_page_to_book(book, rows * cols);
			currentRows = 0;
			if (c != '\n')
				nextSpace = stridxof(text, " \0", i + 1);
			pi = 0;
		}
		else
			book->tail->buffer[book->writeIndex++] = c;
	}
	book->tail->buffer[book->writeIndex] = '\0';
}

static void local_reset_book(PageBook* book, const int32_t pageSize)
{
	local_clear_book(book);
	book->pages = 1;
	book->currentPageIndex = 1;
	book->writeIndex = 0;
	book->head = calloc(1, sizeof(PageBuffer));
	book->head->buffer = malloc(pageSize);
	book->head->buffer[0] = '\0';
	book->tail = book->head;
	book->currentPage = book->tail;
}

/************************************************************************/
/************************************************************************/
/* Book tests                                                           */
/************************************************************************/
/************************************************************************/
static void local_test_book()
{
	const int32_t rows = 50;
	const int32_t cols = 100;
	const int32_t size = rows * cols;
	PageBook* book = calloc(1, sizeof(PageBook));
	local_reset_book(book, size);
	assert(book->head != NULL);
	assert(book->tail == book->head);
	assert(book->currentPage == book->head);
	assert(book->currentPageIndex == 1);
	assert(book->writeIndex == 0);
	assert(book->head->next == NULL);
	char msg[] = "This is a test";
	local_add_to_book(book, msg, rows, cols);
	assert(book->writeIndex == sizeof(msg) - 1);
	assert(strcmp(book->currentPage->buffer, msg) == 0);
	assert(book->currentPage == book->currentPage);
	assert(book->tail == book->currentPage);
	assert(book->currentPage->next == NULL);
	assert(book->currentPage->prev == NULL);
	local_add_to_book(book, msg, rows, cols);
	assert(book->writeIndex == (sizeof(msg) - 1) * 2);
	assert(strcmp(book->currentPage->buffer, "This is a testThis is a test\0") == 0);
	local_reset_book(book, size);
	assert(book->writeIndex == 0);
	assert(book->currentPage == book->head);
	assert(book->currentPage == book->tail);
	assert(book->currentPage->next == NULL);
	assert(book->currentPage->prev == NULL);
	local_clear_book(book);
	free(book);
}

/************************************************************************/
/************************************************************************/
/* User interface API                                                   */
/************************************************************************/
/************************************************************************/
struct ClientUI {
	PageBook* book;
	int32_t rows;
	int32_t cols;
	int writeX;
	int writeY;
};
static void local_print_info_bar(const ClientUI* ui);
static void local_clear_page_space(const ClientUI* ui);

ClientUI* ui_new()
{
	local_test_book();
	ClientUI* ui = calloc(1, sizeof(ClientUI));
	ui->book = calloc(1, sizeof(PageBook));
	local_clear_book(ui->book);
	return ui;
}

void ui_free(ClientUI* ui)
{
	local_clear_book(ui->book);
	free(ui->book);
	free(ui);
}

void ui_print_wrap(ClientUI* ui, const char* text)
{
	int fromX, fromY;
	display_get_yx(&fromY, &fromX);
	local_add_to_book(ui->book, text, ui->rows, ui->cols);
	local_print_page(ui->book->currentPage);
	local_print_info_bar(ui);
	display_move(fromY, fromX);
}

void ui_print_command_prompt(ClientUI* ui, struct TextInput* command, const char* prefix, const char* separator)
{
	int rows, cols;
	display_get_rows_cols(&rows, &cols);
	for (int32_t i = ui->rows + 1; i < rows; ++i)
	{
		display_move(ui->rows + 1, 0);
		display_clear_to_line_end();
	}
	display_move(ui->rows + 1, 0);
	DISPLAY_PRINT_STR("%s%s%s", prefix, separator, text_input_get_buffer(command));
	display_refresh();
}

void ui_page_next(ClientUI* ui)
{
	// TODO:  Fix copy/paste
	if (ui->book->currentPage->next == NULL)
		return;
	int fromX, fromY;
	display_get_yx(&fromY, &fromX);
	local_clear_page_space(ui);
	ui->book->currentPage = ui->book->currentPage->next;
	ui->book->currentPageIndex++;
	local_print_page(ui->book->currentPage);
	local_print_info_bar(ui);
	display_move(fromY, fromX);
	display_refresh();
}

void ui_page_prev(ClientUI* ui)
{
	// TODO:  Fix copy/paste
	if (ui->book->currentPage->prev == NULL)
		return;
	int fromX, fromY;
	display_get_yx(&fromY, &fromX);
	local_clear_page_space(ui);
	ui->book->currentPage = ui->book->currentPage->prev;
	ui->book->currentPageIndex--;
	local_print_page(ui->book->currentPage);
	local_print_info_bar(ui);
	display_move(fromY, fromX);
	display_refresh();
}

void ui_clear_and_print(ClientUI* ui, const char* text)
{
	display_clear();
	display_move(0, 0);
	local_reset_book(ui->book, ui->rows * ui->cols);
	ui_print_wrap(ui, text);
}

void ui_input_area_adjusted(ClientUI* ui, const size_t inputRows)
{
	int rows, cols;
	display_get_rows_cols(&rows, &cols);
	ui->rows = (int32_t)(rows - inputRows - 1);	/* -1 for information row */
	ui->cols = cols;

	// TODO:  Re-adjust pages count if needed
	local_reset_book(ui->book, ui->rows * ui->cols);
}

static void local_print_info_bar(const ClientUI* ui)
{
	int fromX, fromY;
	display_get_yx(&fromY, &fromX);
	display_move(ui->rows, 0);
	display_clear_to_line_end();
	for (int32_t i = 0; i < ui->cols; ++i)
		display_add_char('=');
	display_move(ui->rows, 3);
	DISPLAY_PRINT_STR(" Page %d of %d (page up/down = navigate) ", ui->book->currentPageIndex, ui->book->pages);
	display_move(fromY, fromX);
}

static void local_clear_page_space(const ClientUI* ui)
{
	for (int32_t i = 0; i < ui->rows; ++i)
	{
		display_move(i, 0);
		display_clear_to_line_end();
	}
}