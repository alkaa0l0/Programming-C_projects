#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Room {
    char name[50];
    int level;
    int number;
    int resolution;
} Room;

typedef struct Node {
    Room *room;
    struct Node *prev;
    struct Node *next;
    struct Node *S;
} Node;

typedef struct Dungeon {
    Node *head;
    Node *tail;
    Node *current;
    Node *S;
    int size;
} Dungeon;

Room *createRoom(const char *name, int level, int number, int resolution) {
    Room *room = (Room *)malloc(sizeof(Room));
    strcpy(room->name, name);
    room->level = level;
    room->number = number;
    room->resolution = resolution;
    return room;
}

Node *createNode(Room *room) {
    Node *node = (Node *)malloc(sizeof(Node));
    node->room = room;
    node->prev = NULL;
    node->next = NULL;
    node->S = NULL;
    return node;
}

Dungeon *createDungeon() {
    Dungeon *dungeon = (Dungeon *)malloc(sizeof(Dungeon));
    dungeon->head = NULL;
    dungeon->tail = NULL;
    dungeon->current = NULL;
    dungeon->S = NULL;
    dungeon->size = 0;
    return dungeon;
}

void addToDungeon(Dungeon *dungeon, Room *room) {
    Node *newNode = createNode(room);
    
    if (dungeon->head == NULL) {
        dungeon->head = newNode;
        dungeon->tail = newNode;
        dungeon->current = newNode;
        dungeon->S = newNode;
    } else {
        newNode->prev = dungeon->tail;
        dungeon->tail->next = newNode;
        dungeon->tail = newNode;
        newNode->S = dungeon->S;
    }
    dungeon->size++;
}

void navigateWASD(Dungeon *dungeon) {
    
    while (1) {
        printf("\nЩас местонахождение: %s (ур.%d, №%d) ",
            dungeon->current->room->name,
            dungeon->current->room->level,
            dungeon->current->room->number);
        printf("Направление (WASD/q): ");
        
        char cmd;
        scanf(" %c", &cmd);
        
        switch (cmd) {
            case 'W': case 'w':  // Вверх
                if (dungeon->current->prev) {
                    dungeon->current = dungeon->current->prev;
                    printf("↑ Поднялись выше\n");
                } else {
                    printf("Начало данжа!\n");
                }
                break;
                
            case 'A': case 'a':  // К S
                dungeon->current = dungeon->S;
                printf("← Телепорт к началу \n");
                break;
                
            case 'S': case 's':  
                if (dungeon->current->next) {
                    dungeon->current = dungeon->current->next;
                    printf("↓ Следующая комната\n");
                } else {
                    printf(" Конец данжа!");
                }
                break;
                
            case 'D': case 'd':  
                if (dungeon->current->next) {
                    dungeon->current = dungeon->current->next;
                    printf("→ Вправо\n");
                } else {
                    printf("Конец ");
                }
                break;
                
            case 'Q': case 'q':
                printf("Выход.\n");
                return;
                
            default:
                printf(" WASD или q\n");
        }
    }
}



// void displayDungeon(Dungeon *dungeon) {
//     if (dungeon->head == NULL) {
//         printf("Данж пуст!\n");
//         return;
//     }
    
//     printf("\n(%d комнат)\n", dungeon->size);
//     Node *curr = dungeon->head;
//     int pos = 1;
    
//     while (curr) {
//         printf("%d. ", pos++);
//         if (curr == dungeon->current);
//         if (curr == dungeon->S) printf("S ");
//         printf("%s (ур.%d, №%d)\n",
//             curr->room->name, curr->room->level, curr->room->number);
//         curr = curr->next;
//     }

cur = head;
while (curr != NULL) {
    curr = curr->next

}




void doubleList(Dungeon *dungeon) {
    if (dungeon->head == NULL) {
        printf("Данж пуст\n");
        return;
    };

    Node *curr;
    curr = dungeon->head;
    int pos = 1;

    while( curr != NULL) {
        printf("%d. ", pos++);
        if (curr == dungeon->current) {
            printf("▶");
        }
        printf("%s (ур.%d, №%d)\n",
               curr->room->name,         
               curr->room->level,
               curr->room->number,
               curr->room->resolution); 
        curr = curr->next;
    }
    printf("====================\n\n");
}





int main() {
    Dungeon *dungeon = createDungeon();
    
    int n;
    printf("Сколько комнат? ");
    scanf("%d", &n);
    
    const char* names[] = {"Подвал", "Коридор", "Зал", "Башня", "Пещера", "Лабиринт", "Храм"};
    for (int i = 0; i < n; i++) {
        Room *r = createRoom(names[rand() % 7], 
                           rand() % 10, 
                           rand() % 100, 
                           rand() % 1000);
        addToDungeon(dungeon, r);
    }
    
    // displayDungeon(dungeon);
    navigateWASD(dungeon);
    doubleList(dungeon);
    
    return 0;
}
