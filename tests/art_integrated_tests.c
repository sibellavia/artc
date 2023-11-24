#include "unity.h"
#include "../src/art.h"

void setUp(void) {
    // empty
}

void tearDown(void) {
    // empty
}

void test_createARTAndInsertNode(void) {

    ART *art = initializeAdaptiveRadixTree();
    TEST_ASSERT_NOT_NULL(art);
    TEST_ASSERT_NULL(art->root);
    TEST_ASSERT_EQUAL_UINT64(0, art->size);

    char *key = "key";
    char *value = "valore";

    insert(&(art->root), key, value, 0);

    TEST_ASSERT_NOT_NULL(art->root);
    TEST_ASSERT_EQUAL(art->root->type, LEAF);
    LeafNode *leafNode = (LeafNode *)(art->root);
    TEST_ASSERT_EQUAL_STRING(leafNode->key, key);
    TEST_ASSERT_EQUAL_STRING(leafNode->value, value);


    freeART(art);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_createARTAndInsertNode);
    return UNITY_END();
}