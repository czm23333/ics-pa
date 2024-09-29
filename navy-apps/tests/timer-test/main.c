#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <NDL.h>

int main(int argc, char* argv[]) {
    if (argc != 1) return -1;
    NDL_Init(0);
    uint32_t start = NDL_GetTicks();
    while (1) {
        uint32_t cur = NDL_GetTicks();
        if (cur - start >= 2000) {
            start = cur;
            printf("Timer %s triggered\n", argv[0]);
        }
    }
    NDL_Quit();
    return 0;
}