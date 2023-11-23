/**
 * ART.C - Adaptive Radix Tree in C
 * 
 * Copyright (c) 2023, Simone Bellavia <simone.bellavia@live.it>
 * All rights reserved.
 * Released under MIT License. Please refer to LICENSE for details
*/

#include "art.h"

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

ART *initializeAdaptiveRadixTree() {
    ART *tree = malloc(sizeof(ART));
    if (!tree) {
        return NULL;
    }

    tree->root = createRootNode();
    tree->size = 0;

    return tree;
}

#ifdef __SSE2__
Node *findChildSSE(Node *genericNode, char byte){
    switch (genericNode->type){
    case NODE4:
    case NODE16:{
        Node16 *node = (Node16 *)genericNode;
        __m128i key = _mm_set1_epi8(byte);
        __m128i keys = _mm_loadu_si128((__m128i *)(node->keys));
        __m128i cmp = _mm_cmpeq_epi8(key, keys);
        int mask = (1 << (node->count - 1));
        int bitfield = _mm_movemask_epi8(cmp) & mask;
        if (bitfield){
            int index = __builtin_ctz(bitfield);
            return node->children[index];
        }
        break;
    }
    case NODE48:
        Node48 *node = (Node48 *)genericNode;
        unsigned char childIndex = node->keys[(unsigned char)byte];
        if (childIndex != EMPTY_KEY) {
            return node->children[childIndex];
        }
        break;
    case NODE256:{
        Node256 *node = (Node256 *)genericNode;
        return node->children[byte];
        break;
    }
    default:
        return NULL;
    }
    return NULL;
}
#endif

Node *findChildBinary(Node *genericNode, char byte)
{
    switch (genericNode->type)
    {
    case NODE4:
    case NODE16:
    {
        Node16 *node = (Node16 *)genericNode;
        int low = 0;
        int high = node->count - 1;

        while (low <= high)
        {
            int mid = low + (high - low) / 2;
            char midByte = node->keys[mid];

            if (midByte < byte)
            {
                low = mid + 1;
            }
            else if (midByte > byte)
            {
                high = mid - 1;
            }
            else
            {
                return node->children[mid];
            }
        }
        break;
    }
    case NODE48:
    {
        Node48 *node = (Node48 *)genericNode;
        unsigned char childIndex = node->keys[(unsigned char)byte];
        if (childIndex < 48){
            return node->children[childIndex];
        } else {
            return NULL;
        }
        break;
    }
    case NODE256:
    {
        Node256 *node = (Node256 *)genericNode;
        return node->children[byte];
        break;
    }
    default:
        return NULL;
    }
    return NULL;
}

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

        if(childIndex < 48){
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
            return INVALID;
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
            return INVALID;
    }

    return count;
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

