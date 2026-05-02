#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 64
#define MAX_EMPLOYEES 100

typedef struct {
    char name[64];
    int id;
    char level[32];
} Employee;

void search_employee(Employee *employees, int count) {
    int id;
    printf("\nВведите ID для поиска: ");
    scanf("%d", &id);

+
    printf("Сотрудник не найден\n");
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage: /lab15_lists/texBin staff.csv\n"); 
        exit(1);
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("Error opening file\n");
        return 1;
    }

    Employee employees[MAX_EMPLOYEES];
    int count = 0;

    char line[MAX_LINE];

    printf("+----------------------+----------+----------------+\n");
    printf("| Имя сотрудника       | ID       | Уровень        |\n");
    printf("+----------------------+----------+----------------+\n");

    while (fgets(line, sizeof(line), file) && count < MAX_EMPLOYEES) {

        char *name = strtok(line, ";\n");
        char *id = strtok(NULL, ";\n");
        char *level = strtok(NULL, ";\n");

        if (name && id && level) {

            employees[count].id = atoi(id);
            strcpy(employees[count].name, name);
            strcpy(employees[count].level, level);

            printf("| %-20s | %-8d | %-14s |\n",
                   employees[count].name,
                   employees[count].id,
                   employees[count].level);

            count++;
        }
    }

    printf("+----------------------+----------+----------------+\n");

    fclose(file);

    FILE *bin = fopen("database.dat", "wb");
    if (!bin) {
        printf("Error creating database.dat\n");
        return 1;
    }

    fwrite(employees, sizeof(Employee), count, bin);
    fclose(bin);


    search_employee(employees, count);

    return 0;
}