/*
 * serverside.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: kwang
 */

#include "net.h"
#include <string>
#include "matrix_epoll_server.h"

int main(int argc, char* argv[])
{
	char *type = "TCP";
	char *portChar = "60000";

	pthread_t esThread = create_es_thread(portChar, type);
	pthread_join(esThread, NULL);
}


