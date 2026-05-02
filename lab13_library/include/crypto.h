#ifndef CRYPTO_H
#define CRYPTO_H

#define N 4

struct crypto {
    char name[8];
    int count;
    int price;
    int change;
};

void fill_crypto_data(struct crypto data[]);
void print_crypto_table(const struct crypto data[]);
void bubble_sort_by_price(struct crypto data[]);

#endif
