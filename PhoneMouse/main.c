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



int main(int argC, char* argVec[]) {

	if(INetConnect(TYPE_LAN) < 0) {
		printf("Error opening socket!\n");
	}
	printf("%s", INetGetIp(TYPE_LAN));
	char* SYSTEM_INPUT_FILE = "/dev/uinput";
	FILE* inputFile;
	struct uinput_user_dev uinputUser;
	int dx,dy;

	if(argC > 1) {
		char* validArg = "-f";
		if(*(argVec[1]) == *(validArg) && argC == 3) {
			printf("Changing SYSTEM_INPUT_FILE to %s\n", argVec[2]);
			SYSTEM_INPUT_FILE = argVec[2];
		}
		else {
			printf("Error with arguments!\n");
			printf("Continuing.....");
		}
	}
	inputFile = fopen(SYSTEM_INPUT_FILE, "w");
	int fileRef = inputFile->_fileno;
	if(inputFile == NULL) {
		printf("Error opening %s\n", SYSTEM_INPUT_FILE);
	}
	if(ioctl(fileRef, UI_SET_EVBIT, EV_KEY) < 0)
		printf("Error setting EV_KEY\n");
	if(ioctl(fileRef, UI_SET_KEYBIT, BTN_LEFT) < 0)
		printf("Error setting BTN_LEFT\n");
	if(ioctl(fileRef, UI_SET_KEYBIT, BTN_RIGHT) < 0)
		printf("Error setting BTN_RIGHT\n");
	if(ioctl(fileRef, UI_SET_EVBIT, EV_REL) < 0)
		printf("Error setting EVBIT EV_REL\n");
	if(ioctl(fileRef, UI_SET_RELBIT, REL_X) < 0)
		printf("Error setting REL_X\n");
	if(ioctl(fileRef, UI_SET_RELBIT, REL_Y) < 0)
		printf("Error setting REL_Y\n");
	memset(&uinputUser, 0, sizeof(uinputUser));
	snprintf(uinputUser.name, 80, "Zena Test");
	uinputUser.id.bustype = BUS_USB;
	uinputUser.id.vendor = 0x1;
	uinputUser.id.product = 0x1;
	uinputUser.id.version = 1;
	if(write(fileRef, &uinputUser, sizeof(uinputUser)) < 0)
		printf("Error writing file!");
	/*
	if(fwrite(&uinputUser, sizeof(uinputUser), 1, inputFile) < 0)
		printf("Error writing to file!");
		*/
	if(ioctl(fileRef, UI_DEV_CREATE) < 0)
		printf("Error creating device\n");

	/*
	 * INSERTING EVENTS INTO UINPUT
	 */
	struct input_event event;
	int i;
	while(1) {
		scanf("%d %d", &dx, &dy);
		/*
		char moveChar;
		if(moveChar == 'd') {
			dx = 1;
			dy = 0;
		}
		else if(moveChar == 'a') {
			dx = -1;
			dy = 0;
		}
		else if(moveChar == 'w') {
			dx = 0;
			dy = -1;
		}
		else if(moveChar == 's') {
			dx = 0;
			dy = 1;
		}
		*/
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
	}
	close(fileRef);
	return 0;
}

