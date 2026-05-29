#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


typedef struct 
{
    int thread_id;
    const char *lines[5];
}
ThreadArgs;

void *printLines(void *arg) {
    ThreadArgs *data = (ThreadArgs *)arg;
    for (int i; i <= 5; i++){
        printf("Поток ")
    }
}
