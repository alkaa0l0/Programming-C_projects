#include <stdio.h>
#include "crypto.h"

int main() {
    struct crypto data[N];
    
    printf("До сортировки\n");
    fill_crypto_data(data);
    print_crypto_table(data);
    
    bubble_sort_by_price(data);
    
    printf("\nПосле сортировки\n");
    print_crypto_table(data);
    
    
    return 0;
}
