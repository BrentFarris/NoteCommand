#ifndef LTTP_INPUT_H
#define LTTP_INPUT_H

typedef struct ClientUI ClientUI;
typedef struct TextInput TextInput;

typedef struct {
	TextInput* command;
	ClientUI* ui;
} InputState;

#endif