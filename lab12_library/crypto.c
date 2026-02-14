#include <stdio.h>
#include <string.h>
#include "crypto.h"

void fill_crypto_data(struct crypto data[]) {
    const char* names[] = {"BTC", "ETH", "USDT", "BNB"};
    
    for (int i = 0; i < N; i++) {
        strcpy(data[i].name, names[i]);
        data[i].count = 1;
        
        if (i == 0) {  // BTC
            data[i].price = 65234;
            data[i].change = -456;
        } else if (i == 1) {  // ETH
            data[i].price = 2845;
            data[i].change = 123;
        } else if (i == 2) {  // USDT
            data[i].price = 100;
            data[i].change = 0;
        } else {  // BNB
            data[i].price = 567;
            data[i].change = 89;
        }
    }
    fflush(stdout);
}

void print_crypto_table(const struct crypto data[]) {
    printf("+-----------------+-------+----------+--------+\n");
    printf("| %-15s | %-5s | %-8s | %-6s |\n", "Название", "Кол-во", "Цена", "Измен.");
    printf("+-----------------+-------+----------+--------+\n");
    
    for (int i = 0; i < N; i++) {
        printf("| %-15s | %-5d | $%7d | %+.0d%% |\n", 
               data[i].name, data[i].count, data[i].price, data[i].change);
        fflush(stdout);
    }
    printf("+-----------------+-------+----------+--------+\n");
}

void bubble_sort_by_price(struct crypto data[]) {
    for (int i = 0; i < N - 1; i++) {
        for (int j = 0; j < N - i - 1; j++) {
            if (data[j].price > data[j + 1].price) {
                struct crypto temp = data[j];
                data[j] = data[j + 1];
                data[j + 1] = temp;
            }
        }
    }
}
