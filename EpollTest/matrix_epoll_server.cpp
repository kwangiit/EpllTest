/*
 * matrix_epoll_server.cpp
 *
 *  Created on: Feb 18, 2014
 *      Author: kwang
 */
#include "matrix_epoll_server.h"

#include <stdlib.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "net.h"

using namespace std;

MatrixEventData::MatrixEventData(int fd,
		const char* buf, size_t bufsize, sockaddr addr)
{
	_fd = fd;
	int len = strlen((const char*) buf);
	_buf = (char*) calloc(len + 1, sizeof(char));
	memcpy(_buf, buf, len + 1);

	_bufsize = bufsize;
	_fromaddr = addr;
}

MatrixEventData::~MatrixEventData()
{

}

int MatrixEventData::fd() const
{
	return _fd;
}

char* MatrixEventData::buf() const
{
	return _buf;
}

size_t MatrixEventData::bufsize() const
{
	return _bufsize;
}

sockaddr MatrixEventData::fromaddr()
{
	return _fromaddr;
}

MatrixEpollData::MatrixEpollData(const int& fd, const sockaddr
		*const sender) : _fd(fd), _sender(sender)
{

}

MatrixEpollData::~MatrixEpollData()
{
	if (_sender != NULL)
	{
		free((void*) _sender);
		_sender = NULL;
	}
}

int MatrixEpollData::fd() const
{
	return _fd;
}

const sockaddr* const MatrixEpollData::sender() const
{
	return _sender;
}


const int MatrixEpollServer::MAX_EVENTS = 4096;

MatrixEpollServer::MatrixEpollServer(char *port,
		char *type) : _port(port), _type(type), _eventQueue()
{
	printf("OK, I have created a epoll server, "
			"the port is:%s, and type is:%s\n", _port, _type);
}

MatrixEpollServer::~MatrixEpollServer()
{

}


int MatrixEpollServer::create_and_bind(const char *port)
{

	return create_and_bind(NULL, port);
}

int MatrixEpollServer::create_and_bind(const char *host, const char *port)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s, sfd;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC; /* Return IPv4 and IPv6 choices */
	hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
	hints.ai_flags = AI_PASSIVE; /* All interfaces */

	s = getaddrinfo(host, port, &hints, &result);
	if (s != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;

		s = bind(sfd, rp->ai_addr, rp->ai_addrlen);
		if (s == 0)
		{
			/* We managed to bind successfully! */
			break;
		}

		close(sfd);
	}

	if (rp == NULL)
	{
		fprintf(stderr, "Could not bind\n");
		return -1;
	}

	freeaddrinfo(result);

	return sfd;
}

int MatrixEpollServer::make_socket_non_blocking(const int& sfd)
{
	int flags, s;

	flags = fcntl(sfd, F_GETFL, 0);
	if (flags == -1)
	{
		perror("fcntl");
		return -1;
	}

	flags |= O_NONBLOCK;
	s = fcntl(sfd, F_SETFL, flags);
	if (s == -1)
	{
		perror("fcntl");
		return -1;
	}

	return 0;
}

int MatrixEpollServer::make_svr_socket()
{
	int port = atoi(_port);
	printf("The port number is:%d\n", port);
	struct sockaddr_in svrAdd_in; /* socket info about our server */
	int svrSock = -1;

	try
	{
		memset(&svrAdd_in, 0, sizeof(struct sockaddr_in)); /* zero the struct before filling the fields */
		svrAdd_in.sin_family = AF_INET; /* set the type of connection to TCP/IP */
		svrAdd_in.sin_addr.s_addr = INADDR_ANY; /* set our address to any interface */
		svrAdd_in.sin_port = htons(port); /* set the server port number */

		if (strcmp(_type, "TCP") == 0) //make socket
		{
			svrSock = socket(AF_INET, SOCK_STREAM, 0); /* OS will return a fd for network stream connection*/
			printf("OK, this is TCP, the socket number is:%d\n", svrSock);
		}
		else //UDP
		{
			svrSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		}

		if (svrSock < 0)
		{
			printf("Error occurred when creating the socket:"
					"%d to the server port:%d\n", svrSock, port);
			printf("%s\n", strerror(errno));

			close(svrSock);
			return -1;
		}

		printf("start to bind!\n");
		if (bind(svrSock, (struct sockaddr*) &svrAdd_in,
				sizeof(struct sockaddr)) < 0)
		{
			printf("Error occurred binding the socket:%d "
					"to the server port:%d\n", svrSock, port);
			printf("%s", strerror(errno));

			close(svrSock);
			return -1;
		}

		if (strcmp(_type, "TCP") == 0) //TCP needs listen, UDP does not.
		{
			/* start listening, allowing a queue of up to 1 pending connection */
			if (listen(svrSock, SOMAXCONN) < 0)
			{
				printf("Error occurred while enabling listen "
						"on the socket:%d\n", svrSock);
				printf("%s", strerror(errno));

				close(svrSock);
				return -1;
			}
		}
	}
	catch (exception &e)
	{
		fprintf(stderr, "exception caught:\n\t%s", e.what());
	}

	return svrSock;
}