Node48 *makeNode48() {
    Node48 *node48 = malloc(sizeof(Node48));
    if (!node48) {
        return NULL;
    }

    node48->node.type = NODE48;
    node48->prefixLen = 0;

    memset(node48->keys, EMPTY_KEY, 256);

    for (int i = 0; i < 48; i++) {
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

LeafNode *makeLeafNode(char *key, void *value){
    LeafNode *leafNode = malloc(sizeof(LeafNode));
    if(!leafNode){
        return NULL;
    }

    leafNode->node.type = LEAF;
    leafNode->key = malloc(strlen(key) + 1);
    if(!leafNode->key){
        free(leafNode);
        return NULL;
    }
    strcpy(leafNode->key, key);
    leafNode->value = value;

    return leafNode;
}

int findEmptyIndexForChildren(Node48 *node48){
    for (int i = 0; i < 48; i++){
        if (node48->children[i] == NULL){
            return i;
        }
    }
    
    return INVALID;
}

Node *growFromNode4toNode16(Node *node){
    Node4 *node4 = (Node4 *)node;

    // Create a new Node16
    Node16 *newNode = makeNode16();

    // Copy prefix from node to newNode
    memcpy(newNode->prefix, node4->prefix, node4->prefixLen);
    newNode->prefixLen = node4->prefixLen;

    // Copy each child and key from node4 to newNode
    for (int i = 0; i < node4->count; i++){
        unsigned char keyChar = node4->keys[i];
        newNode->keys[i] = keyChar;
        newNode->children[i] = node4->children[i];
    }
    // Update the children counter in newNode
    newNode->count = node4->count;

    free(node4);
    return (Node *)newNode;
}

Node *growFromNode16toNode48(Node *node){
    Node16 *node16 = (Node16 *)node;

    Node48 *newNode = makeNode48();

    memcpy(newNode->prefix, node16->prefix, node16->prefixLen);
    newNode->prefixLen = node16->prefixLen;

    for (int i = 0; i < node16->count; i++){
        char keyChar = node16->keys[i];
        unsigned char index = (unsigned char)keyChar;

        int childIndex = findEmptyIndexForChildren(newNode);
        newNode->keys[index] = childIndex;
        newNode->children[childIndex] = node16->children[i];
    }

    free(node16);
    return (Node *)newNode;
}

Node *growFromNode48toNode256(Node *node){
    Node48 *node48 = (Node48 *)node;

    Node256 *newNode = makeNode256();

    memcpy(newNode->prefix, node48->prefix, node48->prefixLen);
    newNode->prefixLen = node48->prefixLen;

    int childIndex = 0;

    for (int i = 0; i < 256; i++){
        if (node48->keys[i] != 0)
        {
            childIndex = node48->keys[i];
            newNode->children[i] = node48->children[childIndex];
        }
    }

    free(node48);

    return (Node *)newNode;
}

Node *grow(Node *node){
    switch(node->type){
        case NODE4: {
            growFromNode4toNode16(node);
            break;
        }

        case NODE16: {
            growFromNode16toNode48(node);
            break;
        }
            
        case NODE48: {
            growFromNode48toNode256(node);
            break;
        }

        case NODE256: {
            return INVALID;
            break;
        }

        case LEAF:
            return INVALID;
            break;
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

Node *addChildToNode4(Node *parentNode, char keyChar, Node *childNode){
    Node4 *node = (Node4 *)parentNode;
    if (node->count < 4){
        int position = 0;

        // Find the correct position where to add the new child
        while (position < node->count && node->keys[position] < keyChar){
            position++;
        }

        // Move children and keys to make space for the new child
        for (int i = node->count; i > position; i--){
            node->keys[i] = node->keys[i - 1];
            node->children[i] = node->children[i - 1];
        }

        // Add the new child and his key in the found position
        node->keys[position] = keyChar;
        node->children[position] = childNode;

        node->count++;
    } else {
        grow(parentNode);
        addChild(parentNode, keyChar, childNode);
    }

    return parentNode;
}

Node *addChildToNode16(Node *parentNode, char keyChar, Node *childNode){
    Node16 *node = (Node16 *)parentNode;
    if (node->count < 16){
        int position = 0;

        while (position < node->count && node->keys[position] < keyChar){
            position++;
        }

        for (int i = node->count; i > position; i--){
            node->keys[i] = node->keys[i - 1];
            node->children[i] = node->children[i - 1];
        }

        node->keys[position] = keyChar;
        node->children[position] = childNode;

        node->count++;
    } else {
        parentNode = grow(parentNode);
        addChild(parentNode, keyChar, childNode);
    }

    return parentNode;
}

Node *addChildToNode48(Node *parentNode, char keyChar, Node *childNode) {
    Node48 *node = (Node48 *)parentNode;

    // Checks whether the key byte already has an associated child
    unsigned char index = (unsigned char)keyChar;
    if (node->keys[index] != EMPTY_KEY) {
        // Find an empty index in the child array
        int childIndex = findEmptyIndexForChildren(node);

        // Make sure there is space
        if (childIndex != INVALID) {  
            // Add the new child
            node->keys[index] = childIndex;
            node->children[childIndex] = childNode;
        } else {
            // Full node, need to turn it into a Node256
            parentNode = grow(parentNode);
            addChild(parentNode, keyChar, childNode);
        }
    } else {
        // Update existing child
        int childIndex = node->keys[index];
        node->children[childIndex] = childNode;
    }

    return parentNode;
}

Node *addChildToNode256(Node *parentNode, char keyChar, Node *childNode){
    Node256 *node = (Node256 *)parentNode;
    unsigned char index = (unsigned char)keyChar;
    if (node->children[index] == NULL){
        node->children[index] = childNode;
    }

    return parentNode;
}

Node *addChild(Node *parentNode, char keyChar, Node *childNode){
        switch (parentNode->type){
        case NODE4:{
            addChildToNode4(parentNode, keyChar, childNode);
            break;
        }
        case NODE16:{
            addChildToNode16(parentNode, keyChar, childNode);
            break;
        }
        case NODE48:{
            addChildToNode48(parentNode, keyChar, childNode);
            break;
        }
        case NODE256:{
            addChildToNode256(parentNode, keyChar, childNode);
            break;
        }
        case LEAF:{
            return INVALID;
            break;
        }
    }
}

// Node4 *transformLeafToNode4(Node *leafNode, const char *existingKey, const char *newKey, void *newValue, int depth){
//     Node4 *newNode = makeNode4();
//     if(!newNode){
//         return NULL;
//     }

//     // Check the common prefix between the two keys
//     int prefixLen = 0;
//     while (existingKey[depth + prefixLen] == newKey[depth + prefixLen]) {
//         newNode->prefix[prefixLen] = existingKey[depth + prefixLen];
//         prefixLen++;
//     }
//     newNode->prefixLen = prefixLen;

//     // Add the existing leaf and the new value to Node4
//     char existingKeyChar = existingKey[depth + prefixLen];
//     char newKeyChar = newKey[depth + prefixLen];

//     addChild((Node *)newNode, existingKeyChar, leafNode);
//     addChild((Node *)newNode, newKeyChar, makeLeafNode(newKey, newValue));
// }

// Node *insert(Node *node, char *key, Node *leaf, int depth){
//     if(node == NULL){
//         return leaf;
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
//         addChild(newNode, key[depth], leaf);
//         addChild(newNode, key2[depth], node);
//         // replace(node, newNode)

//         free(key2);
//         return (Node *)newNode;
//     }
// }