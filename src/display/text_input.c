#include "ui.h"
#include <string.h>
#include "display.h"
#include "text_input.h"
#include <libc/string.h>

struct TextInput {
	char* buffer;
	size_t maxLen;
	size_t endIndex;
	size_t writeIndex;
	int32_t lines;
};

TextInput* text_input_new(const size_t len) {
	TextInput* ti = calloc(1, sizeof(TextInput));
	ti->buffer = malloc(len);
	ti->buffer[0] = '\0';
	ti->maxLen = len;
	return ti;
}

void text_input_free(TextInput* input) {
	free(input->buffer);
	free(input);
}

static inline void local_write_letter(TextInput* input, int c) {
	if (input->endIndex < input->maxLen - 1) {
		const size_t ewDiff = input->endIndex - input->writeIndex;
		if (input->writeIndex != input->endIndex)
			memcpy(input->buffer + input->writeIndex + 1, input->buffer + input->writeIndex, ewDiff);
		input->buffer[input->writeIndex] = c;
		input->buffer[++input->endIndex] = '\0';
		DISPLAY_PRINT_STR("%s", input->buffer + input->writeIndex);
		input->writeIndex++;

		int x, y, rows, cols;
		display_get_yx(&y, &x);
		display_get_rows_cols(&rows, &cols);
		if (ewDiff > 0)
			display_move(y - ((int)ewDiff / cols), x - ((int)ewDiff % cols));

		if ((y == rows - 1 && x == cols - 1) || c == '\n') {
			input->lines++;
			display_move(rows - (input->lines + 2), 0);
			display_delete_line();
			display_move(rows - 1, 0);
		}
	}
}

// TODO:  Swap this back to TextInput* input when proper key handling is done
bool text_input_read(InputState* state, int match) {
	TextInput* input = state->command;
	int c = display_get_char();
	if (c != ERR) {
		switch (c) {
			case DKEY_PAGE_UP:
				ui_page_prev(state->ui);
				break;
			case DKEY_PAGE_DOWN:
				ui_page_next(state->ui);
				break;
			case DKEY_CTRL_DOWN_ARROW:
			case DKEY_DOWN_ARROW:
				break;
			case DKEY_CTRL_UP_ARROW:
			case DKEY_UP_ARROW:
				break;
			case DKEY_CTRL_LEFT_ARROW:
				if (input->writeIndex > 0) {
					for (int32_t i = (int32_t)input->writeIndex - 1; i >= 0; --i) {
						if (i == 0 || input->buffer[i - 1] == ' ') {
							int x, y, rows, cols;
							display_get_yx(&y, &x);
							display_get_rows_cols(&rows, &cols);
							x -= (int)(input->writeIndex - i);
							while (x < 0)
							{
								x += cols - 1;
								y--;
							}
							display_move(y, x % cols);
							input->writeIndex = i;
							break;
						}
					}
				}
				break;
			case DKEY_LEFT_ARROW:
				if (input->writeIndex > 0) {
					int x, y, rows, cols;
					display_get_yx(&y, &x);
					display_get_rows_cols(&rows, &cols);

					input->writeIndex--;
					if (x == 0)
						display_move(y - 1, cols - 1);
					else
						display_move(y, x - 1);
				}
				break;
			case DKEY_CTRL_RIGHT_ARROW:
				if (input->writeIndex < input->endIndex) {
					for (size_t i = input->writeIndex + 1; i <= input->endIndex; ++i) {
						if (i == input->endIndex || input->buffer[i - 1] == ' ') {
							int x, y, rows, cols;
							display_get_yx(&y, &x);
							display_get_rows_cols(&rows, &cols);
							x += (int)(i - input->writeIndex);
							while (x > cols)
							{
								x -= cols - 1;
								y++;
							}
							display_move(y, x);
							input->writeIndex = i;
							break;
						}
					}
				}
				break;
			case DKEY_RIGHT_ARROW:
				if (input->writeIndex < input->endIndex) {
					int x, y, rows, cols;
					display_get_yx(&y, &x);
					display_get_rows_cols(&rows, &cols);

					input->writeIndex++;
					if (x == cols - 1)
						display_move(y + 1, 0);
					else
						display_move(y, x + 1);
				}
				break;
			case DKEY_RETURN:
				if (match == DKEY_RETURN) {
					input->buffer[input->endIndex] = '\0';
					input->endIndex = 0;
					input->writeIndex = 0;
					trim(input->buffer);
				} else
					local_write_letter(input, '\n');
				break;
			case DKEY_DELETE:
			case DKEY_BACKSPACE:
				if (input->writeIndex > 0) {
					int x, y;
					display_get_yx(&y, &x);
					if (x > 0)
						display_delete_char_at(y, x - 1);
					else {
						int rows, cols;
						display_get_rows_cols(&rows, &cols);
						display_delete_char_at(y - 1, cols - 2);
						display_move(y - (input->lines + 1), 0);
						input->lines--;
						display_insert_line();
						display_move(rows - 1, cols - 2);
					}
					if (input->endIndex > 0) {
						if (input->writeIndex != input->endIndex)
							memcpy(input->buffer + input->writeIndex - 1, input->buffer + input->writeIndex, input->endIndex - input->writeIndex);
						input->buffer[--input->endIndex] = '\0';
						input->writeIndex--;
					}
				}
				break;
			default:
				local_write_letter(input, c);
				break;
		}
		display_refresh();
	}
	return c == match;
}

const char* text_input_get_buffer(const TextInput* input) {
	return input->buffer;
}

size_t text_input_get_len(const TextInput* input) {
	return strlen(input->buffer);
}

void text_input_clear(TextInput* input) {
	input->buffer[0] = '\0';
}
