/*
 * eventloop.c
 *
 *  Created on: Sep 3, 2013
 *      Author: wisedulab2
 */

#include <stdlib.h>
#include <errno.h>

#include "config.h"
#include "eventloop.h"

#ifdef HAVE_EPOLL
#include "epollevent.c"
#else
#include "selectevent.c"
#endif

eventLoop *createEventLoop(int setsize){
	eventLoop *loop = malloc(sizeof(*loop));

	if(loop == NULL) goto err;

	loop->events = malloc(sizeof(eventEntry)*setsize);
	loop->fired = malloc(sizeof(firedEventEntry)*setsize);

	if(loop->events == NULL || loop->fired == NULL) goto err;

	loop->setsize = setsize;

	if(createEventState(loop)==-1) goto err;

	int i=0;
	//set every event's mask in event loop to 0
	for(;i<setsize;++i){
		loop->events[i].mask = EVENT_NONE;
	}

	return loop;

	err:
		if(loop){
			free(loop->events);
			free(loop->fired);
			free(loop);
		}

		return NULL;
}

int resizeEventLoop(eventLoop *eventLoop,int setsize){
	if(eventLoop->setsize == setsize) return EVENT_OK;
	if(eventLoop->maxfd>setsize) return EVENT_ERR;
	if(Resize(eventLoop,setsize)==-1) return EVENT_ERR;

	eventLoop->events = realloc(eventLoop->events,sizeof(eventEntry)*setsize);
	eventLoop->fired = realloc(eventLoop->events,sizeof(firedEventEntry)*setsize);
	eventLoop->setsize = setsize;

	int i;
	for(i=eventLoop->maxfd+1;i<setsize;++i)
		eventLoop->events[i].mask = EVENT_NONE;

	return EVENT_OK;
}

void delEventLoop(eventLoop *eventLoop){
	if(eventLoop == NULL) return;

	freeEventState(eventLoop);

	free(eventLoop->events);
	free(eventLoop->fired);

	eventLoop->events = NULL;
	eventLoop->fired =NULL;

	free(eventLoop);
}

int createEventEntry(eventLoop *eventLoop,int fd,int mask,fileProc *proc,void *data){
	if(eventLoop == NULL) return 0;

	//if the fd is overflow then error
	if(fd>eventLoop->setsize){
		errno = ERANGE;
		return EVENT_ERR;
	}

	eventEntry *entry = &eventLoop->events[fd];

	if(addEvent(eventLoop,fd,mask)==-1)
		return EVENT_ERR;

	entry->mask |= mask;
	if(mask & EVENT_READABLE) entry->readProc = proc;
	if(mask & EVENT_WRITABLE) entry->writeProc = proc;
	entry->data = data;

	if(fd>eventLoop->maxfd)
		eventLoop->maxfd = fd;

	return EVENT_OK;

}

void delEventEntry(eventLoop *eventLoop,int fd,int mask){
	if(fd>eventLoop->setsize) return;
	eventEntry *entry = &eventLoop->events[fd];

	if(entry->mask == EVENT_NONE) return;

	//set entry's mask to none
	entry->mask = entry->mask & (~entry->mask);

	if(fd == eventLoop->maxfd && entry->mask == EVENT_NONE){
		int j;

		for(j=eventLoop->maxfd-1;j>=0;--j)
			//find the next used event
			if(eventLoop->events[j].mask != EVENT_NONE) break;

		eventLoop->maxfd = j;
	}
	delEvent(eventLoop,fd,mask);
}

int getEventEntry(eventLoop *eventLoop,int fd){
	if(fd>=eventLoop->setsize) return 0;

	eventEntry *entry = &eventLoop->events[fd];

	return entry->mask;
}

int processEvents(eventLoop *eventLoop){
	int processed = 0,numevents;

	numevents = poll(eventLoop, NULL);
	int j;
	for(j=0;j<numevents;++j){
		eventEntry *entry = &eventLoop->events[eventLoop->fired[j].fd];
		int mask = eventLoop->fired[j].mask;
		int fd = eventLoop->fired[j].fd;
		int rfired = 0;

		if(entry->mask & mask & EVENT_READABLE){
			rfired = 1;
			entry->readProc(eventLoop,fd,entry->data,mask);
		}

		if(entry->mask & mask & EVENT_WRITABLE){
			if(!rfired || entry->readProc!=entry->writeProc)
				entry->writeProc(eventLoop,fd,entry->data,mask);
		}

		++ processed;
	}
	return processed;
}

void runEventLoop(eventLoop *eventLoop){
	eventLoop->stop = 0;
	while(!eventLoop->stop){
		if(eventLoop->beforesleep!=NULL)
			eventLoop->beforesleep(eventLoop);
		processEvents(eventLoop);
	}
}

