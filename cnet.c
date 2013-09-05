/*
 * cnet.c
 *
 *  Created on: Sep 4, 2013
 *      Author: wisedulab2
 */

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cnet.h"

static int netListen(int sd,struct sockaddr *addr,socklen_t slen){
	if(bind(sd,addr,slen) == -1){
		perror("bind");
		close(sd);

		return NET_ERR;
	}

	if(listen(sd,511) == -1){
		perror("listen");
		close(sd);

		return NET_ERR;
	}

	return NET_OK;

}

static int netV6only(int s){
	int yes=1;
	if(setsockopt(s,IPPROTO_IPV6,IPV6_V6ONLY,&yes,sizeof(yes)) == -1){
		perror("setsockopt to ipv6");
		close(s);

		return NET_ERR;
	}

	return NET_OK;
}

//set reuse the socket
static int netSetReuseAddr(int s){
	int yes=1;
	if(setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) == -1){
		perror("setsockopt to reuse addr");
		close(s);

		return NET_ERR;
	}

	return NET_OK;
}

int netTcpServer(char *host,int port){
	struct addrinfo hints, *servinfo,*p;
	int status,sd;
	char _port[6];

	snprintf(_port,6,"%d",port);

	memset(&hints,0,sizeof(hints));
	hints.ai_family= AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((status = getaddrinfo(host,_port,&hints,&servinfo))!=0){
		printf("getaddrinfo: %s\n",gai_strerror(status));

		return NET_ERR;
	}

	//find the first address could be listened
	for(p=servinfo;p!=NULL;p=p->ai_next){

		if((sd=socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1)
			continue;

		if(p->ai_family == AF_INET6 && netV6only(sd)==NET_ERR) goto err;
		if(netSetReuseAddr(sd)==NET_ERR) goto err;
		if(netListen(sd,p->ai_addr,p->ai_addrlen)==NET_ERR) goto err;

		goto end;
	}

	if(p==NULL){
		perror("unable to bind socket");
		goto err;
	}

	err:
		sd = NET_ERR;
	end:
		free(servinfo);
		return sd;

}

int netMakeNonBlock(int fd){
	int flags;

	//get original flags
	if((flags = fcntl(fd,F_GETFL))==-1){
		perror("fcntl get flags");

		return NET_ERR;
	}

	//set nonblock
	if(fcntl(fd,F_SETFL,flags | O_NONBLOCK)==-1){
		perror( "fcntl set nonblock");

		return NET_ERR;
	}

	return NET_OK;
}
