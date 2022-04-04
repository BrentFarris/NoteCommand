#ifndef LTTP_CLIENT_UI_H
#define LTTP_CLIENT_UI_H

#include "text_input.h"

typedef struct ClientUI ClientUI;

ClientUI* ui_new();
void ui_free(ClientUI* ui);
void ui_print_wrap(ClientUI* ui, const char* text);
void ui_print_command_prompt(ClientUI* ui,
	struct TextInput* command, const char* prefix, const char* separator);
void ui_page_next(ClientUI* ui);
void ui_page_prev(ClientUI* ui);
void ui_clear_and_print(ClientUI* ui, const char* text);
void ui_input_area_adjusted(ClientUI* ui, size_t inputRows);

#endif