#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>

int main() {
    struct timeval start;
    gettimeofday(&start, NULL);
    while (1) {
        struct timeval cur;
        gettimeofday(&cur, NULL);
        printf("%u\n", cur.tv_sec);
        if (cur.tv_sec > start.tv_sec || cur.tv_usec - start.tv_usec >= 500000) {
            start = cur;
            printf("Timer triggered");
        }
    }
}