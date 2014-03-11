/*
 * client.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: kwang
 */

#include "net.h"
#include <string>
#include "matrix_epoll_server.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

using namespace std;

int main(int argc, char* argv[])
{
	char *type = "TCP";
	char *portChar = "50000";

	pthread_t esThread = create_es_thread(portChar, type);

	char bufSend[30];
	memset(bufSend, '\0', 30);
	strcpy(bufSend, "I am a student from IIT");

	int sock = send_first(argv[1], 60000, bufSend);

	printf("The socket created is:%d\n", sock);
	char bufRecv[30];
	memset(bufRecv, '\0', 30);

	for (int i = 0; i < 10; i++)
	{
		recv_bf(sock, bufRecv);
		printf("The message received is:%s\n", bufRecv);
		memset(bufRecv, '\0', 30);
	}

	pthread_join(esThread, NULL);
}



