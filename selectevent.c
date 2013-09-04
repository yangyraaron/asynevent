/*
 * selectevent.c
 *
 *  Created on: Sep 3, 2013
 *      Author: wisedulab2
 */

#include <sys/select.h>
#include <stdlib.h>

#include "eventloop.h"

typedef struct eventState{
	fd_set rfds,wfds;
	//after select function used,the fd sets my have changed,so copy these fd sets
	fd_set _rfds,_wfds;
} eventState;

static int createEventState(eventLoop *eventLoop){
	eventState *state = malloc(sizeof(eventState));

	if(!state) return -1;

	FD_ZERO(&state->rfds);
	FD_ZERO(&state->wfds);

	eventLoop->state = state;

	return 0;
}

static void freeEventState(eventLoop *eventLoop){
	if(eventLoop == NULL) return;

	free(eventLoop->state);
}

static int canResize(eventLoop* eventLoop,int setsize){
	if(setsize>FD_SETSIZE) return -1;
	return 0;
}

static int addEvent(eventLoop *eventLoop,int fd,int mask){
	eventState *state = eventLoop->state;

	if(mask & EVENT_READABLE) FD_SET(fd,&state->rfds);
	if(mask & EVENT_WRITABLE) FD_SET(fd,&state->wfds);

	return 0;
}

static void delEvent(eventLoop *eventLoop, int fd,int mask){
	eventState *state = eventLoop->state;

	if(mask & EVENT_READABLE) FD_CLR(fd,&state->rfds);
	if(mask & EVENT_WRITABLE) FD_CLR(fd,&state->wfds);
}

static int poll(eventLoop * eventLoop,struct timeval *tvp){
	eventState * state = eventLoop->state;
	int retval,j,numevents=0;

	memcpy(&state->_rfds,&state->rfds,sizeof(fd_set));
	memcpy(&state->_wfds,&state->wfds,sizeof(fd_set));

	retval = select(eventLoop->maxfd+1,&state->_rfds,&state->_wfds,NULL,tvp);

	if(retval>0){
		for(j=0;j<=eventLoop->maxfd;++j){
			int mask = 0;
			eventEntry *entry = &eventLoop->events[j];

			if(entry->mask == EVENT_NONE) continue;
			if((entry->mask & EVENT_READABLE) && FD_ISSET(j,&state->_rfds))
				mask |= EVENT_READABLE;
			if((entry->mask & EVENT_WRITABLE) && FD_ISSET(j,&state->_wfds))
				mask |= EVENT_WRITABLE;

			eventLoop->fired[numevents].fd=j;
			eventLoop->fired[numevents].mask = mask;

			++numevents;
		}
	}
	return numevents;
}
