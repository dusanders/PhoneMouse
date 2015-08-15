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

#define ARG_WIFI "-wifi"
#define ARG_LAN "-lan"
#define ARG_CHG_FILE "-f"

#define BUFF_LEN 255

char buffer[BUFF_LEN];

int openSystemFile(char *SYSTEM_INPUT_FILE, FILE* inputFile,
		struct uinput_user_dev uinputUser);

int main(int argC, char* argVec[]) {

	char* SYSTEM_INPUT_FILE = "/dev/uinput";
	FILE* inputFile = NULL;
	int fileRef;
	struct uinput_user_dev uinputUser;
	int dx,dy;

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
	buffer[0] = 0;

	/*
	 * INSERTING EVENTS INTO UINPUT
	 */
	struct input_event event;
	int i;
	while(scanf("%d %d", &dx, &dy)) {
		printf("Moving to: dx: %d, dy: %d\n",dx,dy);
		for(i=0; i<10;i++) {
			//clear the event struct
			memset(&event, 0, sizeof(struct input_event));
			//set the event parameters
			event.type = EV_REL;
			event.code = REL_X;
			event.value = dx;
			//write the event
			if(write(fileRef, &event, sizeof(struct input_event)) < 0)
				printf("Error writing event!\n");
			//clear the event
			memset(&event, 0, sizeof(struct input_event));
			//set the parameters
			event.type = EV_REL;
			event.code = REL_Y;
			event.value = dy;
			//write the event
			if(write(fileRef, &event, sizeof(struct input_event)) < 0)
				printf("Error writing event!\n");
			//clear the event struct
			memset(&event, 0, sizeof(struct input_event));
			event.type = EV_SYN;
			event.code = 0;
			event.value = 0;
			//write the event
			if(write(fileRef, &event, sizeof(struct input_event)) < 0)
				printf("Error writing event!\n");
		}
	}
	/*
	memset(&event, 0, sizeof(struct input_event));
	event.type = EV_KEY;
	event.code = BTN_RIGHT;
	event.value = 1;
	if(write(fileRef, &event, sizeof(struct input_event)) < 0)
		printf("Error writing click event!\n");
		*/
	printf("closing file\n");
	if(ioctl(fileRef, UI_DEV_DESTROY) < 0) {
		printf("Error destroying device!\n");
		return 0;
	}
	close(fileRef);
	INetDisconnect();
	return 0;
}

int openSystemFile(char *SYSTEM_INPUT_FILE, FILE* inputFile,
		struct uinput_user_dev uinputUser) {

	inputFile = fopen(SYSTEM_INPUT_FILE, "w");
	int fileRef = fileno(inputFile);
	if(inputFile == NULL) {
		printf("Error opening %s\n", SYSTEM_INPUT_FILE);
		return -1;
	}
	if(ioctl(fileRef, UI_SET_EVBIT, EV_KEY) < 0) {
		printf("Error setting EV_KEY\n");
		return -1;
	}
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
	if(ioctl(fileRef, UI_DEV_CREATE) < 0) {
			printf("Error creating device\n");
			return -1;
	}
	return fileRef;
};

