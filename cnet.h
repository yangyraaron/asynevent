/*
 * cnet.h
 *
 *  Created on: Sep 4, 2013
 *      Author: wisedulab2
 */

#ifndef CNET_H_
#define CNET_H_

#define NET_ERR -1
#define NET_OK 0

int netTcpServer(char *host,int port);
int netMakeNonBlock(int fd);

#endif /* CNET_H_ */
