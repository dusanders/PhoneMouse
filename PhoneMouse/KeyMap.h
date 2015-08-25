

#ifndef KEYMAP_H_
#define KEYMAP_H_

#define key_enter 0
#define key_shift 1
#define key_backspace 2

char KeyMap[123][3] = {{'q',KEY_Q,0},{'w',KEY_W,0},{'e',KEY_E,0},{'r',KEY_R,0},{'t',KEY_T,0},
		{'y',KEY_Y,0},{'u',KEY_U,0},{'i',KEY_I,0},{'o',KEY_O,0},{'p',KEY_P,0},{'\\',KEY_BACKSLASH,0},
		{'a',KEY_A,0,},{'s',KEY_S,0},{'d',KEY_D,0},{'f',KEY_F,0},{'g',KEY_G,0},{'h',KEY_H,0},
		{'j',KEY_J,0},{'k',KEY_K,0},{'l',KEY_L,0},{';',KEY_SEMICOLON,0},{8,KEY_BACKSPACE,0},
		{'z',KEY_Z,0},{'x',KEY_X,0},{'c',KEY_C,0},{'v',KEY_V,0},{' ',KEY_SPACE,0},{'\n',KEY_KPENTER,0},
		{'b',KEY_B,0},{'n',KEY_N,0},{'m',KEY_M,0},{',',KEY_COMMA,0},{'.',KEY_DOT,0},{'/',KEY_SLASH,0},
		{'?',KEY_QUESTION,0},{'Q',KEY_Q,1},{'W',KEY_W,1},{'E',KEY_E,1},{'R',KEY_R,1},{'T',KEY_T,1},
		{'Y',KEY_Y,1},{'U',KEY_U,1},{'I',KEY_I,1},{'O',KEY_O,1},{'P',KEY_P,1},{'A',KEY_A,1},{'S',KEY_S,1},
		{'D',KEY_D,1},{'F',KEY_F,1},{'G',KEY_G,1},{'H',KEY_H,1},{'J',KEY_J,1},{'K',KEY_K,1},{'L',KEY_L,1},
		{'Z',KEY_Z,1},{'X',KEY_X,1},{'C',KEY_C,1},{'V',KEY_V,1},{'B',KEY_B,1},{'N',KEY_N,1},{'M',KEY_M,1},
		{'1',KEY_1,0},{'2',KEY_2,0},{'3',KEY_3,0},{'4',KEY_4,0},{'5',KEY_5,0},{'6',KEY_6,0},{'7',KEY_7,0},
		{'8',KEY_8,0},{'9',KEY_9,0},{'0',KEY_0,0},{'=',KEY_EQUAL,0},{'+',KEY_KPPLUS,0}};
#endif
