#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <NDL.h>

int main() {
    NDL_Init(0);
    uint32_t start = NDL_GetTicks();
    while (1) {
        uint32_t cur = NDL_GetTicks();
        if (cur - start >= 500) {
            start = cur;
            printf("Timer triggered\n");
        }
    }
    NDL_Quit();
    return 0;
}