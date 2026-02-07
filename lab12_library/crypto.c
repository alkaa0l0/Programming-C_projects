#include <stdio.h>
#define N 4

struct crypto
{
    char name[8];
    int count;
    int price;
    int change;
};

int main() {
    struct crypto data[N] = 
    {
        {"BTC", 1, 70.212, -314.87},
        {"ETH", 1, 3.884, +68},
        {"USDT", 1, 1.00, 0},
        {"BNB", 1, 850, +58}
    };

    printf("Данные криптовалют:\n");
    return 0;
}
