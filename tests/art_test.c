#include "unity.h"
#include "../src/art.h"

void setUp(void) {
    // empty
}

void tearDown(void) {
    // empty
}

/* Dummy structures and functions for tests */
typedef struct ExampleChild {
    int dummy;
} ExampleChild;

Node16* initializeExampleNode16() {
    Node16 *node = makeNode16();
    if (!node) {
        return NULL;
    }

    node->count = 3;

    for (int i = 0; i < node->count; i++) {
        ExampleChild *child = malloc(sizeof(ExampleChild));
        child->dummy = i;

        node->children[i] = (Node *)child;
        node->keys[i] = 'a' + i;
    }

    return node;
}

Node48* initializeExampleNode48() {
    Node48 *node = makeNode48();
    if (!node) {
        return NULL;
    }

    for (unsigned char i = 0; i < 3; i++) {
        ExampleChild *child = malloc(sizeof(ExampleChild));
        child->dummy = i;

        node->children[i] = (Node *)child;
        node->keys[i] = i;
    }

    return node;
}

Node256* initializeExampleNode256() {
    Node256 *node = makeNode256();
    if (!node) {
        return NULL;
    }

    
    int selectedIndices[] = {50, 100, 150, 200};
    int numOfChildren = sizeof(selectedIndices) / sizeof(selectedIndices[0]);

    for (int i = 0; i < numOfChildren; i++) {
        ExampleChild *child = malloc(sizeof(ExampleChild));
        child->dummy = i;

        node->children[selectedIndices[i]] = (Node *)child;
    }

    return node;
}


/* Node *createRootNode() */

void test_createRootNode_returnsNonNullPointer(void) {
    TEST_ASSERT_NOT_NULL(createRootNode());
}

void test_createRootNode_ShouldAllocateMemory(void) {
    Node *root = createRootNode();
    TEST_ASSERT_NOT_NULL(root);
    free(root);
}

void test_createRootNode_ShouldBeTypeNode4(void) {
    Node *root = createRootNode();
    TEST_ASSERT_NOT_NULL(root);
    TEST_ASSERT_EQUAL(NODE4, root->type);
    free(root);
}

void test_createRootNode_PrefixLenShouldBeZero(void) {
    Node *root = createRootNode();
    TEST_ASSERT_NOT_NULL(root);
    Node4 *root4 = (Node4 *)root;
    TEST_ASSERT_EQUAL_INT(0, root4->prefixLen);
    free(root);
}

void test_createRootNode_KeysShouldBeEmpty(void) {
    Node *root = createRootNode();
    TEST_ASSERT_NOT_NULL(root);
    Node4 *root4 = (Node4 *)root;

    for (int i = 0; i < 4; i++) {
        TEST_ASSERT_EQUAL_HEX8(EMPTY_KEY, root4->keys[i]);
    }
    free(root);
}

void test_createRootNode_WrongNodeType(void) {
    Node *root = createRootNode();
    TEST_ASSERT_NOT_NULL(root);
    TEST_ASSERT_NOT_EQUAL(NODE16, root->type);
    free(root);
}

void test_createRootNode_WrongPrefixLen(void) {
    Node *root = createRootNode();
    TEST_ASSERT_NOT_NULL(root);
    Node4 *root4 = (Node4 *)root;
    TEST_ASSERT_NOT_EQUAL_INT(1, root4->prefixLen);  
    free(root);
}

/* ART *initializeAdaptiveRadixTree() */

void test_initializeAdaptiveRadixTree_Allocation(void) {
    ART *tree = initializeAdaptiveRadixTree();
    TEST_ASSERT_NOT_NULL(tree);
    free(tree);
}

void test_initializeAdaptiveRadixTree_RootNode(void) {
    ART *tree = initializeAdaptiveRadixTree();
    TEST_ASSERT_NOT_NULL(tree);
    TEST_ASSERT_NOT_NULL(tree->root);
    TEST_ASSERT_EQUAL(NODE4, tree->root->type);
    free(tree);
}

void test_initializeAdaptiveRadixTree_InitialSize(void) {
    ART *tree = initializeAdaptiveRadixTree();
    TEST_ASSERT_NOT_NULL(tree);
    TEST_ASSERT_EQUAL_INT(0, tree->size);
    free(tree);
}

/* Node *findChildBinary(Node *genericNode, char byte) */