int MatrixEpollServer::reuse_sock(int sock)
{
	int reuse_addr = 1;
	int ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
			&reuse_addr, sizeof(reuse_addr));
	if (ret < 0)
	{
		//cout << "resuse socket failed: " << strerror(errno) << endl;
		return -1;
	}
	else
		return 0;
}

/*int MatrixEpollServer::serve_request(int client_sock,
		void *buff, sockaddr fromAddr)
{
	Package pkg;
	pkg.ParseFromArray(buff, _BUF_SIZE);

	string msg = pkg.virtualpath();
	if (msg.)
	return -1;
}*/

int MatrixEpollServer::serve_request(int sock, void *buf, sockaddr from)
{
	printf("I received a message:%s\n", buf);
	char retBuf[30];
	memset(retBuf, '\0', 30);
	strcpy(retBuf, "OK, I got you!");

	for (int i = 0; i < 10; i++)
	{
		send_bf(sock, retBuf);
	}
	return 0;
}

void* MatrixEpollServer::threaded_serve(void *arg)
{
	MatrixEpollServer *mes = (MatrixEpollServer*) arg;

	while (true)
	{
		while (!mes->_eventQueue.empty())
		{
			MatrixEventData eventData = mes->_eventQueue.front();

			/*pes->_ZProcessor->process(eventData.fd(), eventData.buf(),
					eventData.fromaddr()); replace this part with matrix logic */
			mes->serve_request(eventData.fd(), eventData.buf(), eventData.fromaddr());
			mes->_eventQueue.pop();

		}
	}

	return NULL;
}

void MatrixEpollServer::init_thread()
{
	pthread_t thread;
	while (pthread_create(&thread, NULL, threaded_serve, this) != 0)
	{
		sleep(1);
	}
}

