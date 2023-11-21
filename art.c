/**
 * ART.C - Adaptive Radix Tree in C
 * 
 * Copyright (c) 2023, Simone Bellavia <simone.bellavia@live.it>
 * All rights reserved.
 * Released under MIT License. Please refer to LICENSE for details
*/

#include <art.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/**
 * createRootNode
 *
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

/**
 * initializeAdaptiveRadixTree 
 *
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
#ifdef __SSE2__
    Node *findChildSSE(Node16 *node, char byte){
        __m128i key = _mm_set1_epi8(byte);
        __m128i keys = _mm_loadu_si128((__m128i *)(node->keys));
        __m128i cmp = _mm_cmpeq_epi8(key, keys);
        int mask = (1 << (node->count - 1));
        int bitfield = _mm_movemask_epi8(cmp) & mask;
        if (bitfield){
            int index = __builtin_ctz(bitfield);
            return node->children[index];
        } else {
            return NULL;
        }
    }
#endif

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
Node *findChildBinary(Node16 *node, char byte){
    int low = 0;
    int high = node->count - 1;

    while (low <= high){
        int mid = low + (high - low) / 2;
        char midByte = node->keys[mid];

        if (midByte < byte){
            low = mid + 1;
        }
        else if (midByte > byte){
            high = mid - 1;
        }
        else{
            return node->children[mid];
        }
    }

    return NULL;
}

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
Node *findChild(Node *node, char byte){
    if (node->type == NODE4){
        Node4 *node4 = (Node4 *)node;

        for (int i = 0; i < node4->count; i++){
            if(node4->keys[i] == byte){
                return node4->children[i];
            }
        }

        return NULL;
    }

    if(node->type == NODE16){
        Node16 *node16 = (Node16 *)node;
        #ifdef __SSE2__
            return findChildSSE(node16, byte);
        #else
            return findChildBinary(node16, byte);
        #endif
    }

    if(node->type == NODE48){
        Node48 *node48 = (Node48 *)node;

        unsigned char childIndex = node48->keys[byte];

        if(childIndex != EMPTY_KEY){
            return node48->children[childIndex];
        } else {
            return NULL;
        }
    }

    if(node->type == NODE256){
        Node256 *node256 = (Node256 *)node;
        return node256->children[byte];
    }

    return NULL;
}

int getPrefixLength(Node *node) {
    switch (node->type){
        case NODE4:
            return ((Node4 *) node)->prefixLen;
        case NODE16:
            return ((Node16 *) node)->prefixLen;
        case NODE48:
            return ((Node48 *) node)->prefixLen;
        case NODE256:
            return ((Node256 *) node)->prefixLen;
        default:
            return -1;
    }
}

int checkPrefix(Node *node, char *key, int depth){
    int count = 0;

    switch(node->type){
        case NODE4:{
                Node4 *node4 = (Node4 *)node;
                
                for (int i = 0; i < node4->prefixLen; i++){
                    if(node4->prefix[i] == key[depth + i]){
                        count++;
                    } else {
                        return count;
                    }
                }
            }
            break;
        case NODE16:{
                Node16 *node16 = (Node16 *)node;
                
                for (int i = 0; i < node16->prefixLen; i++){
                    if(node16->prefix[i] == key[depth + i]){
                        count++;
                    } else {
                        return count;
                    }
                }
            }
            break;
        case NODE48:{
                Node48 *node48 = (Node48 *)node;
                
                for (int i = 0; i < node48->prefixLen; i++){
                    if(node48->prefix[i] == key[depth + i]){
                        count++;
                    } else {
                        return count;
                    }
                }
            }
            break;
        case NODE256:{
                Node256 *node256 = (Node256 *)node;
                
                for (int i = 0; i < node256->prefixLen; i++){
                    if(node256->prefix[i] == key[depth + i]){
                        count++;
                    } else {
                        return count;
                    }
                }
            }
            break;
        default:
            return -1;
    }
}

Node *search(Node *node, char *key, int depth){
    if (node==NULL){
        return NULL;
    }

    if (node->type == LEAF){
        LeafNode *leafNode = (LeafNode *)node;

        if(strcmp(leafNode->key, key) == 0){
            return (Node *)leafNode;
        } else {
            return NULL;
        }
    }

    int nodePrefixLength = getPrefixLength(node);

    if (checkPrefix(node, key, depth) != nodePrefixLength){
        return NULL;
    }

    depth += nodePrefixLength;

    Node *next=findChild(node, key[depth]);

    return search(next, key, depth+1);
}