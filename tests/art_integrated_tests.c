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

void test_transitionFromNode16ToNode48(void) {
    ART *art = initializeAdaptiveRadixTree();
    TEST_ASSERT_NOT_NULL(art);
    TEST_ASSERT_NULL(art->root);

    int numKeysToInsert = 18;

    for (int i = 0; i < numKeysToInsert; i++) {
        char key[10];
        sprintf(key, "key%d", i);
        void *value = "value";
        insert(&(art->root), key, value, 0);
    }

    TEST_ASSERT_NOT_NULL(art->root);
    TEST_ASSERT_EQUAL(art->root->type, NODE48);

    freeART(art);
}

void test_transitionFromNode16ToNode48UnderNode4(void) {
    ART *art = initializeAdaptiveRadixTree();
    TEST_ASSERT_NOT_NULL(art);
    TEST_ASSERT_NULL(art->root);

    // Inserisci chiavi sufficienti per creare un Node4 alla radice
    int keysForNode4 = 4;
    for (int i = 0; i < keysForNode4; i++) {
        char key[10];
        sprintf(key, "a%d", i);  // Chiavi che creeranno un Node4 alla radice
        void *value = (void *)"value";
        insert(&(art->root), key, value, 0);
    }

    // Inserisci ulteriori chiavi che finiscono sotto lo stesso figlio di Node4
    int keysForNode16 = 16;
    for (int i = 0; i < keysForNode16; i++) {
        char key[10];
        sprintf(key, "ab%d", i);  // Chiavi che creeranno e riempiranno un Node16 sotto un figlio di Node4
        void *value = (void *)"value";
        insert(&(art->root), key, value, 0);
    }

    // Inserisci un'altra chiave per innescare la transizione da Node16 a Node48
    char extraKey[] = "ab16";
    insert(&(art->root), extraKey, (void *)"value", 0);

    // Verifica che la radice sia un Node4
    TEST_ASSERT_NOT_NULL(art->root);
    TEST_ASSERT_EQUAL(NODE4, art->root->type);

    // Verifica che uno dei figli di Node4 sia diventato Node48
    Node4 *rootNode = (Node4 *)art->root;
    bool foundNode48 = false;
    for (int i = 0; i < rootNode->count; i++) {
        if (rootNode->children[i]->type == NODE48) {
            foundNode48 = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(foundNode48);

    freeART(art);
}

void test_Node4ToNode16Growth(void) {
    ART *art = initializeAdaptiveRadixTree();
    TEST_ASSERT_NOT_NULL(art);

    // Passo 1: Inserisci chiavi per creare un Node4 alla radice
    for (int i = 0; i < 4; i++) {
        char key[10];
        sprintf(key, "a%d", i);
        printf("Inserting %s at root\n", key);
        insert(&(art->root), key, (void *)"value", 0);
    }

    // Passo 2: Inserisci chiavi per creare un Node4 come figlio del Node4 radice
    for (int i = 0; i < 4; i++) {
        char key[10];
        sprintf(key, "aa%d", i);  // Prefisso comune "aa" per il secondo Node4
        printf("Inserting %s under first Node4\n", key);
        insert(&(art->root), key, (void *)"value", 0);
    }

    // Verifica che la radice sia un Node4
    TEST_ASSERT_EQUAL(NODE4, art->root->type);

    // Passo 3: Inserisci ulteriori chiavi per far crescere il secondo Node4 in un Node16
    for (int i = 4; i < 20; i++) {
        char key[10];
        sprintf(key, "aa%d", i);  // Continua con il prefisso "aa"
        printf("Inserting %s to grow second Node4 to Node16\n", key);
        insert(&(art->root), key, (void *)"value", 0);
    }

    // Verifica che il figlio di Node4 sia cresciuto in Node16
    Node4 *rootNode4 = (Node4 *)art->root;
    bool foundNode16 = false;
    for (int i = 0; i < rootNode4->count; i++) {
        if (rootNode4->children[i]->type == NODE16) {
            foundNode16 = true;
            printf("Second Node4 has grown to Node16\n");
            break;
        }
    }
    TEST_ASSERT_TRUE_MESSAGE(foundNode16, "Second Node4 should have grown into a Node16 but did not");

    freeART(art);
}

// int main(void) {
//     UNITY_BEGIN();

//     RUN_TEST(test_createARTAndInsertNode);

//     RUN_TEST(test_insertMultipleNodes); 

//     RUN_TEST(test_transitionFromNode4ToNode16);
    
//     RUN_TEST(test_transitionFromNode16ToNode48);
    
//     RUN_TEST(test_transitionFromNode16ToNode48UnderNode4);

//     return UNITY_END();
// }