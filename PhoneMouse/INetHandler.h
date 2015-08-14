/*
 * INetHandler.h
 *
 *  Created on: Jul 31, 2015
 *      Author: zena-laptop
 */

#ifndef INETHANDLER_H_
#define INETHANDLER_H_

#define TYPE_WIFI 0
#define TYPE_LAN 1
#define TYPE_REL_X 0
#define TYPE_REL_Y 1
#define TYPE_KEY 2
//int used to check for data...
int DATA_READY;
//struct for passing the data..
struct INetDatagram {
	int type;
	int value;
};
void *INetReceiveData(void *buffer);
int INetServerThreadStart(char *buffer);
int INetConnect(int desiredType);
void INetDisconnect();
char* INetGetIp();
int INetSendInt(int value);
int INetSendString(char* string);
#endif /* INETHANDLER_H_ */
