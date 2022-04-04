//
// Created by brent on 4/4/22.
//

#include "notes.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <db/query.h>
#include <display/ui.h>
#include <libc/string.h>
#include <display/input.h>
#include <display/display.h>
#include <display/text_input.h>

//https://www.sqlite.org/fts5.html
#define CREATE_NOTES_TABLE  "CREATE VIRTUAL TABLE Notes USING fts5(title, body);"
#define SELECT_FORMAT       "SELECT `rowid`, * FROM `Notes` WHERE `rowid`=?"
#define SAERCH_FORMAT       "SELECT `rowid`, * FROM `Notes` WHERE `title` MATCH ? OR `body` MATCH ? ORDER BY rank"
#define INSERT_FORMAT       "INSERT INTO `Notes` (`title`, `body`) VALUES (?, ?)"

typedef struct NotesQueryNode NotesQueryNode;
struct NotesQueryNode {
	int id;
	char* title;
	char* body;
	NotesQueryNode* next;
};

typedef struct {
	NotesQueryNode head;
	NotesQueryNode* current;
} NotesQueryList;

static inline void local_notes_query_free(NotesQueryNode* query) {
	free(query->title);
	free(query->body);
	if (query->next != NULL) {
		local_notes_query_free(query->next);
		free(query->next);
	}
}

static inline void local_query_list_free(NotesQueryList* list) {
	local_notes_query_free(&list->head);
	free(list);
}

static inline int init(Notes* notes) {
	int err = query_run(notes->db, "SELECT * FROM `Notes` LIMIT 1");
	if (err)
		err = query_run(notes->db, CREATE_NOTES_TABLE);
	return err;
}

static inline int wite_note(Notes* notes, const char* title, const char* body) {
	sqlite3_stmt* pStmt = NULL;
	int err = sqlite3_prepare_v2(notes->db, INSERT_FORMAT, -1, &pStmt, NULL);
	if (err)
		return err;
	else {
		sqlite3_bind_text(pStmt, 1, title, (int)strlen(title), NULL);
		sqlite3_bind_text(pStmt, 2, body, (int)strlen(body), NULL);
		if (sqlite3_step(pStmt) != SQLITE_DONE) {
			//sqlite3_errmsg
			return -1;
		}
		return sqlite3_finalize(pStmt);
	}
}

Notes* notes_new(volatile const bool* const prgSig) {
	Notes* notes = calloc(1, sizeof(*notes));
	notes->prgSig = prgSig;
	if (sqlite3_open("./nc.db", &notes->db) == 0) {
		int err = init(notes);
		if (err) {
			free(notes);
			return NULL;
		} else
			return notes;
	} else {
		free(notes);
		return NULL;
	}
}

void notes_free(Notes* notes) {
	sqlite3_close(notes->db);
	free(notes);
}

static void print_note(InputState* state, NotesQueryNode* q) {
	char output[4096];
	snprintf(output, sizeof(output), "ID:    %d\nTitle: %s\n%s",
		q->id, q->title, q->body);
	ui_clear_and_print(state->ui, output);
}

void notes_select(Notes* notes, InputState* state, int32_t id) {
	sqlite3_stmt* pStmt = NULL;
	int err = sqlite3_prepare_v2(notes->db, SELECT_FORMAT, -1, &pStmt, NULL);
	if (!err) {
		sqlite3_bind_int(pStmt, 1, id);
		if (sqlite3_step(pStmt) == SQLITE_ROW) {
			NotesQueryNode q;
			q.id = sqlite3_column_int(pStmt, 0);
			strclone((const char*)sqlite3_column_text(pStmt, 1), &q.title);
			strclone((const char*)sqlite3_column_text(pStmt, 2), &q.body);
			print_note(state, &q);
			local_notes_query_free(&q);
		} else
			ui_clear_and_print(state->ui, "Unable to locate the given note");
		sqlite3_finalize(pStmt);
	}
}

