#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void *thread(void *arg){
    for (int i = 1; i <= 5; i++) {
        printf("Дочерний поток %d\n", i);
    }
}    
int main(void) {
    pthread_t tid;

    pthread_create(&tid, NULL, thread, NULL);


    pthread_join(tid, NULL);
    

    for (int i = 1; i <= 4; i++) {
        printf("Родительсктй поток %d\n", i);
    }
    return 0;


}