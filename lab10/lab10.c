#include <stdio.h>
#include <math.h>

struct Point {


    int x;
    int y;
    int z;

};

int main() {

    struct Point A, B;

    printf("Введите координаты точки A (x, y, z): ");
    scanf("%d %d %d", &A.x, &A.y, &A.z);

    printf("Введите координаты точки B (x, y, z): ");
    scanf("%d %d %d", &B.x, &B.y, &B.z);


    double dx = B.x - A.x;
    double dy = B.y - A.y;
    double dz = B.z - A.z;

    double distance = sqrt(dx * dx + dy * dy + dz * dz);

    printf("Расстояние между точками: %.3f\n", distance);

    return 0;

    
}
