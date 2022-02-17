#include <climits>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <thread>

#include <getopt.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>

#include "pthread.h"

std::vector<uint64_t> resultMylti = std::vector<uint64_t>();
std::mutex mv;

void* Multiplication(std::size_t i, uint64_t begin, uint64_t end ,uint64_t buf, uint64_t Remainder, uint64_t Mod , uint64_t Pnum){
    int Intermediate_result = 1;
    if (Remainder == 0) {
        if (i != Pnum) {
            for (int j = begin + ((i - 1) * buf); j < begin + i * buf; ++j) {
                Intermediate_result = Intermediate_result * j;
                Intermediate_result = Intermediate_result % Mod;
            }
        }
        else{
            for (int j = begin + ((i - 1) * buf); j < begin + i * buf + 1; ++j) {
                Intermediate_result = Intermediate_result * j;
                Intermediate_result = Intermediate_result % Mod;
            }
        }
    }
    else {
        if (i != Pnum)
            for (int j = begin + ((i - 1) * buf); j < begin + i * buf; ++j) {
                Intermediate_result = Intermediate_result * j;
                Intermediate_result = Intermediate_result % Mod;
            }
        else if (i == Pnum) {
            for (int j = begin + ((i - 1) * buf); j <= end; ++j) {
                Intermediate_result = Intermediate_result * j;
                Intermediate_result = Intermediate_result % Mod;
            }
        }
    }
    mv.lock();
    resultMylti.push_back(Intermediate_result);
    mv.unlock();
    return nullptr;
}

int main(int argc, char **argv) {
    int tnum = -1;
    int port = -1;

    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {{"port", required_argument, 0, 0},
                                          {"tnum", required_argument, 0, 0},
                                          {0, 0, 0, 0}};

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1)
            break;

        switch (c) {
            case 0: {
                switch (option_index) {
                    case 0:
                        port = std::stoi(optarg);
                        break;
                    case 1:
                        tnum = std::stoi(optarg);
                        break;
                    default:
                        printf("Index %d is out of options\n", option_index);
                }
            } break;

            case '?':
                printf("Unknown argument\n");
                break;
            default:
                fprintf(stderr, "getopt returned character code 0%o?\n", c);
        }
    }

    if (port == -1 || tnum == -1) {
        fprintf(stderr, "Using: %s --port 20001 --tnum 4\n", argv[0]);
        return 1;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        fprintf(stderr, "Can not create server socket!");
        return 1;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons((uint16_t)port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt_val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

    int err = bind(server_fd, (struct sockaddr *)&server, sizeof(server));
    if (err < 0) {
        fprintf(stderr, "Can not bind to socket!");
        return 1;
    }

    err = listen(server_fd, 128);
    if (err < 0) {
        fprintf(stderr, "Could not listen on socket\n");
        return 1;
    }

    printf("Server listening at %d\n", port);

    while (true) {
        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);
        int client_fd = accept(server_fd, (struct sockaddr *)&client, &client_len);

        if (client_fd < 0) {
            fprintf(stderr, "Could not establish new connection\n");
            continue;
        }

        while (true) {
            unsigned int buffer_size = sizeof(uint64_t) * 3;
            char from_client[buffer_size];
            int read = recv(client_fd, from_client, buffer_size, 0);

            if (!read)
                break;
            if (read < 0) {
                fprintf(stderr, "Client read failed\n");
                break;
            }
            if (read < buffer_size) {
                fprintf(stderr, "Client send wrong data format\n");
                break;
            }

            pthread_t threads[tnum];

            uint64_t begin = 0;
            uint64_t end = 0;
            uint64_t mod = 0;
            memcpy(&begin, from_client, sizeof(uint64_t));
            memcpy(&end, from_client + sizeof(uint64_t), sizeof(uint64_t));
            memcpy(&mod, from_client + 2 * sizeof(uint64_t), sizeof(uint64_t));

            fprintf(stdout, "Receive: %llu %llu %llu\n", begin, end, mod);

            std::vector<std::thread> threadVector = std::vector<std::thread>();

            int count_tnum = tnum;
            int remainder,buf =0;
            if (tnum >= end-begin) { tnum = end-begin; }
            if ((end-begin) % tnum == 0) {
                buf = (end-begin) / tnum;
                remainder = 0;
            }
            else {
                remainder = end-begin % tnum;
                buf = (end-begin - remainder) / tnum;
            }

            for (std::size_t i = 1; i < (tnum + 1); ++i) {
                threadVector.emplace_back(std::thread(&Multiplication, i, begin,end, buf, remainder, mod, tnum));
            }

            for (std::size_t i = 0; i < tnum; ++i) {
                threadVector[i].join();
            }

            uint64_t total = 1;
            for (std::size_t i = 0; i < tnum; ++i) {
                total *= resultMylti[i];
                total = total % mod;
            }

            printf("Total: %llu\n", total);

            char buffer[sizeof(total)];
            memcpy(buffer, &total, sizeof(total));
            err = send(client_fd, buffer, sizeof(total), 0);
            if (err < 0) {
                fprintf(stderr, "Can't send data to client\n");
                break;
            }
        }

        shutdown(client_fd, SHUT_RDWR);
        close(client_fd);
    }

    return 0;
}