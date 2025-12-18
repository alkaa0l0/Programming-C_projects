#include <math.h>
#include "lab1_3.h"

double calculate_distance(struct Point p1, struct Point p2) {
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    double dz = p2.z - p1.z;

    return sqrt(dx * dx + dy * dy + dz * dz);
}