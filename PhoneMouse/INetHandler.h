/*
 * INetHandler.h
 *
 *  Created on: Jul 31, 2015
 *      Author: zena-laptop
 */

#ifndef INETHANDLER_H_
#define INETHANDLER_H_

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <bits/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define TYPE_WIFI 0
#define TYPE_LAN 1
#define TYPE_REL_X 0
#define TYPE_REL_Y 1
#define TYPE_KEY 2

int INET_DATA_READY_FLAG;
//struct for passing the data..
struct INetDatagram {
	int type;
	int value;
};
void *INetReceiveData(void *buffer);
int INetServerThreadStart(char *buffer);
int INetConnect(int desiredType, char *buffer);
void INetDisconnect();
char* INetGetIp();
int INetSendInt(int value);
int INetSendString(char* string);
#endif /* INETHANDLER_H_ */
