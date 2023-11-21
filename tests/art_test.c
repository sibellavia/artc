#include "unity.h"
#include "../art.h"

void setUp(void) {
    // empty
}

void tearDown(void) {
    // empty
}


void test_createRootNode_returnsNonNullPointer(void) {
    TEST_ASSERT_NOT_NULL(createRootNode());
}

void test_initializeAdaptiveRadixTree_returnsNonNullPointer(void) {
    TEST_ASSERT_NOT_NULL(initializeAdaptiveRadixTree());
}

void test_createRootNode_setsCorrectPrefixLen(void) {
    Node *root = createRootNode();
    TEST_ASSERT_EQUAL(NODE4, root->type);
}

void test_initializeAdaptiveRadixTree_setsCorrectRootNode(void) {
    ART *tree = initializeAdaptiveRadixTree();
    TEST_ASSERT_EQUAL(createRootNode(), tree->root);
}

int main(void){
    printf("Starting tests...\n");
    UNITY_BEGIN();

    RUN_TEST(test_createRootNode_returnsNonNullPointer);
    RUN_TEST(test_initializeAdaptiveRadixTree_returnsNonNullPointer);
    RUN_TEST(test_createRootNode_setsCorrectPrefixLen);
    RUN_TEST(test_initializeAdaptiveRadixTree_setsCorrectRootNode);
}