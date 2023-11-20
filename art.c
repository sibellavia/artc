#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_PREFIX_LENGTH 32
#define EMPTY_KEY '\255' // A value that isn't a valid ASCII character

/*** DATA STRUCTURES ***/

/* 
 * NodeType *
 * To define the various types of nodes that an ART can have. 
 * These types can vary in the number of children they can contain.
 */
typedef enum {
    NODE4,
    NODE16,
    NODE48,
    NODE256,
    LEAF
} NodeType;

/*
 * Node *
 * The Node structure acts as the basis for all node types in the ART. Its 
 * main function is to identify the specific node type within the tree.
 * The field 'type' field, of type NodeType (which is the previous declared enum), 
 * indicates the specific type of the node, such as NODE4, NODE16, NODE48, NODE256, 
 * or LEAF. This information is crucial in determining how to handle the node in 
 * various operations such as insert, search, and delete. We will use the type field 
 * to determine the node type at runtime and explicitly cast to the appropriate 
 * derived structure to access its specific fields.
 */
typedef struct Node {
    NodeType type;
} Node;

/*
 * Node4, Node16, Node48 and Node256 *
 * Each type of internal node have a specific structure that 
 * extends Node. The main difference between these nodes is the 
 * number of children they can contain.
 * 
 * The 'prefix' array in Node4 is used to store the common prefix of 
 * all keys passing through this node. In an ART, the prefix is a part 
 * of the key that is common to all children of that node. Example: If 
 * you have keys like "apple," "appetite," and "application" in a node, 
 * the common prefix might be "app."
 * 
 * 'prefixLen' indicates the actual length of the prefix stored in the prefix 
 * array. This value is important because not all prefixes will use the full 
 * maximum length (MAX_PREFIX_LENGTH). Example: If the prefix is "app", prefixLen 
 * will be 3, although MAX_PREFIX_LENGTH could be much greater.
 * 
 * 'keys' is an array that contains the parts of the keys that differentiate the children 
 * in this node. Each element in keys corresponds to a specific child in children.
 * 
 * 'children' is an array of pointers to Node, representing the children of this node. 
 * Each element in children is a child that corresponds to a key in keys.
 * 
 * The association between 'keys' and 'children' is at the heart of ART's functionality.
*/
typedef struct {
    Node node;
    char prefix[MAX_PREFIX_LENGTH];
    int prefixLen;
    char keys[4];
    Node *children[4];
} Node4;

typedef struct {
    Node node;
    char prefix[MAX_PREFIX_LENGTH];
    int prefixLen;
    char keys[16];
    Node *children[16];
} Node16;

typedef struct {
    Node node;
    char prefix[MAX_PREFIX_LENGTH];
    int prefixLen;
    unsigned char keys[256];
    Node *children[48];
} Node48;

typedef struct {
    Node node;
    char prefix[MAX_PREFIX_LENGTH];
    int prefixLen;
    Node *children[256];
} Node256;

/*
 * LeafNode *
 * The LeafNode is the type of node that actually contains 
 * the value (or data) associated with the key.
 */
typedef struct {
    Node node;
    char *key;
    void *value;
} LeafNode;

/**
 * ART
 * 
*/
typedef struct {
    Node *root;
    size_t size;
} ART;

/*** FUNCTIONS ***/

/*
 * createRootNode *
 * Creates and initializes a new Node4 type root node for an Adaptive Radix Tree (ART).
 * Assigns an initial value of EMPTY_KEY to all elements in the keys array
 * and sets all child pointers to NULL, indicating that the node is initially empty.
 * Returns a pointer to the created root node, or NULL if memory allocation fails.
 */
Node *createRootNode() {
    Node4 *root = calloc(1, sizeof(Node4));
    if (!root) {
        return NULL;
    }

    root->node.type = NODE4;
    root->prefixLen = 0;

    for (int i = 0; i < 4; i++) {
        root->keys[i] = EMPTY_KEY;
    }

    return (Node *)root;
}

/*
 * initializeAdaptiveRadixTree *
 * Initializes a new Adaptive Radix Tree (ART).
 * Creates a new ART structure and sets its root node
 * by calling the createRootNode function. Also initializes the size
 * of the tree to 0, indicating that the tree initially has no elements.
 * Returns a pointer to the initialized ART, or NULL if memory allocation fails.
 */
ART *initializeAdaptiveRadixTree() {
    ART *tree = malloc(sizeof(ART));
    if (!tree) {
        return NULL;
    }

    tree->root = createRootNode();
    tree->size = 0;

    return tree;
}