void test_findChildBinary_ExistingChild(void) {
    Node16 *node = initializeExampleNode16();
    TEST_ASSERT_NOT_NULL(node);

    char byte = 'b';
    Node *expectedChild = node->children[1];  

    Node *result = findChildBinary(node, byte);
    TEST_ASSERT_EQUAL_PTR(expectedChild, result);

    for (int i = 0; i < node->count; i++) {
        free(node->children[i]);
    }
    free(node);
}

void test_findChildBinary_NonExistingByte(void) {
    Node16 *node = initializeExampleNode16();
    char byte = 'x';

    Node *result = findChildBinary(node, byte);
    TEST_ASSERT_NULL(result);

    free(node);
}

void test_findChildBinary_EmptyNode(void) {
    Node16 *node = initializeExampleNode16();

    char byte = 'f';
    Node *result = findChildBinary(node, byte);
    TEST_ASSERT_NULL(result);

    free(node);
}

void test_findChildBinary_Extremes(void) {
    Node16 *node = initializeExampleNode16();

    char firstByte = node->keys[0];
    Node *firstChild = findChildBinary(node, firstByte);
    TEST_ASSERT_EQUAL_PTR(node->children[0], firstChild);

    char lastByte = node->keys[node->count - 1];
    Node *lastChild = findChildBinary(node, lastByte);
    TEST_ASSERT_EQUAL_PTR(node->children[node->count - 1], lastChild);

    free(node);
}

void test_findChildBinary_ByteComparison(void) {
    Node16 *node = initializeExampleNode16();

    for (int i = 0; i < node->count; i++) {
        Node *child = findChildBinary(node, node->keys[i]);
        TEST_ASSERT_EQUAL_PTR(node->children[i], child);
    }

    free(node);
}

void test_findChildBinary_ExistingChild_Node48(void) {
    Node48 *node = initializeExampleNode48();
    TEST_ASSERT_NOT_NULL(node);

    char byte = 1;
    Node *expectedChild = node->children[byte];

    Node *result = findChildBinary(node, byte);
    TEST_ASSERT_EQUAL_PTR(expectedChild, result);

    for (int i = 0; i < 3; i++) {
        free(node->children[i]);
    }
    free(node);
}

void test_findChildBinary_NonExistingByte_Node48(void) {
    Node48 *node = initializeExampleNode48();
    TEST_ASSERT_NOT_NULL(node);

    char byte = 5;
    Node *result = findChildBinary(node, byte);
    TEST_ASSERT_NULL(result);

    for (int i = 0; i < 3; i++) {
        free(node->children[i]);
    }
    free(node);
}

void test_findChildBinary_ExistingChild_Node256(void) {
    Node256 *node = initializeExampleNode256();
    TEST_ASSERT_NOT_NULL(node);

    char byte = 50;
    Node *expectedChild = node->children[byte];

    Node *result = findChildBinary((Node *)node, byte);
    TEST_ASSERT_EQUAL_PTR(expectedChild, result);

    for (int i = 0; i < 256; i++) {
        free(node->children[i]);
    }
    free(node);
}

/* Main */

int main(void){
    printf("Starting tests...\n");
    UNITY_BEGIN();

    /* createRootNode */
    RUN_TEST(test_createRootNode_returnsNonNullPointer);
    RUN_TEST(test_createRootNode_ShouldAllocateMemory);
    RUN_TEST(test_createRootNode_ShouldBeTypeNode4);
    RUN_TEST(test_createRootNode_PrefixLenShouldBeZero);
    RUN_TEST(test_createRootNode_KeysShouldBeEmpty);
    RUN_TEST(test_createRootNode_WrongNodeType);
    RUN_TEST(test_createRootNode_WrongPrefixLen);

    /* initializeAdaptiveRadixTree */
    RUN_TEST(test_initializeAdaptiveRadixTree_Allocation);
    RUN_TEST(test_initializeAdaptiveRadixTree_RootNode);
    RUN_TEST(test_initializeAdaptiveRadixTree_InitialSize);

    /* findChildBinary */
    RUN_TEST(test_findChildBinary_ExistingChild);
    RUN_TEST(test_findChildBinary_NonExistingByte);
    RUN_TEST(test_findChildBinary_EmptyNode);
    RUN_TEST(test_findChildBinary_Extremes);
    RUN_TEST(test_findChildBinary_ByteComparison);
    RUN_TEST(test_findChildBinary_ExistingChild_Node48);
    RUN_TEST(test_findChildBinary_NonExistingByte_Node48);
    RUN_TEST(test_findChildBinary_ExistingChild_Node256);
}