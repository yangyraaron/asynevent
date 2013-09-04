/*
 * server.c
 *
 *  Created on: Sep 4, 2013
 *      Author: wisedulab2
 */
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <signal.h>

#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "eventloop.h"

eventLoop *ep;
int listener;

void readFromClient(eventLoop *,int ,void *,int );
void replyToClient(eventLoop *,int ,void *,int );

static void on_server_close(){
	printf("a  terminal signal is received \n");

	delEventLoop(ep);
	close(listener);
}

static void make_non_blocking(int fd){
	fcntl(fd,F_SETFL,O_NONBLOCK);
}

void readFromClient(eventLoop *loop,int fd,void *data,int mask){
	char buf[1024];
	//int i;
	//ssize_t result;

	read(fd,buf,sizeof(buf));

	printf("received data : %s\n",buf);

	createEventEntry(ep,fd,EVENT_WRITABLE,replyToClient,NULL);
}

void replyToClient(eventLoop *loop,int fd,void *data,int mask){
	char *buf = "ok";
	write(fd,buf,strlen(buf));

	printf("replied \n");

	delEventEntry(loop,fd,EVENT_READABLE);
	delEventEntry(loop,fd,EVENT_WRITABLE);
	close(fd);
}

void acceptClient(eventLoop *loop,int fd,void *data,int mask){
	struct sockaddr_storage ss;
	socklen_t slen = sizeof(ss);

	int clifd = accept(fd,(struct sockaddr *)&ss,&slen);
	if(clifd<0){
		perror("server accept");
		return;
	}

	make_non_blocking(clifd);
	createEventEntry(ep,clifd,EVENT_READABLE,readFromClient,NULL);
}

int server_run(){
	ep = createEventLoop(1024);

	struct sockaddr_in sin;

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = 0;
	sin.sin_port = htons(40713);

	listener = socket(AF_INET,SOCK_STREAM,0);
	make_non_blocking(listener);

	if(bind(listener,(struct sockaddr *)&sin,sizeof(sin))<0){
		perror("bind server");
		return 1;
	}
	if(listen(listener,16)){
		perror("server listening");
		return 1;
	}

	struct sigaction sa;
	memset(&sa,0,sizeof(sa));
	sa.sa_handler = on_server_close;
	sigaction(SIGTERM,&sa,NULL);

	createEventEntry(ep,listener,EVENT_READABLE,acceptClient,NULL);

	printf("server is running...\n");

	runEventLoop(ep);

	return 0;

}


int main(int argc,char *argv[]){
	server_run();

	return 0;
}
