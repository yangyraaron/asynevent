/*
 * epollevent.c
 *
 *  Created on: Sep 5, 2013
 *      Author: wisedulab2
 */


#include <sys/epoll.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "eventloop.h"

#define MAX_EVENTS 10

typedef struct epollState{
	int epollfd;
	struct epoll_event *events;

} epollState;

static int createEventState(eventLoop *eventLoop){
	epollState *state = malloc(sizeof(state));

	if(!state) return -1;

	state->events = malloc(sizeof(struct epoll_event)*eventLoop->setsize);
	if(!state->events){
		free(state);
		return -1;
	}

	state->epollfd = epoll_create(1024);
	if(state->epollfd==-1){
		free(state->events);
		free(state);
		return 01;
	}

	eventLoop->state = state;

	return 0;
}

static void freeEventState(eventLoop *eventLoop){
	epollState *state = (epollState *)eventLoop->state;

	close(state->epollfd);
	free(state->events);
	free(eventLoop->state);
}

static int Resize(eventLoop *eventLoop,int setsize){
	epollState *state = (epollState *)eventLoop->state;
	state->events = malloc(sizeof(struct epoll_event)*setsize);

	return 0;
}

static int addEvent(eventLoop *eventLoop,int fd,int mask){
	epollState *state = (epollState *)eventLoop->state;
	struct epoll_event ev;

	int op = eventLoop->events[fd].mask==EVENT_NONE?EPOLL_CTL_ADD:EPOLL_CTL_MOD;

	ev.events = 0;
	ev.data.fd = fd;
	mask |= eventLoop->events[fd].mask;//merge old events
	if(mask & EVENT_READABLE) ev.events |= EPOLLIN;
	if(mask & EVENT_WRITABLE) ev.events |= EPOLLOUT;

	if(epoll_ctl(state->epollfd,op,fd,&ev)==-1){
		perror("epoll_ctl add event");

		return -1;
	}

	return 0;
}

static void delEvent(eventLoop *eventLoop,int fd,int delmask){
	epollState *state = (epollState *)eventLoop->state;
	struct epoll_event ev;
	//remove the del event mask
	int mask = eventLoop->events[fd].mask & (~delmask);

	ev.events=0;
	ev.data.fd = fd;
	if(mask & EVENT_READABLE) ev.events |= EPOLLIN;
	if(mask & EVENT_WRITABLE) ev.events |= EPOLLOUT;

	//if fd still has events
	if(mask!=EVENT_NONE){
		if(epoll_ctl(state->epollfd,EPOLL_CTL_MOD,fd,&ev)==-1){
			perror("epoll_ctl mod event");
		}
	}else if(epoll_ctl(state->epollfd,EPOLL_CTL_DEL,fd,&ev)==-1){
		perror("epoll_ctl del event");
	}

}

static int poll(eventLoop *eventLoop,struct timeval *tvp){
	epollState *state = (epollState *)eventLoop->state;
	int nfds,numevents=0;

	if((nfds=epoll_wait(state->epollfd,state->events,eventLoop->setsize,
			tvp ? (tvp->tv_sec*1000 + tvp->tv_usec/1000):-1))==-1){
		perror("epoll_wait");

		return -1;
	}

	if(nfds>0){
		int n;
		numevents = nfds;

		for(n=0;n<nfds;++n){
			int mask=0;
			struct epoll_event *ev = state->events+n;

			if(ev->events & EPOLLIN) mask |= EVENT_READABLE;
			if(ev->events & EPOLLOUT) mask |= EVENT_WRITABLE;
			if(ev->events & EPOLLERR) mask |= EVENT_WRITABLE;
			if(ev->events & EPOLLHUP) mask |= EVENT_WRITABLE;

			eventLoop->fired[n].fd = ev->data.fd;
			eventLoop->fired[n].mask = mask;
		}
	}

	return numevents;
}
