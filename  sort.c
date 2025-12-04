#include <stdio.h>
#include <stdlib.h>

int comp(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

int main() {
    int arr[] = {5, 2, 3, 1, 4};
    int n = sizeof(arr) / sizeof(arr[0]);

    // Sort the array arr
    qsort(arr, n, sizeof(arr[0]), comp);

    for (int i = 0; i < n; i++)
        printf("%d ", arr[i]);
    return 0;
}