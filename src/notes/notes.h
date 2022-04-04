//
// Created by brent on 4/4/22.
//

#ifndef NOTECOMMANDER_NOTES_H
#define NOTECOMMANDER_NOTES_H

#include <stdint.h>
#include <stdbool.h>
#include <ext/sqlite3.h>
#include <display/input.h>

typedef struct {
	sqlite3* db;
	volatile const bool* prgSig;
} Notes;

Notes* notes_new(volatile const bool* prgSig);
void notes_free(Notes* notes);
void notes_select(Notes* notes, InputState* state, int32_t id);
void notes_search(Notes* notes, InputState* state, const char* term);
void notes_create(Notes* notes, InputState* state);
void notes_edit(Notes* notes, InputState* state, int id);
void notes_list(Notes* notes, InputState* state);

#endif
