#include <stdio.h>
#include <unistd.h>
int main(int argc, char *argv[]) {
  execv("parallel_min_max", argv);
  return 0;
}