void notes_search(Notes* notes, InputState* state, const char* term) {
	sqlite3_stmt* pStmt = NULL;
	int err = sqlite3_prepare_v2(notes->db, SAERCH_FORMAT, -1, &pStmt, NULL);
	if (!err) {
		sqlite3_bind_text(pStmt, 1, term, (int)strlen(term), NULL);
		sqlite3_bind_text(pStmt, 2, term, (int)strlen(term), NULL);
		NotesQueryList* list = calloc(1, sizeof(*list));
		list->current = &list->head;
		while (sqlite3_step(pStmt) == SQLITE_ROW) {
			list->current->id = sqlite3_column_int(pStmt, 0);
			strclone((const char*)sqlite3_column_text(pStmt, 1), &list->current->title);
			strclone((const char*)sqlite3_column_text(pStmt, 2), &list->current->body);
			list->current->next = calloc(1, sizeof(*list->current->next));
			list->current = list->current->next;
			memset(list->current, 0, sizeof(*list->current));
		}
		sqlite3_finalize(pStmt);
		list->current = &list->head;
		if (list->current->id <= 0)
			ui_clear_and_print(state->ui, "Could not locate any matches");
		else if (list->head.next == NULL || list->head.next->id <= 0) {
			char output[4096];
			snprintf(output, sizeof(output), "ID:    %d\nTitle: %s\n%s",
				list->head.id, list->head.title, list->head.body);
			ui_clear_and_print(state->ui, output);
		} else {
			// TODO:  Pick an option
			char output[4096];
			int offset = 0;
			snprintf(output, sizeof(output), "Multiple entries found, pick an id...\n");
			offset = (int)strlen(output);
			while (list->current != NULL && list->current->id > 0 && offset+40 < (int)sizeof(output)) {
				snprintf(output+offset, 40, "(%d) %s\n",
					list->current->id, list->current->title);
				offset = (int)strlen(output);
				list->current = list->current->next;
			}
			ui_clear_and_print(state->ui, output);
			text_input_clear(state->command);
			ui_print_command_prompt(state->ui, state->command, ">\0", " \0");
			while (!*notes->prgSig) {
				if (text_input_read(state, DKEY_RETURN) && text_input_get_len(state->command) > 0) {
					int32_t num = strtoint32(text_input_get_buffer(state->command));
					notes_select(notes, state, num);
					break;
				}
			}
		}
		local_query_list_free(list);
	} else
		ui_clear_and_print(state->ui, "Failed to query the database");
}

void notes_create(Notes* notes, InputState* state) {
	ui_clear_and_print(state->ui, "Write a title for your note...");
	text_input_clear(state->command);
	ui_print_command_prompt(state->ui, state->command, ">\0", " \0");
	display_refresh();
	while (!*notes->prgSig) {
		if (text_input_read(state, DKEY_RETURN) && text_input_get_len(state->command) > 0) {
			char title[256];
			char titleLbl[sizeof(title)+256];
			snprintf(title, sizeof(title), "%s", text_input_get_buffer(state->command));
			snprintf(titleLbl, sizeof(titleLbl),
				"Title: %s\nNow write the body of your note (F2 to submit)...", title);
			ui_clear_and_print(state->ui, titleLbl);
			text_input_clear(state->command);
			ui_print_command_prompt(state->ui, state->command, ">\0", " \0");
			while (!*notes->prgSig) {
				if (text_input_read(state, DKEY_F2) && text_input_get_len(state->command) > 0) {
					int err = wite_note(notes, title, text_input_get_buffer(state->command));
					if (err)
						ui_clear_and_print(state->ui, "There was an issue creating your note... please try again");
					else
						ui_clear_and_print(state->ui, "Note created!");
					break;
				}
			}
			display_refresh();
			break;
		}
		display_refresh();
	}
}

void notes_edit(Notes* notes, InputState* state, int id) {

}

void notes_list(Notes* notes, InputState* state) {

}