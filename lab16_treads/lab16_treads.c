// #include <stdio.h>
// #include <pthread.h>
// #include <unistd.h>

// void *thread(void *arg){
//     for (int i = 1; i <= 5; i++) {
//         printf("Дочерний поток %d\n", i);
//     }
//     return NULL;
// }    
// int main(void) {
//     pthread_t tid;

//     pthread_create(&tid, NULL, thread, NULL);

//     pthread_join(tid, NULL);

//     for (int i = 1; i <= 5; i++) {
//         printf("Родительсктй поток %d\n", i);
//     }
//     return 0;

    
// }



// #include <stdio.h>
// #include <pthread.h>
// #include <unistd.h>

// typedef struct {
//     int num;
//     const char *text1;
//     const char *text2;
//     const char *text3;
//     const char *text4;
//     const char *text5;
// } ThreadData;


// void cleanup(void *arg){
//     int num = *(int *)arg;
//     printf("Поток %d завершает работу\n", num);

// }

// void *thread_func(void *arg)
// {
//     ThreadData *data = (ThreadData *)arg;

//     pthread_cleanup_push(cleanup, &data->num);

//     printf("Поток %d:\n", data->num);
//     printf("%s\n", data->text1);
//     sleep(1);
//     printf("%s\n", data->text2);
//     sleep(1);
//     printf("%s\n", data->text3);
//     sleep(1);
//     printf("%s\n", data->text4);
//     sleep(1);
//     printf("%s\n", data->text5);
//     sleep(1);

//     pthread_cleanup_pop(1);
//     return NULL;
// }

// int main(void)
// {
//     pthread_t tid[4];

//     ThreadData data[4] = {
//         {1, "Строка 1", "Строка 2", "Строка 3", "Строка 4", "Строка 5"},
//         {2, "А", "Б", "В", "Г", "Д"},
//         {3, "Один", "Два", "Три", "Четыре", "Пять"},
//         {4, "Первый", "Второй", "Третий", "Четвертый", "Пятый"}
//     };


//     for (int i = 0; i < 4; i++) {
//         pthread_create(&tid[i], NULL, thread_func, &data[i]);
//     }

//     sleep(2);

//     for (int i = 0; i < 4; i++) {
//         pthread_cancel(tid[i]); //Прерывает каждый доч поток
//     }

//     for (int i = 0; i < 4; i++) {
//         pthread_join(tid[i], NULL);
//     }
//     return 0;
// }



#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define SIZE 10

void *sleep_sort(void *arg)
{
    int num = *(int *)arg;

    sleep(num);

    printf("%d\n", num);

    return NULL;
}

int main()
{
    int arr[SIZE] = {3, 1, 5, 2, 4, 7,4,44,10,18};

    pthread_t tid[SIZE];

    for (int i = 0; i < SIZE; i++) {
        pthread_create(&tid[i], NULL, sleep_sort, &arr[i]);
    }

    for (int i = 0; i < SIZE; i++) {
        pthread_join(tid[i], NULL);
    }

    printf("\n");

    return 0;
}