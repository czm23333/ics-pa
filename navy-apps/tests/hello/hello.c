#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  while (1) {
    void* tmp = malloc(600000);
    free(tmp);
  }

  write(1, "Hello World!\n", 13);
  int i = 2;
  volatile int j = 0;
  while (1) {
    j ++;
    if (j == 10000) {
      printf("Hello World from Navy-apps for the %dth time!\n", i ++);
      j = 0;
    }
  }
  return 0;
}
