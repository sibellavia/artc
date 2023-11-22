/**
 * ART.C - Adaptive Radix Tree in C
 * 
 * Copyright (c) 2023, Simone Bellavia <simone.bellavia@live.it>
 * All rights reserved.
 * Released under MIT License. Please refer to LICENSE for details
*/

#include "art.h"
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

Node *doesNodeHaveChild(Node *node){
    switch(node->type){
        case NODE4:{
                Node4 *node4 = (Node4 *)node;
                
                for (int i = 0; i < 4; i++){
                    if (node4->children[i] != NULL){
                        return node4->children[i];
                    }
                }

                return NULL;
            }
            break;
        case NODE16:{
                Node16 *node16 = (Node16 *)node;

                for (int i = 0; i < 16; i++){
                    if (node16->children[i] != NULL){
                        return node16->children[i];
                    }
                }

                return NULL;
            }
            break;
        case NODE48:{
                Node48 *node48 = (Node48 *)node;

                for (int i = 0; i < 48; i++){
                    if (node48->children[i] != NULL){
                        return node48->children[i];
                    }
                }

                return NULL;
            }

            break;
        case NODE256:{
                Node256 *node256 = (Node256 *)node;

                for (int i = 0; i < 256; i++){
                    return node256->children[i];
                }

                return NULL;
            }
            break;
        default:
            return NULL;
    }
}

Node4 *makeNode4(){
    Node4 *node4 = malloc(sizeof(Node4));
    if(!node4){
        return NULL;
    }

    node4->node.type = NODE4;
    node4->prefixLen = 0;

    for (int i = 0; i < 4; i++){
        node4->keys[i] = 0;
        node4->children[i] = NULL;
    }

    node4->count= 0;

    return node4;
}

Node16 *makeNode16(){
    Node16 *node16 = malloc(sizeof(Node16));
    if(!node16){
        return NULL;
    }

    node16->node.type = NODE16;
    node16->prefixLen = 0;

    for (int i = 0; i < 16; i++){
        node16->keys[i] = 0;
        node16->children[i] = NULL;
    }

    node16->count= 0;

    return node16;
}

Node48 *makeNode48(){
    Node48 *node48 = malloc(sizeof(Node48));
    if(!node48){
        return NULL;
    }

    node48->node.type = NODE4;
    node48->prefixLen = 0;

    memset(node48->keys, 0, 256);

    for (int i = 0; i < 48; i++){
        node48->children[i] = NULL;
    }

    return node48;
}

Node256 *makeNode256(){
    Node256 *node256 = malloc(sizeof(Node256));
    if(!node256){
        return NULL;
    }

    node256->node.type = NODE256;
    node256->prefixLen = 0;

    for (int i = 0; i < 256; i++){
        node256->children[i] = NULL;
    }

    return node256;
}

int findEmptyIndexForChildren(Node48 *node48){
    for (int i = 0; i < 48; i++){
        if (node48->children[i] == NULL){
            return i;
        }
    }
    
    return -1;
}

Node *grow(Node *node){
    switch(node->type){
        case NODE4: {
            Node4 *node4 = (Node4 *)node;

            // Create a new Node16
            Node16 *newNode = makeNode16();
            
            // Copy prefix from node to newNode
            memcpy(newNode->prefix, node4->prefix, node4->prefixLen);
            newNode->prefixLen = node4->prefixLen;

            // Copy each child and key from node4 to newNode
            for(int i = 0; i < node4->count; i++){
                unsigned char keyChar = node4->keys[i];
                newNode->keys[i] = keyChar;
                newNode->children[i] = node4->children[i];
            }

            // Update the children counter in newNode
            newNode->count = node4->count;

            free(node4);
            return (Node *)newNode;

            break;
        }

        case NODE16: {
            Node16 *node16 = (Node16 *)node;

            // Create a new Node48
            Node48 *newNode = makeNode48();
            
            // Copy prefix from node to newNode
            memcpy(newNode->prefix, node16->prefix, node16->prefixLen);
            newNode->prefixLen = node16->prefixLen;

            // Copy each child and key from node16 to newNode
            for(int i = 0; i < node16->count; i++){
                char keyChar = node16->keys[i];
                unsigned char index = (unsigned char)keyChar;

                int childIndex = findEmptyIndexForChildren(newNode);
                newNode->keys[index] = childIndex;
                newNode->children[childIndex] = node16->children[i];

                
            }

            free(node16);
            return (Node *)newNode;

            break;
        }
            
        case NODE48: {
            Node48 *node48 = (Node48 *)node;
            
            Node256 *newNode = makeNode256();

            memcpy(newNode->prefix, node48->prefix, node48->prefixLen);
            newNode->prefixLen = node48->prefixLen;

            int childIndex = 0;

            for(int i = 0; i < 256; i++){
                if(node48->keys[i] != 0){
                    childIndex = node48->keys[i];
                    newNode->children[i] = node48->children[childIndex];
                }
            }

            free(node48);

            return (Node *)newNode;
            break;
        }

        // case LEAF:
            // if parentNode is a leaf, transform it into a Node4
    }
}

char *loadKey(Node *node){
    if(node->type == LEAF){
        LeafNode *leafNode = (LeafNode *)node;
        return strdup(leafNode->key);
    } else {
        return NULL;
    }
}

// Node *addChild(Node *parentNode, char *keyChar, Node *childNode){
//     switch(parentNode->type){
//         case NODE4:
//             // if parentNode has less than 4 child
//                 // add childNode to parentNode based on keyChar
//                 // update the parentNode structure
//             // else
//                 // grow parentNode to 16 and call addChild again
//         case NODE16:
//             // if parentNode has less than 16 child
//                 // add childNode to parentNode based on keyChar
//                 // update the parentNode structure
//             // else
//                 // grow parentNode to 48 and call addChild again
//         case NODE48:
//             // if parentNode has less than 48 child
//                 // add childNode to parentNode based on keyChar
//                 // update the parentNode structure
//             // else
//                 // grow parentNode to 256 and call addChild again

//         case NODE256:
//             // add childNode to parentNode
//         case LEAF:
//             // if parentNode is a leaf, transform it into a Node4
//     }
// }

// Node *insert(Node *node, char *key, Node *leaf, int depth){
//     if(node == NULL){
//         node = leaf;
//         return node;
//     }

//     if(node->type == LEAF){
//         Node4 *newNode = makeNode4();
//         char *key2 = loadKey(node);
//         int count = 0;
//         for(int i = depth; key[i] == key2[i]; i=i+1){
//             newNode->prefix[i-depth] = key[i];
//             count = i;
//         }
//         newNode->prefixLen = count-depth;
//         depth += newNode->prefixLen;
//         // addChild(newNode, key[depth], leaf)
//         // addChild(newNode, key2[depth], node)
//         // replace(node, newNode)

//         free(key2);
//         return;
//     }
// }