/**
 * ART.C - Adaptive Radix Tree in C
 * 
 * Copyright (c) 2023, Simone Bellavia <simone.bellavia@live.it>
 * All rights reserved.
 * Released under MIT License. Please refer to LICENSE for details
*/

#include "art.h"

// #include "../tests/art_integrated_tests.c" // TEMPORARY, TO DELETE

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
    #ifdef __SSE2__
        return findChildSSE(node, byte);
    #else
        return findChildBinary(node, byte);
    #endif
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

    leafNode->value = malloc(strlen(value) + 1);
    if (!leafNode->value)
    {
        free(leafNode->key);
        free(leafNode);
        return NULL;
    }
    strcpy(leafNode->value, value);

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
    Node4 *oldNode = (Node4 *)*nodePtr;
    Node16 *newNode = makeNode16(); 

    if (newNode == NULL) {
        // Handle the memory allocation failure
        return NULL;
    }
    
    // Copy prefix from oldNode to newNode.
    memcpy(newNode->prefix, oldNode->prefix, oldNode->prefixLen);
    newNode->prefixLen = oldNode->prefixLen;

    // Copy each children and key from oldNode to newNode
    for (int i = 0; i < oldNode->count; i++) {
        // Find key for current child 
        unsigned char keyChar = oldNode->keys[i];
        newNode->keys[i] = keyChar;
        newNode->children[i] = oldNode->children[i];
    }
    
    // Update children counter in newNode 
    newNode->count = oldNode->count;

    free(oldNode);

    *nodePtr = (Node *)newNode;

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

Node *grow(Node **node){
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
            return NULL;
        }
    }
}

Node4 *transformLeafToNode4(Node *leafNode, const char *existingKey, const char *newKey, void *newValue, int depth){
    Node4 *newNode = makeNode4();
    if(!newNode){
        return NULL;
    }

    // Check the common prefix between the two keys
    int prefixLen = 0;
    while (existingKey[depth + prefixLen] == newKey[depth + prefixLen]) {
        newNode->prefix[prefixLen] = existingKey[depth + prefixLen];
        prefixLen++;
    }
    newNode->prefixLen = prefixLen;

    // Add the existing leaf and the new value to Node4
    char existingKeyChar = existingKey[depth + prefixLen];
    char newKeyChar = newKey[depth + prefixLen];

    addChild((Node *)newNode, existingKeyChar, leafNode);
    addChild((Node *)newNode, newKeyChar, (Node *)makeLeafNode(newKey, newValue));
}

int charToIndex(char c) {
    if (c >= 'a' && c <= 'z') {
        return c - 'a';
    } else if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 26;
    } else if (c >= '0' && c <= '9') {
        return c - '0' + 52;
    }
    
    return -1;
}

void replaceChildInParent(Node *parent, Node *oldChild, Node *newChild){
    if (parent == NULL){
        return;
    }

    switch (parent->type){
    case NODE4:{
        Node4 *node4 = (Node4 *)parent;
        for (int i = 0; i < node4->count; i++){
            if (node4->children[i] == oldChild){
                node4->children[i] = newChild;
                return;
            }
        }
        break;
    }
    case NODE16: {
        Node16 *node16 = (Node16 *)parent;
        for (int i = 0; i < node16->count; i++){
            if (node16->children[i] == oldChild){
                node16->children[i] = newChild;
                return;
            }
        }
        break;
    }
    case NODE48:{
        Node48 *node48 = (Node48 *)parent;
        for (int i = 0; i < 256; i++){
            if (node48->children[node48->keys[i]] == oldChild){
                node48->children[node48->keys[i]] = newChild;
                return;
            }
        }
        break;
    }
    case NODE256:{
        Node256 *node256 = (Node256 *)parent;
        for (int i = 0; i < 256; i++)
        {
            if (node256->children[i] == oldChild)
            {
                node256->children[i] = newChild;
                return;
            }
        }
        break;
    }
    }
}

