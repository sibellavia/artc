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

    for (int i = 0; i < 3; i++) {
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

Node *makeComplexTree() {

    Node4 *root = makeNode4();
    setPrefix(root, "root", 4);

    for (int i = 0; i < 3; i++) {
        Node16 *childNode16 = makeNode16();
        setPrefix(childNode16, "node16_", 7);
        addChildToNode4(root, 'a' + i, (Node *)childNode16);

        for (int j = 0; j < 2; j++) {
            Node48 *childNode48 = makeNode48();
            setPrefix(childNode48, "node48_", 7);
            addChildToNode16(childNode16, '1' + j, (Node *)childNode48);

            LeafNode *leaf = makeLeafNode("leafkey", "value");
            addChildToNode48(childNode48, 'x', (Node *)leaf);
        }
    }

    return (Node *)root;
}

Node4 *makeNode4WithChildren() {
    Node4 *node = makeNode4();
    setPrefix(node, "test", 4);

    for (int i = 0; i < 3; i++) {
        char childKey = 'a' + i;  
        LeafNode *childLeaf = makeLeafNode(&childKey, "value"); 
        addChildToNode4(node, childKey, (Node *)childLeaf);  
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
    
    TEST_ASSERT_EQUAL_INT(0, root4->node.prefixLen);
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
    TEST_ASSERT_NOT_EQUAL_INT(1, root4->node.prefixLen);  
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

/* int getPrefixLength(Node *node) */

void test_getPrefixLength_Node4(void) {
    Node4 *node = makeNode4();
    TEST_ASSERT_NOT_NULL(node);

    node->node.prefixLen = 3;
    int result = getPrefixLength((Node *)node);
    TEST_ASSERT_EQUAL(3, result);

    free(node);
}

void test_getPrefixLength_Node16(void) {
    Node16 *node = makeNode16();
    TEST_ASSERT_NOT_NULL(node);

    node->node.prefixLen = 4;
    int result = getPrefixLength((Node *)node);
    TEST_ASSERT_EQUAL(4, result);

    free(node);
}

void test_getPrefixLength_Node48(void) {
    Node48 *node = makeNode48();
    TEST_ASSERT_NOT_NULL(node);

    node->node.prefixLen = 5;
    int result = getPrefixLength((Node *)node);
    TEST_ASSERT_EQUAL(5, result);

    free(node);
}

void test_getPrefixLength_Node256(void) {
    Node256 *node = makeNode256();
    TEST_ASSERT_NOT_NULL(node);

    node->node.prefixLen = 6;
    int result = getPrefixLength((Node *)node);
    TEST_ASSERT_EQUAL(6, result);

    free(node);
}
void test_getPrefixLength_InvalidNodeType(void) {
    Node invalidNode;
    invalidNode.type = 999;  // Invalid node

    int result = getPrefixLength(&invalidNode);
    TEST_ASSERT_EQUAL(INVALID, result);
}

/* int checkPrefix(Node *node, char *key, int depth) */

void test_checkPrefix_FullMatch_Node4(void) {
    Node4 *node = makeNode4();
    strcpy(node->node.prefix, "test");
    node->node.prefixLen = 4;

    char *key = "testkey";
    int result = checkPrefix((Node *)node, key, 0);
    TEST_ASSERT_EQUAL(4, result);

    free(node);
}

void test_checkPrefix_NoMatch_Node4(void) {
    Node4 *node = makeNode4();
    strcpy(node->node.prefix, "abcd");
    node->node.prefixLen = 4;

    char *key = "xyzkey";
    int result = checkPrefix((Node *)node, key, 0);
    TEST_ASSERT_EQUAL(0, result);

    free(node);
}

void test_checkPrefix_PartialMatch_Node4(void) {
    Node4 *node = makeNode4();
    strcpy(node->node.prefix, "abc");
    node->node.prefixLen = 3;

    char *key = "abxyz";
    int result = checkPrefix((Node *)node, key, 0);
    TEST_ASSERT_EQUAL(2, result);

    free(node);
}

void test_checkPrefix_DifferentDepths_Node4(void) {
    Node4 *node = makeNode4();
    strcpy(node->node.prefix, "hello");
    node->node.prefixLen = 5;

    char *key = "worldhello";
    int result = checkPrefix((Node *)node, key, 5);
    TEST_ASSERT_EQUAL(5, result);

    free(node);
}

void test_checkPrefix_FullMatch_Node16(void) {
    Node16 *node = makeNode16();
    strcpy(node->node.prefix, "test");
    node->node.prefixLen = 4;

    char *key = "testkey";
    int result = checkPrefix((Node *)node, key, 0);
    TEST_ASSERT_EQUAL(4, result);

    free(node);
}

void test_checkPrefix_NoMatch_Node16(void) {
    Node16 *node = makeNode16();
    strcpy(node->node.prefix, "abcd");
    node->node.prefixLen = 4;

    char *key = "xyzkey";
    int result = checkPrefix((Node *)node, key, 0);
    TEST_ASSERT_EQUAL(0, result);

    free(node);
}

void test_checkPrefix_PartialMatch_Node16(void) {
    Node16 *node = makeNode16();
    strcpy(node->node.prefix, "abc");
    node->node.prefixLen = 3;

    char *key = "abxyz";
    int result = checkPrefix((Node *)node, key, 0);
    TEST_ASSERT_EQUAL(2, result);

    free(node);
}

void test_checkPrefix_DifferentDepths_Node16(void) {
    Node16 *node = makeNode16();
    strcpy(node->node.prefix, "hello");
    node->node.prefixLen = 5;

    char *key = "worldhello";
    int result = checkPrefix((Node *)node, key, 5);
    TEST_ASSERT_EQUAL(5, result);

    free(node);
}

void test_checkPrefix_FullMatch_Node48(void) {
    Node48 *node = makeNode48();
    strcpy(node->node.prefix, "test");
    node->node.prefixLen = 4;

    char *key = "testkey";
    int result = checkPrefix((Node *)node, key, 0);
    TEST_ASSERT_EQUAL(4, result);

    free(node);
}

void test_checkPrefix_NoMatch_Node48(void) {
    Node48 *node = makeNode48();
    strcpy(node->node.prefix, "abcd");
    node->node.prefixLen = 4;

    char *key = "xyzkey";
    int result = checkPrefix((Node *)node, key, 0);
    TEST_ASSERT_EQUAL(0, result);

    free(node);
}

void test_checkPrefix_PartialMatch_Node48(void) {
    Node48 *node = makeNode48();
    strcpy(node->node.prefix, "abc");
    node->node.prefixLen = 3;

    char *key = "abxyz";
    int result = checkPrefix((Node *)node, key, 0);
    TEST_ASSERT_EQUAL(2, result);

    free(node);
}

void test_checkPrefix_DifferentDepths_Node48(void) {
    Node48 *node = makeNode48();
    strcpy(node->node.prefix, "hello");
    node->node.prefixLen = 5;

    char *key = "worldhello";
    int result = checkPrefix((Node *)node, key, 5);
    TEST_ASSERT_EQUAL(5, result);

    free(node);
}

/* Insert algorithm */

void test_insert_intoEmptyTree() {
    ART *art = initializeAdaptiveRadixTree();
    TEST_ASSERT_NOT_NULL(art);
    TEST_ASSERT_NULL(art->root);

    char *key = "test";
    void *value = "testvalue";
    insert(&(art->root), key, value, 0);

    TEST_ASSERT_NOT_NULL(art->root);
    TEST_ASSERT_EQUAL(LEAF, art->root->type);
    TEST_ASSERT_EQUAL_STRING(key, ((LeafNode *)art->root)->key);
    TEST_ASSERT_EQUAL_STRING(value, ((LeafNode *)art->root)->value);

    freeART(art);
}

void test_growNode4ToNode16() {
    ART *art = initializeAdaptiveRadixTree();
    
    for (int i = 0; i < 4; i++) {
        char key[5];
        sprintf(key, "key%d", i);
        void *value = "value";
        insert(&(art->root), key, value, 0);
    }
    
    char *newKey = "key4";
    void *newValue = "value";
    insert(&(art->root), newKey, newValue, 0);

    TEST_ASSERT_NOT_NULL(art->root);
    TEST_ASSERT_EQUAL(NODE16, art->root->type);
    
    freeART(art);
}

void test_growNode16ToNode48() {
    ART *art = initializeAdaptiveRadixTree();
    
    for (int i = 0; i < 16; i++) {
        char key[10];
        snprintf(key, sizeof(key), "key16%d", i);
        void *value = "value16";
        insert(&(art->root), key, value, 0);
    }

    TEST_ASSERT_NOT_NULL(art->root);
    TEST_ASSERT_EQUAL(NODE16, art->root->type);

    for (int i = 0; i < 4; i++) {
        char newKey[7];
        sprintf(newKey, "key48%d", i);
        void *newValue = "value48";
        insert(&(art->root), newKey, newValue, 0);
    }


    TEST_ASSERT_NOT_NULL(art->root);
    TEST_ASSERT_EQUAL(NODE48, art->root->type);

    freeART(art);
}

void test_growNode48ToNode256() {
    ART *art = initializeAdaptiveRadixTree();
    
    for (int i = 0; i < 48; i++) {
        char key[7];
        snprintf(key, sizeof(key), "key48%d", i);
        void *value = "value48";
        insert(&(art->root), key, value, 0);
    }

    TEST_ASSERT_NOT_NULL(art->root);
    TEST_ASSERT_EQUAL(NODE48, art->root->type);

    for (int i = 0; i < 5; i++) {
        char newKey[8];
        snprintf(newKey, sizeof(newKey), "key256%d", i);
        void *newValue = "value256";
        insert(&(art->root), newKey, newValue, 0);
    }

    TEST_ASSERT_NOT_NULL(art->root);
    TEST_ASSERT_EQUAL(NODE256, art->root->type);

    freeART(art);
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
    RUN_TEST(test_findChildBinary_NonExistingByte);
    RUN_TEST(test_findChildBinary_EmptyNode);
    RUN_TEST(test_findChildBinary_ExistingChild_Node48);
    RUN_TEST(test_findChildBinary_NonExistingByte_Node48);
    RUN_TEST(test_findChildBinary_ExistingChild_Node256);

    /* int getPrefixLength(Node *node) */
    RUN_TEST(test_getPrefixLength_Node4);
    RUN_TEST(test_getPrefixLength_Node16);
    RUN_TEST(test_getPrefixLength_Node48);
    RUN_TEST(test_getPrefixLength_Node256);
    RUN_TEST(test_getPrefixLength_InvalidNodeType);

    /* int checkPrefix(Node *node, char *key, int depth) */
    // Node4 tests
    RUN_TEST(test_checkPrefix_FullMatch_Node4);
    RUN_TEST(test_checkPrefix_NoMatch_Node4);
    RUN_TEST(test_checkPrefix_PartialMatch_Node4);
    RUN_TEST(test_checkPrefix_DifferentDepths_Node4);

    // Node16 tests
    RUN_TEST(test_checkPrefix_FullMatch_Node16);
    RUN_TEST(test_checkPrefix_NoMatch_Node16);
    RUN_TEST(test_checkPrefix_PartialMatch_Node16);
    RUN_TEST(test_checkPrefix_DifferentDepths_Node16);

    // Node48 tests
    RUN_TEST(test_checkPrefix_FullMatch_Node48);
    RUN_TEST(test_checkPrefix_NoMatch_Node48);
    RUN_TEST(test_checkPrefix_PartialMatch_Node48);
    RUN_TEST(test_checkPrefix_DifferentDepths_Node48);

    /* Insert algorithm */
    RUN_TEST(test_insert_intoEmptyTree);
    RUN_TEST(test_growNode4ToNode16);
    RUN_TEST(test_growNode16ToNode48);
    RUN_TEST(test_growNode48ToNode256);
}