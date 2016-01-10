/*
 * main.c
 *
 *  Created on: Jul 22, 2015
 *      Author: zena-laptop
 */
#include <linux/input.h>
#include <linux/uinput.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include "INetHandler.h"
#include "KeyMap.h"

#define ARG_WIFI "-wifi"
#define ARG_LAN "-lan"
#define ARG_CHG_FILE "-f"
#define MOUSE_EVENT 1
#define KEYBOARD_EVENT 2
#define QUERY_EVENT 3
#define BUFF_LEN 5

struct MouseEvent {
	float dx;
	float dy;
	int mouseLeft;
	int mouseRight;
};
struct KeyboardEvent {
	int keyCode;
	int uppercase;
};

void syncEvent(int fileSysRef);

void keyboardKey(struct KeyboardEvent, int sysFileRef);

void clickMouse(struct MouseEvent event, int sysFileRef);

void moveMouse(struct MouseEvent event, int sysFileRef);

int openSystemFile(char *SYSTEM_INPUT_FILE, FILE* inputFile,
		struct uinput_user_dev uinputUser);

int setBitMap(int fileRef);

int main(int argC, char* argVec[]) {

	char* SYSTEM_INPUT_FILE = "/dev/uinput";
	FILE* inputFile = NULL;
	int fileRef;
	struct uinput_user_dev uinputUser;
	char buffer[BUFF_LEN];
	memset(buffer, '\0', sizeof(buffer));


	if(argC < 2 || argC > 3) {
		printf("Error! Invalid arguments!\n");
		printf("./PhoneMouse [-wifi || -lan] || [-option] [file name]\n");
		printf("Options: -f\t\t--change System Input File\n");
		return -1;
	}
	if(strcmp(argVec[1], ARG_WIFI) == 0) {
		if(INetConnect(TYPE_WIFI, buffer) < 0) {
			printf("Error opening socket!\n");
		}
		printf("%s\n", INetGetIp(TYPE_WIFI));
	}
	else if(strcmp(argVec[1], ARG_LAN) == 0) {
		if(INetConnect(TYPE_LAN, buffer) < 0) {
			printf("Error opening socket!\n");
			return -1;
		}
		printf("%s\n", INetGetIp(TYPE_LAN));
	}
	else if(strcmp(argVec[1], ARG_CHG_FILE) == 0) {
		if(argC < 3 || argC >3) {
			printf("Error parsing arguments!\n");
			printf("./PhoneMouse [-option] [file name]");
			return -1;
		}
		printf("Changing SYSTEM_INPUT_FILE to %s\n", argVec[2]);
		SYSTEM_INPUT_FILE = argVec[2];
	}
	else {
		printf("Error! Invalid arguments!\n");
		printf("./PhoneMouse [-wifi || -lan] || [-option] [file name]\n");
		printf("Options: -f\t\t--change System Input File\n");
		return -1;
	}
	if((fileRef = openSystemFile(SYSTEM_INPUT_FILE, inputFile, uinputUser)) == 0) {
		printf("Error! Exiting....\n");
		return -1;
	}
	while(1) {
		if(INET_DATA_READY_FLAG) {
			//reset the data ready flag
			if(buffer[0] == MOUSE_EVENT) {
				struct MouseEvent event;
				event.dx = buffer[1];
				event.dy = buffer[2];
				event.mouseLeft = buffer[3];
				event.mouseRight = buffer[4];
				moveMouse(event,fileRef);
				if(event.mouseLeft || event.mouseRight) {
					clickMouse(event, fileRef);
				}
			}
			else if(buffer[0] == KEYBOARD_EVENT) {
				struct KeyboardEvent event;
				int i=0;
				for(; i < 120; i++) {
					if(buffer[1] == KeyMap[i][0]){
						printf("%c", (char)KeyMap[i][0]);
						event.keyCode = KeyMap[i][1];
						event.uppercase = KeyMap[i][2];
						continue;
					}
				}
				keyboardKey(event, fileRef);
			}
			else if(buffer[0] == QUERY_EVENT) {
				unsigned char response[] = {3};
				INetSend(response);
			}
			memset(&buffer,0,sizeof(buffer));
			INET_DATA_READY_FLAG = 0;
		}
	}
	printf("closing file\n");
	if(ioctl(fileRef, UI_DEV_DESTROY) < 0) {
		printf("Error destroying device!\n");
		return 0;
	}
	close(fileRef);
	INetDisconnect();
	return 0;
}//end main()

