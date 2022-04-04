#ifndef LTTP_CLIENT_TEXT_INPUT_H
#define LTTP_CLIENT_TEXT_INPUT_H

#include "input.h"
#include <stdlib.h>
#include <stdbool.h>

#define TEXT_INPUT_MATCH_RETURN DKEY_RETURN

typedef struct TextInput TextInput;

TextInput* text_input_new(size_t len);
void text_input_free(TextInput* input);
bool text_input_read(InputState* state, int match);
const char* text_input_get_buffer(const TextInput* input);
size_t text_input_get_len(const TextInput* input);
void text_input_clear(TextInput* input);

#endif