void MatrixEpollServer::serve()
{
	init_thread();

	int sfd, s;
	int efd;
	struct epoll_event event;
	struct epoll_event *events;

	sfd = make_svr_socket();
	if (sfd == -1)
		abort();

	s = make_socket_non_blocking(sfd);
	if (s == -1)
		abort();

	reuse_sock(sfd);

	efd = epoll_create(1);
	if (efd == -1)
	{
		perror("epoll_create");
		abort();
	}

	event.data.ptr = new MatrixEpollData(sfd, NULL);
	event.events = EPOLLIN | EPOLLET;
	s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);
	if (s == -1)
	{
		perror("epoll_ctl");
		abort();
	}

	/* Buffer where events are returned */
	events = (epoll_event *) calloc(MAX_EVENTS, sizeof event);

	printf("Now, I am waiting for event!\n");
	/* The event loop */
	while (1)
	{
		int n, i;

		n = epoll_wait(efd, events, MAX_EVENTS, -1);

		for (i = 0; i < n; i++)
		{
			MatrixEpollData *edata = (MatrixEpollData*) events[i].data.ptr;

			if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)
					|| (!(events[i].events & EPOLLIN)))
			{
				/* An error has occured on this fd, or the socket is not
				 ready for reading (why were we notified then?) */
				fprintf(stderr, "epoll error\n");
				close(edata->fd());
				delete edata;
				continue;
			}
			else if (sfd == edata->fd())
			{
				if (strcmp(_type, "TCP") == 0)
				{
					/* We have a notification on the listening socket, which
					 means one or more incoming connections. */
					while (1)
					{
						sockaddr *in_addr = (sockaddr *) calloc(1,
								sizeof(struct sockaddr));
						socklen_t in_len = sizeof(struct sockaddr);

						int infd = accept(sfd, in_addr, &in_len);
						if (infd == -1)
						{
							free(in_addr);

							if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
							{
								/* We have processed all incoming connections. */
								break;
							}
							else
							{
								perror("accept");
								break;
							}
						}

						/* Make the incoming socket non-blocking and add it to the
						 list of fds to monitor. */
						s = make_socket_non_blocking(infd);
						if (s == -1)
						{
							free(in_addr);
							abort();
						}

						reuse_sock(infd);

						event.data.ptr = new MatrixEpollData(infd, in_addr);
						event.events = EPOLLIN | EPOLLET;
						s = epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event);
						if (s == -1)
						{
							free(in_addr);
							perror("epoll_ctl");
							abort();
						}
					}
					continue;
				}
				else
				{
					int done = 0;

					while (1)
					{
						char buf[600];
						memset(buf, 0, sizeof(buf));

						sockaddr fromaddr;
						socklen_t sender_len = sizeof(struct sockaddr);
						ssize_t count = recvfrom(edata->fd(), buf, sizeof buf,
								0, &fromaddr, &sender_len);

						if (count == -1)
						{
							if (errno != EAGAIN)
							{
								perror("read");
								done = 1;
							}

						}
						else if (count == 0)
						{
							done = 1;
							break;
						}
						else
						{
//#ifdef BIG_MSG
//							bool ready = false;
//							string bd = pbrb->getBdStr(sfd, buf, count, ready);
//
//							if (ready) {
//
//#ifdef THREADED_SERVE
//								EventData eventData(edata->fd(), bd.c_str(), bd.size(),
//										fromaddr);
//								_eventQueue.push(eventData);
//
//#else
//								_ZProcessor->process(edata->fd(), bd.c_str(),
//										fromaddr);
//#endif
//							}
//#endif
//
//#ifdef SML_MSG
//#ifdef THREADED_SERVE
							MatrixEventData eventData(edata->fd(), buf, sizeof(buf),
									fromaddr);
							_eventQueue.push(eventData);

//#else
//							string bufstr(buf);
//							_ZProcessor->process(edata->fd(), bufstr.c_str(),
//									fromaddr);
//#endif
//#endif
						}
					}
				}

			}
			else
			{
				if (strcmp(_type, "TCP") == 0)
				{
					/* We have data on the fd waiting to be read. Read and
					 display it. We must read whatever data is available
					 completely, as we are running in edge-triggered mode
					 and won't get a notification again for the same
					 data. */
					int done = 0;

					while (1)
					{
						char buf[600];
						memset(buf, 0, sizeof(buf));

						ssize_t count = recv(edata->fd(), buf, sizeof(buf), 0);

						if (count == -1)
						{
							/* If errno == EAGAIN, that means we have read all
							 data. So go back to the main loop. */
							if (errno != EAGAIN)
							{
								perror("read");
								done = 1;
							}
							break;
						}
						else if (count == 0)
						{
							/* End of file. The remote has closed the
							 connection. */
							done = 1;
							break;
						}
						else
						{
//#ifdef BIG_MSG
//							bool ready = false;
//							string bd = pbrb->getBdStr(sfd, buf, count, ready);
//
//							if (ready) {
//
//#ifdef THREADED_SERVE
//								EventData eventData(edata->fd(), bd.c_str(), bd.size(),
//										*edata->sender());
//								_eventQueue.push(eventData);
//#else
//								_ZProcessor->process(edata->fd(), bd.c_str(),
//										*edata->sender());
//#endif
//							}
//#endif
//
//#ifdef SML_MSG
//#ifdef THREADED_SERVE
							MatrixEventData eventData(edata->fd(), buf, sizeof(buf),
									*edata->sender());
							_eventQueue.push(eventData);
//#else
//							string bufstr(buf);
//							_ZProcessor->process(edata->fd(), bufstr.c_str(),
//									*edata->sender());
//#endif
//#endif
						}
					}

					if (done)
					{
						/* Closing the descriptor will make epoll remove it
						 from the set of descriptors which are monitored. */
						close(edata->fd());
						delete edata;
					}
				}
			}
		}
	}

	free(events);

	close(sfd);

	MatrixEpollData *edata = (MatrixEpollData*) event.data.ptr;
	delete edata;
}
