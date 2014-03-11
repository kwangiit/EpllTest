/*
 * net.h
 *
 *  Created on: Feb 27, 2014
 *      Author: kwang
 */

#ifndef NET_H_
#define NET_H_

#include <stropts.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/netdevice.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include "matrix_epoll_server.h"
#include <stdlib.h>
#include <stdio.h>

using namespace std;

extern int send_first(char*, long, char*);
extern int send_bf(int, char*);
extern int recv_bf(int, char*);
extern pthread_t create_es_thread(char*, char*);


#endif /* NET_H_ */
