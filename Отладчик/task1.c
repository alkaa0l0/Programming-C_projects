#include <stdio.h>
#include <stdlib.h>

void init(int **arr, int *n) { 
    *arr = malloc(*n * sizeof(int));
    if (*arr == NULL) return;
    for (int i = 0; i < *n; ++i) { 
        (*arr)[i] = i;     
    }
     *n = 5;
}

int main() {
    int *arr = NULL;
    int n = 10;

    init(&arr, &n);
    
    if (arr != NULL) {
        for (int i = 0; i < n; ++i) {
            printf("%d\n", arr[i]);
        }
        free(arr);
    }
    return 0;
}