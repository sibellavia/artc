/**
 * ART.C - Adaptive Radix Tree in C
 * 
 * Copyright (c) 2023, Simone Bellavia <simone.bellavia@live.it>
 * All rights reserved.
 * Released under MIT License. Please refer to LICENSE for details
*/

#include "art.h"
// #include "../tests/art_integrated_tests.c" // TEMPORARY, TO DELETE
// #include "../tests/art_unit_tests.c" // TEMPORARY, TO DELETE

Node *createRootNode() {
    Node4 *root = calloc(1, sizeof(Node4));
    if (!root) {
        return NULL;
    }

    root->node.type = NODE4;
    root->node.prefixLen = 0;

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

    tree->root = NULL;
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

Node *findChildBinary(Node *genericNode, char byte) {
    switch (genericNode->type) {
        case NODE4: {
            Node4 *node = (Node4 *)genericNode;
            for (int i = 0; i < 4; i++) {
                if (node->keys[i] == byte) {
                    return node->children[i];
                }
            }
            break;
        }
        case NODE16: {
            Node16 *node = (Node16 *)genericNode;
            for (int i = 0; i < 16; i++) {
                if (node->keys[i] == byte) {
                    return node->children[i];
                }
            }
            break;
        }
        case NODE48: {
            Node48 *node = (Node48 *)genericNode;
            unsigned char childIndex = node->keys[(unsigned char)byte];
            if (childIndex != EMPTY_KEY) {
                return node->children[childIndex];
            }
            break;
        }
        case NODE256: {
            Node256 *node = (Node256 *)genericNode;
            return node->children[(unsigned char)byte];
            break;
        }
        default:
            return NULL;
    }
    return NULL;
}

Node *findChild(Node *node, char byte){
    #ifdef __SSE2__
        return findChildSSE(node, byte);
    #else
        return findChildBinary(node, byte);
    #endif
}


int getPrefixLength(Node *node) {
    if (node == NULL) {
        return INVALID;
    }
    return node->prefixLen;
}

int checkPrefix(Node *node, const char *key, int depth) {
    int count = 0;
    int keyLength = strlen(key);
    int maxLength = MIN(node->prefixLen, keyLength - depth);
    
    for (int i = 0; i < maxLength; i++) {
        if (node->prefix[i] != key[depth + i]) {
            break;
        }
        count++;
    }

    return count;
}

Node4 *makeNode4(){
    Node4 *node4 = malloc(sizeof(Node4));
    if(!node4){
        return NULL;
    }

    node4->node.type = NODE4;
    node4->node.prefixLen = 0;
    memset(node4->children, 0, sizeof(node4->children));
    memset(node4->keys, EMPTY_KEY, 4);

    return node4;
}

Node16 *makeNode16(){
    Node16 *node16 = malloc(sizeof(Node16));
    if(!node16){
        return NULL;
    }

    node16->node.type = NODE16;
    node16->node.prefixLen = 0;
    memset(node16->node.prefix, 0, MAX_PREFIX_LENGTH);
    memset(node16->children, 0, sizeof(node16->children));
    memset(node16->keys, EMPTY_KEY, 16);

    return node16;
}

Node48 *makeNode48() {
    Node48 *node48 = malloc(sizeof(Node48));
    if (!node48) {
        return NULL;
    }
    node48->node.type = NODE48;
    node48->node.prefixLen = 0;
    memset(node48->node.prefix, 0, MAX_PREFIX_LENGTH);
    memset(node48->keys, EMPTY_KEY, 256);
    memset(node48->children, 0, sizeof(node48->children));

    return node48;
}

Node256 *makeNode256(){
    Node256 *node256 = malloc(sizeof(Node256));
    if(!node256){
        return NULL;
    }

    node256->node.type = NODE256;
    node256->node.prefixLen = 0;
    memset(node256->node.prefix, 0, MAX_PREFIX_LENGTH);
    memset(node256->children, 0, sizeof(node256->children));

    return node256;
}

LeafNode *makeLeafNode(const char *key, const void *value, size_t valueLength){
    size_t keyLength = strlen(key);
    
    // Allocazione della memoria per LeafNode con spazio aggiuntivo per la chiave
    LeafNode *leafNode = malloc(sizeof(LeafNode) + keyLength + 1);
    if(!leafNode){
        return NULL;
    }

    leafNode->node.type = LEAF;
    leafNode->node.prefixLen = 0;
    memset(leafNode->node.prefix, 0, MAX_PREFIX_LENGTH);
    
    // Copia della chiave
    memcpy(leafNode->key, key, keyLength + 1);

    // Allocazione della memoria per il valore
    leafNode->value = malloc(valueLength);
    if(!leafNode->value){
        free(leafNode);
        return NULL;
    }

    // Copia del valore
    memcpy(leafNode->value, value, valueLength);    

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

Node *growFromNode4toNode16(Node **nodePtr) {
    if (nodePtr == NULL || *nodePtr == NULL) {
        return NULL;
    }

    Node4 *oldNode = (Node4 *)*nodePtr;
    Node16 *newNode = makeNode16();

    if (newNode == NULL) {
        return NULL;
    }

    memcpy(newNode->node.prefix, oldNode->node.prefix, oldNode->node.prefixLen);
    newNode->node.prefixLen = oldNode->node.prefixLen;

    // Copy each child and key from oldNode to newNode
    for (int i = 0; i < 4; i++) {
        newNode->keys[i] = oldNode->keys[i];
        newNode->children[i] = oldNode->children[i];
    }

    free(oldNode);

    *nodePtr = (Node *)newNode;

    return (Node *)newNode;
}

int findNextAvailableChild(Node **children) {
    for (int i = 0; i < 48; i++) {
        if (children[i] == NULL) {
            return i;
        }
    }
    return INVALID; // No space available
}

Node *growFromNode16toNode48(Node **nodePtr) {
    if (nodePtr == NULL || *nodePtr == NULL) {
        return NULL;
    }

    Node16 *oldNode = (Node16 *)*nodePtr;
    Node48 *newNode = makeNode48();

    if (newNode == NULL) {
        return NULL;
    }

    
    memcpy(newNode->node.prefix, oldNode->node.prefix, oldNode->node.prefixLen);
    newNode->node.prefixLen = oldNode->node.prefixLen;

    
    memset(newNode->keys, EMPTY_KEY, sizeof(newNode->keys));

    for (int i = 0; i < 16; i++) {
        if (oldNode->children[i] != NULL) {
            uint8_t keyChar = oldNode->keys[i];
            int childIndex = findNextAvailableChild(newNode->children);

            if (childIndex == INVALID) {
                freeNode((Node *) newNode);
                return NULL;
            }

            newNode->keys[keyChar] = childIndex + 1;
            newNode->children[childIndex] = oldNode->children[i];
        }
    }

    freeNode((Node *) oldNode);
    *nodePtr = (Node *)newNode;
    return (Node *)newNode;
}

Node *growFromNode48toNode256(Node **nodePtr){
    if (nodePtr == NULL || *nodePtr == NULL) {
        return NULL;
    }

    Node48 *oldNode = (Node48 *)*nodePtr;
    Node256 *newNode = makeNode256();

    if (newNode == NULL) {
        return NULL;
    }

    memcpy(newNode->node.prefix, oldNode->node.prefix, oldNode->node.prefixLen);
    newNode->node.prefixLen = oldNode->node.prefixLen;

    for (int i = 0; i < 256; i++) {
        unsigned char childIndex = oldNode->keys[i];
        if (childIndex != EMPTY_KEY) {
            newNode->children[i] = oldNode->children[childIndex];
        }
    }

    free(oldNode);

    return (Node *)newNode;
}

Node *grow(Node **node){
    if(node == NULL || *node == NULL){
        return NULL;
    }

    switch((*node)->type){
        case NODE4: {
            return growFromNode4toNode16(node);
        }

        case NODE16: {
            return growFromNode16toNode48(node);
        }
            
        case NODE48: {
            return growFromNode48toNode256(node);
        }

        case NODE256: {
            // Not possible to grow
            return NULL;
        }

        case LEAF:{
            // A LeafNode cannot grow in this context
            return NULL;
        }

        default: {
            // Error
            return NULL;
        }
    }
}

Node *addChildToNode4(Node *parentNode, char keyChar, Node *childNode){
    if (parentNode == NULL || childNode == NULL){
        return NULL;
    }

    Node4 *node = (Node4 *)parentNode;
    int count = 0;
    for (int i = 0; i < 4; i++){
        if (node->keys[i] != EMPTY_KEY){
            count++;
        }
    }

    if (count >= 4){
        parentNode = grow(&parentNode);
        return addChild(parentNode, keyChar, childNode);
    }

    // Find the position to insert the new key
    int position = 0;
    while (position < count && node->keys[position] < keyChar){
        position++;
    }

    // Shift the keys and children right by one
    for (int i = count; i > position; i--){
        node->keys[i] = node->keys[i - 1];
        node->children[i] = node->children[i - 1];
    }

    // Insert the new key and child
    node->keys[position] = keyChar;
    node->children[position] = childNode;

    return parentNode;
}

Node *addChildToNode16(Node *parentNode, char keyChar, Node *childNode){
    if (parentNode == NULL || childNode == NULL){
        return NULL;
    }

    Node16 *node = (Node16 *)parentNode;

    // Calculate the effective number of children in node
    int count = 0;
    for (int i = 0; i < 16; i++){
        if (node->keys[i] != EMPTY_KEY){
            count++;
        }
    }

    if (count >= 16){
        parentNode = grow(&parentNode);
        return addChild(parentNode, keyChar, childNode);
    }

    // Find the position to insert the new key
    int position = 0;
    while (position < count && node->keys[position] < keyChar){
        position++;
    }

    // Shift the keys and children right by one
    for (int i = count; i > position; i--){
        node->keys[i] = node->keys[i - 1];
        node->children[i] = node->children[i - 1];
    }

    // Insert the new key and child
    node->keys[position] = keyChar;
    node->children[position] = childNode;

    return parentNode;
}

Node *addChildToNode48(Node *parentNode, char keyChar, Node *childNode){
    if (parentNode == NULL || childNode == NULL){
        return NULL;
    }

    Node48 *node = (Node48 *)parentNode;

    // Check whether we already have a child with this key
    unsigned char index = (unsigned char)keyChar;
    if (node->keys[index] != EMPTY_KEY){
        node->children[node->keys[index]] = childNode;
        return parentNode;
    }

    // Find an empty position
    int position = findEmptyIndexForChildren(node);
    if (position == INVALID){
        parentNode = grow(&parentNode);
        return addChild(parentNode, keyChar, childNode);
    }

    // Insert the child into the node
    node->keys[index] = position;
    node->children[position] = childNode;

    return parentNode;
}

Node *addChildToNode256(Node *parentNode, char keyChar, Node *childNode){
    if (parentNode == NULL || childNode == NULL){
        return NULL;
    }

    Node256 *node = (Node256 *)parentNode;
    node->children[(unsigned char)keyChar] = childNode;

    return parentNode;
}

Node *addChild(Node *parentNode, char keyChar, Node *childNode){
        if(parentNode == NULL || childNode == NULL){
            return NULL;
        }
        
        switch (parentNode->type){
        case NODE4:{
            return addChildToNode4(parentNode, keyChar, childNode);
        }
        case NODE16:{
            return addChildToNode16(parentNode, keyChar, childNode);
        }
        case NODE48:{
            return addChildToNode48(parentNode, keyChar, childNode);
        }
        case NODE256:{
            return addChildToNode256(parentNode, keyChar, childNode);
        }
        case LEAF:{
            // A LeafNode cannot have children
            return NULL;
        }
    }
}

Node4 *transformLeafToNode4(Node *leafNode, const char *existingKey, const char *newKey, void *newValue, size_t newValueLength, int depth){
   if (leafNode == NULL || existingKey == NULL || newKey == NULL || newValue == NULL){
       return NULL;
   }

    Node4 *newNode = makeNode4();
    if (newNode == NULL){
        return NULL;
    }

    // Set the prefix and the prefix length of new Node4
    int prefixLen = 0;
    while (existingKey[depth + prefixLen] == newKey[depth + prefixLen] && 
           existingKey[depth + prefixLen] != '\0'){
        newNode->node.prefix[prefixLen] = existingKey[depth + prefixLen];
        prefixLen++;
        if (prefixLen >= MAX_PREFIX_LENGTH){
            break; // We cannot have a prefix longer than 10 bytes
        }
    }
    newNode->node.prefixLen = prefixLen;

    // Add existing leaf node and the new value to Node4
    LeafNode *newLeafNode = makeLeafNode(newKey + depth + prefixLen, newValue, newValueLength);
    if (newLeafNode == NULL){
        free(newNode);
        return NULL;
    }

    char existingKeyChar = existingKey[depth + prefixLen];
    char newKeyChar = newKey[depth + prefixLen];

    addChild((Node *)newNode, existingKeyChar, leafNode);
    addChild((Node *)newNode, newKeyChar, (Node *)newLeafNode);

    return newNode;
}

bool isNodeFull(Node *node) {
    if (node == NULL) {
        return false;
    }

    switch (node->type) {
        case NODE4: {
            Node4 *node4 = (Node4 *)node;
            for (int i = 0; i < 4; i++) {
                if (node4->keys[i] == EMPTY_KEY) {
                    return false;
                }
            }
            return true;
        }
        case NODE16: {
            Node16 *node16 = (Node16 *)node;
            for (int i = 0; i < 16; i++) {
                if (node16->keys[i] == EMPTY_KEY) {
                    return false;
                }
            }
            return true;
        }
        case NODE48: {
            Node48 *node48 = (Node48 *)node;
            for (int i = 0; i < 256; i++) {
                if (node48->keys[i] == EMPTY_KEY) {
                    return false;
                }
            }
            return true;
        }
        case NODE256: {
            Node256 *node256 = (Node256 *)node;
            for (int i = 0; i < 256; i++) {
                if (node256->children[i] == NULL) {
                    return false;
                }
            }
            return true;
        }
        default:
            return false;
    }
}

void setPrefix(Node *node, const char *prefix, int prefixLen) {
    if (prefixLen > MAX_PREFIX_LENGTH) {
        prefixLen = MAX_PREFIX_LENGTH;
    }

    memcpy(node->prefix, prefix, prefixLen);

    if (prefixLen < MAX_PREFIX_LENGTH) {
        node->prefix[prefixLen] = '\0';
    }

    node->prefixLen = prefixLen;
}

Node *insert(Node **root, const char *key, void *value, size_t valueLength, int depth){
    if(*root == NULL){
        *root = (Node *)makeLeafNode(key, value, valueLength);
        return *root;
    }

    Node *node = *root;
    Node **parentPointer = root;

    while (node != NULL){
        
        if (node->type == LEAF){
            // Compare current leaf's key with the key we want to insert
            LeafNode *leafNode = (LeafNode *)node;
            if (strcmp((const char *)leafNode->key, key) == 0){
                // La chiave esiste già, sostituisci il valore o restituisci il nodo
                // leafNode->value = value; // Opzionale, a seconda del comportamento desiderato. Devo decidere.
                
                return node;
            }
            else{
                // Calcola il prefisso comune tra la chiave del leafNode e la nuova chiave
                int commonPrefixLength = 0;
                while (leafNode->key[commonPrefixLength] == key[depth + commonPrefixLength]){
                    commonPrefixLength++;
                    if (commonPrefixLength >= MAX_PREFIX_LENGTH){
                        break; // Non possiamo avere un prefisso più lungo di 10 byte
                    }
                }

                // Crea un nuovo Node4 e imposta il prefisso
                Node4 *newNode4 = makeNode4();
                if (newNode4 == NULL){
                    return NULL;
                }

                setPrefix((Node *)newNode4, key + depth, commonPrefixLength);

                // Aggiungi il leafNode esistente e il nuovo valore al nuovo Node4
                addChildToNode4((Node *)newNode4, leafNode->key[depth + commonPrefixLength], node);
                addChildToNode4((Node *)newNode4, key[depth + commonPrefixLength], (Node *)makeLeafNode(key, value, valueLength));

                *parentPointer = (Node *)newNode4;
                return (Node *)newNode4;
            }
        }

        // Here we handle the internal nodes
        if(isNodeFull(node)){
            // Node is full, we have to grow it
            Node *grownNode = grow(&node);
            if (!grownNode){
                return NULL;
            }

            // Update the parent pointer to the new node
            *parentPointer = grownNode;
            node = grownNode;
        } else {
            addChild(node, key[depth], (Node *)makeLeafNode(key, value, valueLength));
            return *root;
        }

    }

    return *root;
}

typedef void (*FreeValueFunc)(void *);

void freeNode(Node *node) {
    if (node == NULL) {
        return;
    }

    switch (node->type) {
        case NODE4: {
            Node4 *node4 = (Node4 *)node;
            for (int i = 0; i < 4; i++) {
                if (node4->keys[i] != EMPTY_KEY) {
                    freeNode(node4->children[i]);
                }
            }
            break;
        }
        case NODE16: {
            Node16 *node16 = (Node16 *)node;
            for (int i = 0; i < 16; i++) {
                if (node16->keys[i] != EMPTY_KEY) {
                    freeNode(node16->children[i]);
                }
            }
            break;
        }
        case NODE48: {
            Node48 *node48 = (Node48 *)node;
            for (int i = 0; i < 256; i++) {
                if (node48->keys[i] != EMPTY_KEY) {
                    freeNode(node48->children[i]);
                }
            }
            break;
        }
        case NODE256: {
            Node256 *node256 = (Node256 *)node;
            for (int i = 0; i < 256; i++) {
                if (node256->children[i] != NULL) {
                    freeNode(node256->children[i]);
                }
            }
            break;
        }
        case LEAF: {
            LeafNode *leafNode = (LeafNode *)node;
            
            free(leafNode->value);
            break;
        }
    }

    free(node);
}



void freeART(ART *art) {
    if (art != NULL) {
        freeNode(art->root);
        free(art);
    }
}

// int main(void){
//     UNITY_BEGIN();

//     RUN_TEST(test_Node48KeysPopulation);
//     // RUN_TEST(test_growNode48ToNode256);

//     return UNITY_END();
// }