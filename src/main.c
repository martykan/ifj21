#include<stdio.h>
#include "errors.h"

int main(int argc, char** argv) {
  printf("%i %s\n", argc, argv[0]);
  return EXITSTATUS_OK;
}
