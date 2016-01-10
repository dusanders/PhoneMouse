/*
 * INetHandler.c
 *
 *  Created on: Jul 31, 2015
 *      Author: zena-laptop
 */
#include "INetHandler.h"


pthread_t receiveThread;
int inetSocket;
struct ifreq interfaceReq;
struct sockaddr_in socketAddr;
struct sockaddr_in clientAddr;
char *ipStringWifi;
char *ipStringLan;

void *INetReceiveData(void *buffer) {
	printf("Receiving data....\n");
	int recd;
	CLOSE = 0;
	INET_DATA_READY_FLAG = 0;
	socklen_t fromLen = sizeof(clientAddr);
	//enter endless loop while waiting for client...
	while(!CLOSE) {
		//receive the data from client
		recd = recvfrom(inetSocket, buffer, sizeof(buffer),
				MSG_DONTWAIT, (struct sockaddr *)&clientAddr, &fromLen);

		if(recd > 0) {
			//set the data flag indicating we are ready for consumption.
			INET_DATA_READY_FLAG = 1;
			recd = 0;
			while(INET_DATA_READY_FLAG) {
				/*	for thread safety, we must wait for consumer to
				 * consume the buffer...
				 */
			}
		}
	}
	return 0;
}

int INetServerThreadStart(char receiveBuffer[]) {
	printf("Begin opening server thread....\n");
	//default pthread_attr_t is 'detached' which is what
	//	we need.
	pthread_attr_t attr;
	printf("Init pthread_attr....\n");
	//initialize the pthread_attr...;
	pthread_attr_init(&attr);
	printf("Calling for pthread_create().....\n");
	//create the thread, checking for errors and exiting
	//	if any returns.
	if(pthread_create(&receiveThread, &attr,
				&INetReceiveData, receiveBuffer) != 0) {
		printf("Error creating server thread!\n");
		return -1;
	}
	printf("Receive Thread started....\n");
	return 1;
}


int INetConnect(int desiredType, char buffer[]) {
	//open a file descriptor from the System with a call to
	// to 'socket' AF_INET = internet, SOCK_DGRAM = UDP,
	//	0 = System choose protocol (should default to UDP)...
	inetSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if(inetSocket == -1 ) {
		printf("Error creating Socket! Exiting...\n");
		return -1;
	}
	printf("Opened socket...\n");
	//set the values for the sockaddr_in struct required by the
	//	System.
	memset((char *)&socketAddr, 0 ,sizeof(socketAddr));
	socketAddr.sin_family = AF_INET;
	//socketAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	printf("Finished setup sockAddr...\n");
	//choose which IP to display....
	if(desiredType == TYPE_WIFI) {
		printf("Sending for TYPE_WIFI...\n");
		interfaceReq.ifr_ifru.ifru_addr.sa_family = AF_INET;
		strncpy(interfaceReq.ifr_ifrn.ifrn_name, "wlan0", IFNAMSIZ-1);
		ioctl(inetSocket, SIOCGIFADDR, &interfaceReq);
		ipStringWifi = inet_ntoa(((struct sockaddr_in *)
				&interfaceReq.ifr_ifru.ifru_addr)->sin_addr);
		if(inet_aton(ipStringWifi, &socketAddr.sin_addr) == 0) {
			printf("invalid IP");
			return -1;
		}

	}
	else if(desiredType == TYPE_LAN) {
		printf("Sending for TYPE_LAN....\n");
		interfaceReq.ifr_ifru.ifru_addr.sa_family = AF_INET;
		strncpy(interfaceReq.ifr_ifrn.ifrn_name, "eth0", IFNAMSIZ-1);
		ioctl(inetSocket, SIOCGIFADDR, &interfaceReq);
		ipStringLan = inet_ntoa(((struct sockaddr_in *)
				&interfaceReq.ifr_ifru.ifru_addr)->sin_addr);
		inet_aton(ipStringLan, &socketAddr.sin_addr);
	}
	socketAddr.sin_port = htons(6666);
	printf("Finished finding IP...\n");
	printf("Going to bind Socket and sockAddr...\n");
	//bind the socket with the file descriptor. Check this for errors.
	if(bind(inetSocket, (struct sockaddr *)&socketAddr, sizeof(socketAddr)) < 0) {
		printf("Error binding socket and address\n");
	}
	printf("%s\n", inet_ntoa((struct in_addr)socketAddr.sin_addr));
	printf("%d\n", socketAddr.sin_port);
	printf("Finished binding...\n");
	//open a buffer to pass to the server thread for receiving data.
	printf("Calling to open sever thread.....\n");
	//call our method to open a new thread.
	INetServerThreadStart(buffer);
	return 1;
}
void INetDisconnect() {
	CLOSE = 1;
	pthread_cancel(receiveThread);
	close(inetSocket);
}
char* INetGetIp(int desiredType) {
	if(desiredType == TYPE_WIFI) {
		return ipStringWifi;
	}
	else {
		return ipStringLan;
	}
}
int INetSend(char byteArray[]) {
	socklen_t fromLen = sizeof(clientAddr);
	printf("%d : %d", *byteArray, byteArray[0]);
	if(sendto(inetSocket, byteArray, sizeof(byteArray),
			0, (struct sockaddr *)&clientAddr, fromLen) < 0) {
		printf("Error sending to client!\n");
		return 0;
	}
	return 1;
}
