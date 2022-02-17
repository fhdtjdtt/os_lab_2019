#include <thread>
#include <vector>
#include <iostream>
#include "string"

std::vector<uint64_t> resultMylti = std::vector<uint64_t>();

void* Multiplication(std::size_t i, int k , int Qty_multiplications , int Remainder, int Mod , int Pnum){
    int Intermediate_result = 1;
    if (Remainder == 0){
        for (int j = 1 + ((i-1) * Qty_multiplications); j <= i * Qty_multiplications  ; ++j ) {
            Intermediate_result = Intermediate_result * j;
            Intermediate_result = Intermediate_result % Mod;
        }
    }
    else{
        if (i != Pnum )
            for (int j = 1 + ((i-1) * Qty_multiplications); j <= i * Qty_multiplications  ; ++j ) {
                Intermediate_result = Intermediate_result * j;
                Intermediate_result = Intermediate_result % Mod;
            }
        else if(i == Pnum){
            for (int j = 1 + ((i-1) * Qty_multiplications); j <= k ; ++j ) {
                Intermediate_result = Intermediate_result * j;
                Intermediate_result = Intermediate_result % Mod;
            }
        }
    }
    resultMylti.push_back(Intermediate_result);
    return nullptr;
}

int main(int argc, char *argv[]) {
    std::vector<std::thread> ths;
    int Pnum, Number, buf, Remainder, Mod;
    long int Result = 1;

    Number = std::stoi(std::string(argv[2]));
    Pnum = std::stoi(std::string(argv[3]).substr(std::string(argv[3]).find('=') + 1, -1));
    Mod = std::stoi(std::string(argv[4]).substr(std::string(argv[4]).find('=') + 1, -1));
    if (Number == 0 or Number == 1) { std::cout << "Результат  = 1"; }

    else {
        if (Pnum >= Number) { Pnum = Number; }
        if (Number % Pnum == 0) {
            buf = Number / Pnum;
            Remainder = 0;
        }
        else {
            Remainder = Number % Pnum;
            buf = (Number - Remainder) / Pnum;
        }

        std::vector<std::thread> threads = std::vector<std::thread>();

        for (std::size_t i = 1; i < (Pnum + 1); ++i) {
            threads.emplace_back(std::thread(&Multiplication, i, Number, buf, Remainder, Mod, Pnum));
        }
        for (std::size_t i = 0; i < Pnum; ++i) {
            threads[i].join();
        }

        for (std::size_t i = 0; i < Pnum; ++i) {
            Result *= resultMylti[i];
            Result %= Mod;
        }

        std::cout << Result;

    }
}