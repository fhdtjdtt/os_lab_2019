#include "sum.h"

//#include <stdio.h>
//#include <stdlib.h>

int Sum(const struct SumArgs *args) {
  int sum = 0;
  for(int i=args->begin; i<args->end; i++) {
    sum+=args->array[i];
  }
  return sum;
}
