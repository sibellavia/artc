#include <stdio.h>
#include <stdlib.h>

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
 * Node is the basis for all node types and will contain common 
 * information, such as node type.
 */
typedef struct Node {
    NodeType type;
} Node;

/*
 * Node4, Node16, Node48 and Node256 *
 * Each type of internal node have a specific structure that 
 * extends Node. The main difference between these nodes is the 
 * number of children they can contain.
*/
typedef struct {
    Node node;
    char keys[4];
    Node *children[4];
} Node4;

typedef struct {
    Node node;
    char keys[16];
    Node *children[16];
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

/*
 * LeafNode *
 * Node is the basis for all node types and will contain common 
 * information, such as node type.
 */
typedef struct {
    Node node;
    char *key;
    void *value;
} LeafNode;