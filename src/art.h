/**
 * ART.C - Adaptive Radix Tree in C
 * 
 * Copyright (c) 2023, Simone Bellavia <simone.bellavia@live.it>
 * All rights reserved.
 * Released under MIT License. Please refer to LICENSE for details
*/

#ifndef ART_H
#define ART_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifdef __x86_64__
    #include <emmintrin.h>
#endif

#define MAX_PREFIX_LENGTH 32
#define EMPTY_KEY 0xFF
#define INVALID -1

/*** DATA STRUCTURES ***/

/**
 * NodeType
 * 
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

/**
 * Node
 * 
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
    char prefix[MAX_PREFIX_LENGTH];
    int prefixLen;
} Node;

/**
 * Node4, Node16, Node48 and Node256
 *
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
    char keys[4];
    Node *children[4];
    int count;
} Node4;

typedef struct {
    Node node;
    char keys[16];
    Node *children[16];
    int count;
} Node16;

typedef struct {
    Node node;
    unsigned char keys[256];
    Node *children[48];
} Node48;

typedef struct {
    Node node;
    Node *children[256];
} Node256;

/**
 * LeafNode
 *
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

/**
 * createRootNode
 *
 * Creates and initializes a new Node4 type root node for an Adaptive Radix Tree (ART).
 * Assigns an initial value of EMPTY_KEY to all elements in the keys array
 * and sets all child pointers to NULL, indicating that the node is initially empty.
 * Returns a pointer to the created root node, or NULL if memory allocation fails.
 */
Node *createRootNode();

/**
 * initializeAdaptiveRadixTree 
 *
 * Initializes a new Adaptive Radix Tree (ART).
 * Creates a new ART structure and sets its root node
 * by calling the createRootNode function. Also initializes the size
 * of the tree to 0, indicating that the tree initially has no elements.
 * Returns a pointer to the initialized ART, or NULL if memory allocation fails.
 */
ART *initializeAdaptiveRadixTree();

/**
 * findChildSSE - Finds a child node in a Node16 using SSE instructions.
 * 
 * Utilizes SSE (Streaming SIMD Extensions) to perform an efficient,
 * parallel comparison of a given byte against all keys in a Node16.
 * This function is optimized for architectures that support SSE and provides
 * a significant speed-up by processing multiple bytes in parallel.
 *
 * @param node A pointer to the Node16 to search in.
 * @param byte The byte (key) to find the corresponding child for.
 * @return A pointer to the found child node, or NULL if no match is found.
 */
Node *findChildSSE(Node *genericNode, char byte);

/**
 * findChildBinary - Finds a child node in a Node16 using binary search.
 * 
 * Implements a binary search algorithm to find a specific byte in the
 * keys array of a Node16. This method is used as a portable alternative
 * to SSE-based search, suitable for platforms that do not support SSE.
 * Binary search offers better performance than a linear search, especially
 * when the number of keys is relatively large.
 *
 * @param node A pointer to the Node16 to search in.
 * @param byte The byte (key) to find the corresponding child for.
 * @return A pointer to the found child node, or NULL if no match is found.
 */
Node *findChildBinary(Node *genericNode, char byte);

/**
 * findChild - Finds a child node in an ART node based on the given byte (key).
 * 
 * This function handles the retrieval of a child node from different types
 * of ART nodes (Node4, Node16, Node48, Node256) based on the provided byte.
 * The specific search algorithm or method used depends on the node type:
 *   - For Node4, a linear search is used.
 *   - For Node16, it utilizes either SSE-based search or binary search,
 *     depending on the platform's support for SSE.
 *   - For Node48, the function performs a lookup using an index array.
 *   - For Node256, it directly accesses the child based on the byte value.
 *
 * This approach ensures that the search is as efficient as possible given
 * the characteristics of each node type.
 *
 * @param node A pointer to the ART node to search in.
 * @param byte The byte (key) to find the corresponding child for.
 * @return A pointer to the found child node, or NULL if no match is found.
 */
Node *findChild(Node *node, char byte);

int getPrefixLength(Node *node);

int checkPrefix(Node *node, char *key, int depth);

Node *search(Node *node, char *key, int depth);

Node *doesNodeHaveChild(Node *node);

Node4 *makeNode4();
Node16 *makeNode16();
Node48 *makeNode48();
Node256 *makeNode256();

LeafNode *makeLeafNode(char *key, void *value);

int findEmptyIndexForChildren(Node48 *node48);

Node *growFromNode4toNode16(Node **nodePtr);
Node *growFromNode16toNode48(Node **nodePtr);
Node *growFromNode48toNode256(Node *node);
Node *grow(Node **node);

char *loadKey(Node *node);

Node *addChildToNode4(Node *parentNode, char keyChar, Node *childNode);
Node *addChildToNode16(Node *parentNode, char keyChar, Node *childNode);
Node *addChildToNode48(Node *parentNode, char keyChar, Node *childNode);
Node *addChildToNode256(Node *parentNode, char keyChar, Node *childNode);
Node *addChild(Node *parentNode, char keyChar, Node *childNode);

Node4 *transformLeafToNode4(Node *leafNode, const char *existingKey, const char *newKey, void *newValue, int depth);

Node *insert(Node **root, char *key, void *value, int depth);

void freeNode(Node *node);

void freeART(ART *art);

void setPrefix(Node *node, const char *prefix, int prefixLen);

#endif // ART_H