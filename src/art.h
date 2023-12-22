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
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifdef __x86_64__
    #include <emmintrin.h>
#endif

#define MAX_PREFIX_LENGTH 32
#define EMPTY_KEY '\0'
#define INVALID -1
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/*** DATA STRUCTURES ***/

typedef enum {
    NODE4,
    NODE16,
    NODE48,
    NODE256,
    LEAF
} NodeType;

typedef struct Node {
    NodeType type;
    uint8_t prefix[MAX_PREFIX_LENGTH];
    uint32_t prefixLen;
} Node;

typedef struct {
    Node node;
    uint8_t keys[4];
    Node *children[4];
} Node4;

typedef struct {
    Node node;
    uint8_t keys[16];
    Node *children[16];
} Node16;

typedef struct {
    Node node;
    uint8_t keys[256];
    Node *children[48];
} Node48;

typedef struct {
    Node node;
    Node *children[256];
} Node256;

typedef struct {
    Node node;
    void *value;
    uint8_t key[];
} LeafNode;

typedef struct {
    Node *root;
    size_t size;
} ART;

/*** FUNCTIONS ***/

Node *createRootNode();
ART *initializeAdaptiveRadixTree();

Node *findChildSSE(Node *genericNode, char byte);
Node *findChildBinary(Node *genericNode, char byte);
Node *findChild(Node *node, char byte);

int getPrefixLength(Node *node);
int checkPrefix(Node *node, const char *key, int depth);

Node4 *makeNode4();
Node16 *makeNode16();
Node48 *makeNode48();
Node256 *makeNode256();

LeafNode *makeLeafNode(const char *key, const void *value, size_t keyLength, size_t valueLength);

int findEmptyIndexForChildren(Node48 *node48);

Node *growFromNode4toNode16(Node **nodePtr);
int findNextAvailableChild(Node **children);
int findUnusedKey(uint8_t *keys);
Node *growFromNode16toNode48(Node **nodePtr);
Node *growFromNode48toNode256(Node **nodePtr);
Node *grow(Node **node);

Node *addChildToNode4(Node *parentNode, const void *keyPart, Node *childNode);
Node *addChildToNode16(Node *parentNode, const void *keyPart, Node *childNode);
Node *addChildToNode48(Node *parentNode, const void *keyPart, Node *childNode);
Node *addChildToNode256(Node *parentNode, const void *keyPart, Node *childNode);
Node *addChild(Node *parentNode, const void *keyPart, Node *childNode);

Node4 *transformLeafToNode4(Node *leafNode, const char *existingKey, size_t existingKeyLength, const char *newKey, void *newValue, size_t newKeyLength, size_t newValueLength, int depth);

bool isNodeFull(Node *node);

void setPrefix(Node *node, const char *prefix, int prefixLen);

typedef int (*compare_func)(const void *a, const void *b, size_t size);
int compare_ints(const void *a, const void *b, size_t size);
int compare_strings(const void *a, const void *b, size_t size);
Node *insert(Node **root, const void *key, size_t keyLength, void *value, size_t valueLength, int depth, compare_func cmp);
Node *insertInt(Node **root, int key, void *value, size_t valueLength);
Node *insertString(Node **root, const char *key, void *value, size_t valueLength);

typedef void (*FreeValueFunc)(void *);
void freeNode(Node *node);
void freeART(ART *art);

#endif // ART_H