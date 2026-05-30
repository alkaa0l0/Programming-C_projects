#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

struct Node {
    int data;
    struct Node *next;
};

struct Node *new_node(int x) {
    struct Node *n = (struct Node *)malloc(sizeof(struct Node));
    if (n != NULL) {
        n->data = x;
        n->next = NULL;
    }
    return n;
}

void free_list(struct Node *head) {
    while (head) {
        struct Node *tmp = head;
        head = head->next;
        free(tmp);
    }
}

bool isSorted(struct Node *head) {
    if (head == NULL || head->next == NULL)
        return true;

    bool increasing = true, decreasing = true;
    struct Node *curr = head;

    while (curr->next != NULL) {
        if (curr->data > curr->next->data)
            increasing = false;
        if (curr->data < curr->next->data)
            decreasing = false;
        curr = curr->next;
    }

    return increasing || decreasing;
}

static void test_empty_list(void **state) {
    (void)state;
    assert_true(isSorted(NULL));
}

static void test_single_element(void **state) {
    struct Node *head = new_node(42);
    assert_true(isSorted(head));
    free_list(head);
}

static void test_increasing(void **state) {
    struct Node *head = new_node(1);
    head->next = new_node(2);
    head->next->next = new_node(3);
    assert_true(isSorted(head));
    free_list(head);
}

static void test_decreasing(void **state) {
    struct Node *head = new_node(3);
    head->next = new_node(2);
    head->next->next = new_node(1);
    assert_false(isSorted(head));
    free_list(head);
}

static void test_non_sorted(void **state) {
    struct Node *head = new_node(1);
    head->next = new_node(3);
    head->next->next = new_node(2);
    assert_false(isSorted(head));
    free_list(head);
}

static void test_all_equal(void **state) {
    struct Node *head = new_node(5);
    head->next = new_node(5);
    head->next->next = new_node(5);
    assert_true(isSorted(head));
    free_list(head);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_empty_list),
        cmocka_unit_test(test_single_element),
        cmocka_unit_test(test_increasing),
        cmocka_unit_test(test_decreasing),
        cmocka_unit_test(test_non_sorted),
        cmocka_unit_test(test_all_equal),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}