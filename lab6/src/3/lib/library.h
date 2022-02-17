#ifndef LYBRARYFACTORIAL_LIBRARY_H
#define LYBRARYFACTORIAL_LIBRARY_H

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <utility>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

std::pair<sockaddr_in,int> GetConnectToServer(u_short  port, unsigned long addr);
void SendMessageToServer(std::pair<sockaddr_in,int> pairConnect,char message[],int size);
uint64_t GetMessageToServer(std::pair<sockaddr_in,int> pairConnect);

#endif //LYBRARYFACTORIAL_LIBRARY_H
