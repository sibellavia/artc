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
    char *value = "value";

    insert(&(art->root), key, value, 0);

    TEST_ASSERT_NOT_NULL(art->root);
    TEST_ASSERT_EQUAL(art->root->type, LEAF);
    LeafNode *leafNode = (LeafNode *)(art->root);
    TEST_ASSERT_EQUAL_STRING(leafNode->key, key);
    TEST_ASSERT_EQUAL_STRING(leafNode->value, value);


    freeART(art);
}

void test_insertMultipleNodes(void) {
    ART *art = initializeAdaptiveRadixTree();
    TEST_ASSERT_NOT_NULL(art);
    TEST_ASSERT_NULL(art->root);

    char *key1 = "key1";
    char *value1 = "value1";
    insert(&(art->root), key1, value1, 0);
    
    char *key2 = "key2";
    char *value2 = "value2";
    insert(&(art->root), key2, value2, 0);

    // Verify that the root node has changed after the second insertion
    // Assume that after two nodes are inserted, the root node becomes a Node4
    TEST_ASSERT_NOT_NULL(art->root);
    TEST_ASSERT_EQUAL(art->root->type, NODE4);

    freeART(art);
}

void test_transitionFromNode4ToNode16(void) {
    ART *art = initializeAdaptiveRadixTree();
    TEST_ASSERT_NOT_NULL(art);
    TEST_ASSERT_NULL(art->root);

    int numKeysToInsert = 5;

    for (int i = 0; i < numKeysToInsert; i++) {
        char key[10];
        sprintf(key, "key%d", i);
        char *value = "value";
        insert(&(art->root), key, value, 0);
    }

    // Verify that the root node is a Node16
    TEST_ASSERT_NOT_NULL(art->root);
    TEST_ASSERT_EQUAL(art->root->type, NODE16);

    freeART(art);
}


int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_createARTAndInsertNode);

    RUN_TEST(test_insertMultipleNodes); 

    RUN_TEST(test_transitionFromNode4ToNode16);
    
    return UNITY_END();
}