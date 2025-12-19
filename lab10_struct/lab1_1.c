#include <stdio.h>
#include <stdlib.h> 
#include "lab1_3.h"

int main() {
    
    struct Point *A = malloc(sizeof(struct Point));
    struct Point *B = malloc(sizeof(struct Point));
    struct Point point;


    printf("Введите координаты точки A (x y z): ");
    scanf("%d %d %d", &A->x, &A->y, &A->z);

    printf("Введите координаты точки B (x y z): ");
    scanf("%d %d %d", &B->x, &B->y, &B->z);

    double distance = calculate_distance(A, B);
    printf("Расстояние между точками: %.3f\n", distance);

    free(A);
    free(B);

    return 0;
}
