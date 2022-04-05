#include <signal.h>
#include <string.h>
#include <stdbool.h>
#include <display/ui.h>
#include <notes/notes.h>
#include <libc/string.h>
#include <display/input.h>
#include <display/display.h>
#include <display/text_input.h>

#define INPUT_BUFFER_SIZE	65556
static volatile bool s_quit = 0;

#define SPLASH	\
"_________________________________________________________________________________\n"	\
"    _     _                          __                                          \n"	\
"    /|   /                         /    )                                       /\n"	\
"---/-| -/-----__--_/_----__-------/---------__---_--_---_--_----__----__----__-/-\n"	\
"  /  | /    /   ) /    /___)     /        /   ) / /  ) / /  ) /   ) /   ) /   /  \n"	\
"_/___|/____(___/_(_ __(___ _____(____/___(___/_/_/__/_/_/__/_(___(_/___/_(___/___\n"	\
"\n"									\
"Commands:\n"							\
"new - Create a new note\n"				\
"list - List all notes\n"				\
"delete [id] - Delete a note\n"			\
"find [query] - Search all notes\n"		\
"[id] - View a note matching this id\n"	\
"clear - Clear the screen"

static inline void local_interrupt_handler(int sig) {
	s_quit = true;
}

int main(int argc, char** argv) {
	signal(SIGINT, local_interrupt_handler);
	s_quit = false;
	display_init();
	display_move(0, 0);
	InputState state;
	state.command = text_input_new(INPUT_BUFFER_SIZE);
	state.ui = ui_new();
	ui_input_area_adjusted(state.ui, 1);
	ui_clear_and_print(state.ui, SPLASH "\n");
	ui_print_command_prompt(state.ui, state.command, ">\0", " \0");
	Notes* notes = notes_new(&s_quit);
	while (!s_quit) {
		if (text_input_read(&state, DKEY_RETURN) && text_input_get_len(state.command) > 0) {
			if (strcmp(text_input_get_buffer(state.command), "exit") == 0) {
				s_quit = true;
			} else if (strcmp(text_input_get_buffer(state.command), "clear") == 0) {
				ui_clear_and_print(state.ui, SPLASH "\n");
				ui_print_command_prompt(state.ui, state.command, ">\0", " \0");
			} else if (strcmp(text_input_get_buffer(state.command), "list") == 0) {
				ui_clear_and_print(state.ui, "");
				notes_list(notes, &state);
			} else if (strcmp(text_input_get_buffer(state.command), "create") == 0
				|| strcmp(text_input_get_buffer(state.command), "new") == 0)
			{
				notes_create(notes, &state);
			} else if (strcmp(text_input_get_buffer(state.command), "search") == 0
				|| strcmp(text_input_get_buffer(state.command), "find") == 0)
			{
				ui_clear_and_print(state.ui, "Enter your search term...");
				text_input_clear(state.command);
				ui_print_command_prompt(state.ui, state.command, ">\0", " \0");
				while (true) {
					if (text_input_read(&state, DKEY_RETURN) && text_input_get_len(state.command) > 0) {
						notes_search(notes, &state, text_input_get_buffer(state.command));
						break;
					}
					display_refresh();
				}
			} else if (stridxof(text_input_get_buffer(state.command), "find ", 0) == 0)
				notes_search(notes, &state, text_input_get_buffer(state.command)+5);
			else if (stridxof(text_input_get_buffer(state.command), "search ", 0) == 0)
				notes_search(notes, &state, text_input_get_buffer(state.command)+7);
			else if (stridxof(text_input_get_buffer(state.command), "delete ", 0) == 0) {
				int id = strtoint32(text_input_get_buffer(state.command)+7);
				notes_delete(notes, &state, id);
			} else {
				int id = strtoint32(text_input_get_buffer(state.command));
				notes_select(notes, &state, id);
			}
			text_input_clear(state.command);
			ui_print_command_prompt(state.ui, state.command, ">\0", " \0");
		}
		display_refresh();
	}
	notes_free(notes);
	display_clear();
	DISPLAY_PRINT_STR("%s", "Cancel has been invoked! Closing connection with server...");
	ui_free(state.ui);
	text_input_free(state.command);
	display_quit();
	return 0;
}
