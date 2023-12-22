#include "unity.h"
#include "../src/art.h"

void setUp(void) {
    // empty
}

void tearDown(void) {
    // empty
}

/*** TESTS ***/

void test_createRootNode(void) {
    Node *root = createRootNode();
    TEST_ASSERT_NOT_NULL(root);
    TEST_ASSERT_EQUAL(NODE4, root->type);
    freeNode(root);
}

void test_initializeAdaptiveRadixTree(void) {
    ART *tree = initializeAdaptiveRadixTree();
    TEST_ASSERT_NOT_NULL(tree);
    freeART(tree);
}

void test_insertAndFind(void) {
    ART *tree = initializeAdaptiveRadixTree();
    char *key = "key1";
    int value = 123;
    insertString(&tree->root, key, &value, sizeof(value));
    
    Node *found = findChild(tree->root, 'k');
    TEST_ASSERT_NOT_NULL(found);
    
    freeART(tree);
}

void test_insertAndFindString(void) {
    ART *tree = initializeAdaptiveRadixTree();
    char *key = "testKey";
    char *value = "testValue";
    insertString(&tree->root, key, value, strlen(value) + 1);

    Node *found = findChild(tree->root, key[0]);
    TEST_ASSERT_NOT_NULL(found);
    
    LeafNode *leaf = (LeafNode *)found;
    TEST_ASSERT_EQUAL_STRING(value, leaf->value);

    freeART(tree);
}

void test_insertAndFindInt(void) {
    ART *tree = initializeAdaptiveRadixTree();
    int key = 12345;
    int value = 67890;
    insertInt(&tree->root, key, &value, sizeof(value));

    Node *found = findChild(tree->root, key);
    TEST_ASSERT_NOT_NULL(found);
    
    LeafNode *leaf = (LeafNode *)found;
    TEST_ASSERT_EQUAL_INT(value, *(int *)leaf->value);

    freeART(tree);
}

void test_integratedART(void) {
    ART *tree = initializeAdaptiveRadixTree();

    // Inserisci una serie di stringhe.
    char *keysString[] = {"alpha", "beta", "gamma", "delta"};
    char *valuesString[] = {"data1", "data2", "data3", "data4"};
    for (int i = 0; i < 4; i++) {
        insertString(&tree->root, keysString[i], valuesString[i], strlen(valuesString[i]) + 1);
    }

    // Inserisci una serie di interi.
    int keysInt[] = {10, 20, 30, 40};
    int valuesInt[] = {100, 200, 300, 400};
    for (int i = 0; i < 4; i++) {
        insertInt(&tree->root, keysInt[i], &valuesInt[i], sizeof(valuesInt[i]));
    }

    // Verifica la ricerca di una chiave stringa specifica.
    Node *foundString = findChild(tree->root, 'a');  // Inizia la ricerca per 'alpha'.
    TEST_ASSERT_NOT_NULL(foundString);
    // Assumi che 'foundString' sia un LeafNode e verifica che il valore sia corretto.
    // (Devi aggiungere logica per navigare dall'inizio della chiave fino al leaf corrispondente).
    LeafNode *leafString = (LeafNode *)foundString;
    TEST_ASSERT_EQUAL_STRING("data1", leafString->value);

    // Verifica la ricerca di una chiave intera specifica.
    Node *foundInt = findChild(tree->root, 20);  // Cerca il valore associato a 20.
    TEST_ASSERT_NOT_NULL(foundInt);
    // Assumi che 'foundInt' sia un LeafNode e verifica che il valore sia corretto.
    LeafNode *leafInt = (LeafNode *)foundInt;
    TEST_ASSERT_EQUAL_INT(200, *(int *)leafInt->value);

    freeART(tree);
}

void test_integratedARTExpansion(void) {
    ART *tree = initializeAdaptiveRadixTree();

    // Inserisci una serie di stringhe che dovrebbero portare a espansioni del nodo.
    char *keys[] = {"key1", "key2", "key3", "key4", "key5", "key6", "key7", "key8", "key9", "key10", "key11", "key12", "key13", "key14", "key15", "key16", "key17", "key18", "key19", "key20"};
    char *values[] = {"val1", "val2", "val3", "val4", "val5", "val6", "val7", "val8", "val9", "val10", "val11", "val12", "val13", "val14", "val15", "val16", "val17", "val18", "val19", "val20"};
    for (int i = 0; i < 8; i++) {
        insertString(&tree->root, keys[i], values[i], strlen(values[i]) + 1);
    }

    // Dopo l'inserimento di alcuni elementi, verifica il tipo di nodo della radice.
    // Questo dipenderà dal tuo algoritmo specifico e dai valori di soglia per l'espansione.
    // Ad esempio, se il tuo ART si espande a NODE16 dopo 4 inserimenti, puoi verificarlo così:
    TEST_ASSERT_MESSAGE(tree->root->type == NODE16, "Root should have expanded to a NODE16 type.");

    // Continua con ulteriori inserimenti e verifiche se necessario.
    for (int i = 8; i < 18; i++) {
        insertString(&tree->root, keys[i], values[i], strlen(values[i]) + 1);
    }

    // Verifica se l'albero si è espanso a NODE48 o a un altro tipo di nodo appropriato.
    TEST_ASSERT_MESSAGE(tree->root->type == NODE48, "Root should have expanded to a NODE48 type.");

    // Effettua una ricerca per verificare che i valori possano essere correttamente recuperati dopo l'espansione.
    for (int i = 0; i < 20; i++) {  
        Node *found = findChild(tree->root, keys[i][0]);
        TEST_ASSERT_NOT_NULL(found);
        // Assumi che 'found' sia un LeafNode e verifica che il valore sia corretto.
        LeafNode *leaf = (LeafNode *)found;
        TEST_ASSERT_EQUAL_STRING(values[i], leaf->value);
    }

    freeART(tree);
}

/*** MAIN ***/

int main(void){
    printf("Starting tests...\n");
    UNITY_BEGIN();

    // Run tests
    RUN_TEST(test_createRootNode);
    RUN_TEST(test_initializeAdaptiveRadixTree);
    RUN_TEST(test_insertAndFind);
    RUN_TEST(test_insertAndFindString);
    RUN_TEST(test_insertAndFindInt);
    RUN_TEST(test_integratedART);
    RUN_TEST(test_integratedARTExpansion);

    return UNITY_END();
}