void syncEvent(int sysFileRef) {
	struct input_event sysEvent;
	memset(&sysEvent, 0, sizeof(struct input_event));
	sysEvent.type = EV_SYN;
	sysEvent.code = 0;
	sysEvent.value = 0;
	if(write(sysFileRef, &sysEvent,sizeof(struct input_event)) < 0) {
		printf("Error sync write!\n");
	}
}

void keyboardKey(struct KeyboardEvent event, int sysFileRef) {
	struct input_event sysEvent;
	memset(&sysEvent, 0, sizeof(struct input_event));
	if(event.uppercase) {
		sysEvent.type = EV_KEY;
		sysEvent.code = KEY_LEFTSHIFT;
		sysEvent.value = 1;
		if(write(sysFileRef, &sysEvent, sizeof(struct input_event)) < 0) {
			printf("Error writing shift key!\n");
		}
		syncEvent(sysFileRef);
	}
	sysEvent.type = EV_KEY;
	sysEvent.code = event.keyCode;
	sysEvent.value = 1;
	if(write(sysFileRef, &sysEvent, sizeof(struct input_event)) < 0) {
		printf("Error writing key event!\n");
	}
	syncEvent(sysFileRef);
	sysEvent.type = EV_KEY;
	sysEvent.code = event.keyCode;
	sysEvent.value = 0;
	if(write(sysFileRef, &sysEvent, sizeof(struct input_event)) < 0) {
		printf("Error writing unkey event!\n");
	}
	syncEvent(sysFileRef);
	if(event.uppercase) {
		sysEvent.type = EV_KEY;
		sysEvent.code = KEY_LEFTSHIFT;
		sysEvent.value = 0;
		if(write(sysFileRef, &sysEvent, sizeof(struct input_event)) < 0) {
			printf("Error writing shift key!\n");
		}
		syncEvent(sysFileRef);
	}
}

void moveMouse(struct MouseEvent event, int sysFileRef) {
	struct input_event sysEvent;
	printf("Moving to: dx: %f, dy: %f\n",event.dx,event.dy);
	//clear the event struct
	memset(&sysEvent, 0, sizeof(struct input_event));
	//set the event parameters
	sysEvent.type = EV_REL;
	sysEvent.code = REL_X;
	sysEvent.value = event.dx;
	//write the event
	if(write(sysFileRef, &sysEvent, sizeof(struct input_event)) < 0)
		printf("Error writing event!\n");
	//clear the event
	memset(&sysEvent, 0, sizeof(struct input_event));
	//set the parameters
	sysEvent.type = EV_REL;
	sysEvent.code = REL_Y;
	sysEvent.value = event.dy;
	//write the event
	if(write(sysFileRef, &sysEvent, sizeof(struct input_event)) < 0)
		printf("Error writing event!\n");
	syncEvent(sysFileRef);
 }

void clickMouse(struct MouseEvent event, int sysFileRef) {
	if(event.mouseRight) {
		printf("Right click!\n");
		struct input_event sysEvent;
		memset(&sysEvent, 0, sizeof(struct input_event));
		sysEvent.type = EV_KEY;
		sysEvent.code = BTN_RIGHT;
		sysEvent.value = 1;
		if(write(sysFileRef, &sysEvent, sizeof(struct input_event)) < 0)
			printf("Error writing click event!\n");
		syncEvent(sysFileRef);
		memset(&sysEvent, 0, sizeof(struct input_event));
		sysEvent.type = EV_KEY;
		sysEvent.code = BTN_RIGHT;
		sysEvent.value = 0;
		if(write(sysFileRef, &sysEvent, sizeof(struct input_event)) < 0)
			printf("Error writing unclick event!\n");
		syncEvent(sysFileRef);
	}
	if(event.mouseLeft) {
		printf("Left Click!!\n");
		struct input_event sysEvent;
		memset(&sysEvent, 0, sizeof(struct input_event));
		sysEvent.type = EV_KEY;
		sysEvent.code = BTN_LEFT;
		sysEvent.value = 1;
		if(write(sysFileRef, &sysEvent, sizeof(struct input_event)) < 0)
			printf("Error writing click event!\n");
		syncEvent(sysFileRef);
		memset(&sysEvent, 0, sizeof(struct input_event));
		sysEvent.type = EV_KEY;
		sysEvent.code = BTN_LEFT;
		sysEvent.value = 0;
		if(write(sysFileRef, &sysEvent, sizeof(struct input_event)) < 0)
			printf("Error writing unclick event!\n");
		syncEvent(sysFileRef);
	}
}

