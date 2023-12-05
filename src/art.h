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
#define EMPTY_KEY 0xFF
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