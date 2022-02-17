#include <cstdbool>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>
#include <fstream>
#include <thread>
#include <iostream>

#include <cerrno>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>


std::vector<uint64_t> vectorResultServer = std::vector<uint64_t>();

struct Server {
    std::string ip;
    int port;
    Server(std::string _ip,int _port)
    {
        ip = std::move(_ip);
        port = _port;
    }
};

struct Parametrs {
    uint64_t k;
    uint64_t mod;
    std::string pathtoserver;
    Parametrs()
    {
        k = -1;
        mod = -1;
        pathtoserver = "";
    }
};


void* sendToServer(const Server& addressStruct,const Parametrs& originalParametrs,const uint64_t begin,const uint64_t end)
{
    struct hostent *hostname = gethostbyname(addressStruct.ip.c_str());
    if (hostname == nullptr) {
        fprintf(stderr, "gethostbyname failed with %s\n", addressStruct.ip.c_str());
        exit(1);
    }

    struct sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(addressStruct.port);
    server.sin_addr.s_addr = *((unsigned long *)hostname->h_addr);

    int sck = socket(AF_INET, SOCK_STREAM, 0);
    if (sck < 0) {
        fprintf(stderr, "Socket creation failed!\n");
        exit(1);
    }

    if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
        fprintf(stderr, "Connection failed\n");
        exit(1);
    }

    char task[sizeof(uint64_t) * 3];
    memcpy(task, &begin, sizeof(uint64_t));
    memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &originalParametrs.mod, sizeof(uint64_t));

    if (send(sck, task, sizeof(task), 0) < 0) {
        fprintf(stderr, "Send failed\n");
        exit(1);
    }

    char buffer[sizeof(uint64_t)];
    if (recv(sck, buffer, sizeof(buffer), 0) < 0) {
        fprintf(stderr, "Send failed\n");
        exit(1);
    }
    uint64_t answer = 0;
    memcpy(&answer, buffer, sizeof(uint64_t));
    close(sck);
    vectorResultServer.push_back(answer);
    return nullptr;
}

std::vector<Server> parseFileWithAddressServer(const std::string& pathToFile)
{
    std::vector<Server> result;
    std::string buffer;
    std::ifstream file(pathToFile);
    while(getline(file, buffer)){ // пока не достигнут конец файла класть очередную строку в переменную (buffer)
        std::string ip = buffer.substr(0,buffer.find(':'));
        int port = std::stoi(buffer.substr(buffer.find(':')+1,-1));
        result.emplace_back(Server(ip,port));
    }
    return result;
}

bool ConvertStringToUI64(const char *str, uint64_t *val) {
    char *end = nullptr;
    unsigned long long i = strtoull(str, &end, 10);
    if (errno == ERANGE) {
        fprintf(stderr, "Out of uint64_t range: %s\n", str);
        return false;
    }

    if (errno != 0)
        return false;

    *val = i;
    return true;
}

int main(int argc, char **argv) {
    auto parameters = Parametrs();
    char servers[256] = {'\0'};

    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {{"k", required_argument, 0, 0},
                                          {"mod", required_argument, 0, 0},
                                          {"servers", required_argument, 0, 0},
                                          {0, 0, 0, 0}};

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1)
            break;

        switch (c) {
            case 0: {
                switch (option_index) {
                    case 0:
                        ConvertStringToUI64(optarg, &parameters.k);
                        break;
                    case 1:
                        ConvertStringToUI64(optarg, &parameters.mod);
                        break;
                    case 2:
                        strcpy(servers, optarg);
                        parameters.pathtoserver = servers;
                        break;
                    default:
                        printf("Index %d is out of options\n", option_index);
                }
            } break;

            case '?':
                printf("Arguments error\n");
                break;
            default:
                fprintf(stderr, "getopt returned character code 0%o?\n", c);
        }
    }

    if (parameters.k == -1 || parameters.mod == -1 || !strlen(servers)) {
        fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
                argv[0]);
        return 1;
    }


    std::vector<Server> vectorServer = parseFileWithAddressServer(parameters.pathtoserver);
    unsigned int servers_num = vectorServer.size();

    int buf,remainder = 0;
    if (servers_num >= parameters.k) { servers_num = parameters.k; }
    if ( parameters.k % servers_num == 0 ) { buf = parameters.k / servers_num ; remainder = 0;}
    else {remainder = parameters.k % servers_num ; buf = (parameters.k - remainder) / servers_num;}

    std::vector<std::thread> threads = std::vector<std::thread>();

    for (int i = 1; i < servers_num+1; i++) {
        int begin = 1 + ((i-1) * buf);
        int end =  i * buf;
        threads.emplace_back(std::thread(&sendToServer,vectorServer[i-1],parameters,begin,end));
    }
    for (int i = 0; i < servers_num; i++) {
        threads[i].join();
    }
    uint64_t result =1;
    for(int i =0;i<servers_num;++i)
    {
        result *= vectorResultServer[i];
        result = result % parameters.mod;
    }
    return result;
}