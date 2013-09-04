/*
 * eventloop.h
 *
 *  Created on: Sep 3, 2013
 *      Author: wisedulab2
 */

#ifndef EVENTLOOP_H_
#define EVENTLOOP_H_

#define EVENT_ERR -1
#define EVENT_OK 0

#define EVENT_NONE 0
#define EVENT_READABLE 1
#define EVENT_WRITABLE 2

struct eventLoop;

typedef void fileProc(struct eventLoop *eventLoop,int fd, void *data,int mask);
typedef void beforesleepproc(struct eventLoop *eventLoop);

typedef struct eventEntry{
	int mask;
	fileProc *readProc;
	fileProc *writeProc;
	void *data;
} eventEntry;

typedef struct firedEventEtnry{
	int fd;
	int mask;
} firedEventEntry;

typedef struct eventLoop{
	int maxfd;
	int setsize;
	eventEntry *events;
	firedEventEntry *fired;
	void *state;
	int stop;
	beforesleepproc *beforesleep;
} eventLoop;

eventLoop *createEventLoop(int setsize);
void delEventLoop(eventLoop *eventLoop);
int resizeEventLoop(eventLoop *eventLoop,int setsize);
int createEventEntry(eventLoop *eventLoop,int fd,int mask,fileProc *proc,void *data);
void delEventEntry(eventLoop *eventLoop,int fd,int mask);
int getEventEntry(eventLoop *eventLoop,int fd);
int processEvents(eventLoop *eventLoop);
void runEventLoop(eventLoop *eventLoop);

#endif /* EVENTLOOP_H_ */
