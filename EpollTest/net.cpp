/*
 * net.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: kwang
 */
#include "net.h"
#include <pthread.h>
#include <string.h>

int send_first(char *ip, long port, char *buf)
{
	int to_sock;
	struct sockaddr_in dest;
	memset(&dest, 0, sizeof(struct sockaddr_in)); /*zero the struct*/
	struct hostent * hinfo = gethostbyname(ip);
	printf("OK, I am good\n");
	if (hinfo == NULL)
	{
		printf("getbyname failed!\n");
		return -1;
	}

	dest.sin_family = PF_INET; /*storing the server info in sockaddr_in structure*/
	dest.sin_addr = *(struct in_addr *) (hinfo->h_addr); /*set destination IP number*/
	dest.sin_port = htons(port);
	printf("Before creating a socket!\n");
	to_sock = socket(PF_INET, SOCK_STREAM, 0); //try change here.................................................
	printf("The socket number is:%d\n", to_sock);
	if (to_sock < 0)
	{
		return -1;
	}

	printf("Now, I am going to connect to a server!\n");
	int ret_con = connect(to_sock, (struct sockaddr *) &dest,
			sizeof(sockaddr));
	if (ret_con < 0)
	{
		return -1;
	}

	printf("Great, I have already built a socket!\n");
	send_bf(to_sock, buf);

	return to_sock;
}

int send_bf(int sock, char *buf)
{
	int bufSize = strlen(buf);
	return send(sock, (const void*)buf, bufSize, 0);
}

int recv_bf(int sock, char *buf)
{
	return recv(sock, buf, 30, 0);
}

void* es_proc(void *arg)
{
	MatrixEpollServer *ms = (MatrixEpollServer*)arg;
	ms->serve();
	pthread_exit(NULL);
	return NULL;
}

pthread_t create_es_thread(char *port, char *type)
{
	printf("Now, I am creating a epoll server!\n");
	pthread_t esThread;
	MatrixEpollServer *ms = new MatrixEpollServer(port, type);

	while (pthread_create(&esThread, NULL, es_proc, (void*)ms) != 0)
	{
		sleep(1);
	}

	return esThread;
}