int openSystemFile(char *SYSTEM_INPUT_FILE, FILE* inputFile,
		struct uinput_user_dev uinputUser) {

	inputFile = fopen(SYSTEM_INPUT_FILE, "w");
	int fileRef = fileno(inputFile);
	if(inputFile == NULL) {
		printf("Error opening %s\n", SYSTEM_INPUT_FILE);
		return -1;
	}
	memset(&uinputUser, 0, sizeof(uinputUser));
	snprintf(uinputUser.name, 80, "Zena Test");
	uinputUser.id.bustype = BUS_USB;
	uinputUser.id.vendor = 0x1;
	uinputUser.id.product = 0x1;
	uinputUser.id.version = 1;
	if(write(fileRef, &uinputUser, sizeof(uinputUser)) < 0) {
		printf("Error writing file!");
		return -1;
	}

	if(setBitMap(fileRef) < 0) {
		printf("Error setting bit map for input device!\n");
	}
	if(ioctl(fileRef, UI_DEV_CREATE) < 0) {
			printf("Error creating device\n");
			return -1;
	}
	sleep(1);
	return fileRef;
};

int setBitMap(int fileRef) {
	if(ioctl(fileRef, UI_SET_EVBIT, EV_KEY) < 0) {
		printf("Error setting EV_KEY\n");
		return -1;
	}
	if(ioctl(fileRef, UI_SET_KEYBIT, KEY_SPACE) < 0) {
			printf("Error setting BIT\n");
			return -1;
	}

	if(ioctl(fileRef, UI_SET_KEYBIT, KEY_LEFTSHIFT) < 0) {
			printf("Error setting BIT\n");
			return -1;
	}

	if(ioctl(fileRef, UI_SET_KEYBIT, KEY_Q) < 0) {
			printf("Error setting BIT\n");
			return -1;
	}

	if(ioctl(fileRef, UI_SET_KEYBIT, KEY_W) < 0) {
			printf("Error setting BIT\n");
			return -1;
	}

	if(ioctl(fileRef, UI_SET_KEYBIT, KEY_E) < 0) {
			printf("Error setting BIT\n");
			return -1;
	}
	if(ioctl(fileRef, UI_SET_KEYBIT, KEY_R) < 0) {
		printf("Error setting BIT\n");
	}
	ioctl(fileRef, UI_SET_KEYBIT, KEY_T);
	ioctl(fileRef, UI_SET_KEYBIT, KEY_Y);
	ioctl(fileRef, UI_SET_KEYBIT, KEY_U);
	ioctl(fileRef, UI_SET_KEYBIT, KEY_I);
	ioctl(fileRef, UI_SET_KEYBIT, KEY_O);
	ioctl(fileRef, UI_SET_KEYBIT, KEY_P);
	ioctl(fileRef, UI_SET_KEYBIT, KEY_A);
	ioctl(fileRef, UI_SET_KEYBIT, KEY_S);
	ioctl(fileRef, UI_SET_KEYBIT, KEY_D);
	ioctl(fileRef, UI_SET_KEYBIT, KEY_F);
	ioctl(fileRef, UI_SET_KEYBIT, KEY_G);
	ioctl(fileRef, UI_SET_KEYBIT, KEY_H);
	ioctl(fileRef, UI_SET_KEYBIT, KEY_J);
	ioctl(fileRef, UI_SET_KEYBIT, KEY_K);
	ioctl(fileRef, UI_SET_KEYBIT, KEY_L);
	ioctl(fileRef, UI_SET_KEYBIT, KEY_Z);
	ioctl(fileRef, UI_SET_KEYBIT, KEY_X);
	ioctl(fileRef, UI_SET_KEYBIT, KEY_C);
	ioctl(fileRef, UI_SET_KEYBIT, KEY_V);
	ioctl(fileRef, UI_SET_KEYBIT, KEY_B);
	ioctl(fileRef, UI_SET_KEYBIT, KEY_N);
	ioctl(fileRef, UI_SET_KEYBIT, KEY_M);
	ioctl(fileRef, UI_SET_KEYBIT, KEY_BACKSPACE);
	if(ioctl(fileRef, UI_SET_KEYBIT, BTN_LEFT) < 0) {
		printf("Error setting BTN_LEFT\n");
		return -1;
	}
	if(ioctl(fileRef, UI_SET_KEYBIT, BTN_RIGHT) < 0) {
		printf("Error setting BTN_RIGHT\n");
		return -1;
	}
	if(ioctl(fileRef, UI_SET_EVBIT, EV_REL) < 0) {
		printf("Error setting EVBIT EV_REL\n");
		return -1;
	}
	if(ioctl(fileRef, UI_SET_RELBIT, REL_X) < 0) {
		printf("Error setting REL_X\n");
		return -1;
	}
	if(ioctl(fileRef, UI_SET_RELBIT, REL_Y) < 0) {
		printf("Error setting REL_Y\n");
		return -1;
	}
	return fileRef;
}
