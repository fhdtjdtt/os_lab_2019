#include "library.h"

std::pair<sockaddr_in,int> GetConnectToServer(u_short  port, unsigned long addr)
{
    struct sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = port;
    server.sin_addr.s_addr = addr;

    int sck = socket(AF_INET, SOCK_STREAM, 0);
    if (sck < 0) {
        fprintf(stderr, "Socket creation failed!\n");
        exit(1);
    }

    if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
        fprintf(stderr, "Connection failed\n");
        exit(1);
    }
    return std::make_pair(server,sck);
}

void SendMessageToServer(std::pair<sockaddr_in,int> pairConnect,char message[],int size)
{
    if (send(pairConnect.second, message, size, 0) < 0) {
        fprintf(stderr, "Send failed\n");
        exit(1);
    }
}

uint64_t GetMessageToServer(std::pair<sockaddr_in,int> pairConnect)
{
    char buffer[sizeof(uint64_t)];
    if (recv(pairConnect.second, buffer, sizeof(buffer), 0) < 0) {
        fprintf(stderr, "Send failed\n");
        exit(1);
    }
    uint64_t answer = 0;
    memcpy(&answer, buffer, sizeof(uint64_t));
    return answer;
}