bool isNodeFull(Node *node) {
    if (node == NULL) {
        return false;
    }

    switch (node->type) {
        case NODE4: {
            Node4 *node4 = (Node4 *)node;
            return node4->count == 4;
        }
        case NODE16: {
            Node16 *node16 = (Node16 *)node;
            return node16->count == 16;
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


Node *insert(Node **root, char *key, void *value, int depth) {
    if (*root == NULL) {
        *root = makeLeafNode(key, value);
        return *root;
    }

    Node *node = *root;

    if (node->type == LEAF) {
        Node4 *newNode = makeNode4();
        char *key2 = loadKey(node);
        int count = 0;
        for (int i = depth; key[i] == key2[i]; i = i + 1) {
            newNode->prefix[i - depth] = key[i];
            count++;
        }
        newNode->prefixLen = count - depth;
        depth += newNode->prefixLen;

        addChild(newNode, key[depth], makeLeafNode(key, value));
        addChild(newNode, key2[depth], node);

        *root = (Node *)newNode;

        free(key2);
        return *root;
    } else {
        // Here we handle the internal nodes
        Node **parentPointer = root;
        while (node) {
            if (depth == strlen(key)){
                // We reached the end of the key we want to add
                // Collision: we reject the insert
                return *root;
            }

            Node *child = findChild(node, key[depth]);
            if(child){
                if(child->type == LEAF){
                    /* If child is a LEAF and key doesn't match 
                    then we have to transform the LEAF in an internal
                    node and add both keys */
                    LeafNode *existingLeaf = (LeafNode *)child;
                    if (strcmp(existingLeaf->key, key) == 0){
                        // Key already present, we reject the insert as collision management
                        return NULL;
                    } else {
                        // Create a new Node4 to replace existing LeafNode
                        Node4 *newNode = makeNode4();

                        // Calculate the common prefix and the point in which the keys differ
                        int commonPrefixLength = checkPrefix((Node *)existingLeaf, key, depth);

                        // Copy common prefix in new Node4
                        memcpy(newNode->prefix, key + depth, commonPrefixLength);
                        newNode->prefixLen = commonPrefixLength;

                        // Calculate new depth after common prefix
                        int newDepth = depth + commonPrefixLength;

                        // Add the existing leaf node and the new leaf node to Node4
                        int existingChildIndex = charToIndex(existingLeaf->key[newDepth]);
                        int newChildIndex = charToIndex(key[newDepth]);

                        if (existingChildIndex == -1 || newChildIndex == -1){
                            // Error: not a valid character
                            return NULL;
                        }

                        newNode->children[existingChildIndex] = existingLeaf;
                        newNode->children[newChildIndex] = makeLeafNode(key + newDepth, value);
                        replaceChildInParent(node, child, (Node *)newNode);
                    }
                }
                node = child; // We continue to traverse the tree
            } else {
                // No corresponding child, we add the new one
                if (isNodeFull(node)){
                    Node *grownNode = grow(&node);
                    if (grownNode == NULL){
                        // We handle the growth error of the node
                        return NULL;
                    }
                    *parentPointer = grownNode;
                    node = grownNode;
                }
                addChild(node, key[depth], makeLeafNode(key + depth, value));
                return *root;
            }
            depth++;
        }
    }

    return *root;
}

void freeNode(Node *node) {
    if (node == NULL) {
        return;
    }   

    switch (node->type) {
        case NODE4: {
            Node4 *node4 = (Node4 *)node;
            for (int i = 0; i < node4->count; ++i) {
                freeNode(node4->children[i]);
            }
            break;
        }
        case NODE16: {
            Node16 *node16 = (Node16 *)node;
            for (int i = 0; i < node16->count; ++i) {
                freeNode(node16->children[i]);
            }
            break;
        }
        case NODE48: {
            Node48 *node48 = (Node48 *)node;
            for (int i = 0; i < 48; ++i) {
                if (node48->keys[i] != EMPTY_KEY) {
                    freeNode(node48->children[i]);
                }
            }
            break;
        }
        case NODE256: {
            Node256 *node256 = (Node256 *)node;
            for (int i = 0; i < 256; ++i) {
                if (node256->children[i] != NULL) {
                    freeNode(node256->children[i]);
                }
            }
            break;
        }
        case LEAF:{
            LeafNode *leafNode = (LeafNode *)node;
            free(leafNode->key);
            free(leafNode->value);
            break;
        }
        default:
            break;
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

//     RUN_TEST(test_transitionFromNode4ToNode16);

//     return UNITY_END();
// }