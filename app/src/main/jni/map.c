// Copyright 2021 CDO Apps
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "common.h"
#include "map.h"

#include "gpio.h"
#include <stdlib.h>

struct MapNode {
    void *key;
    void *data;

    struct MapNode *left;
    struct MapNode *right;
};

struct Map {
    void * (*allocator)(void *);
    void (*deallocator)(void *);
    int (*comparator)(void *, void *);

    struct MapNode *root;
};

MapRef MapCreate(void *(*keyAllocator)(void *), void (*keyDeallocator)(void *), int (*keyComparator)(void *, void *)) {
    MapRef map = (MapRef)malloc(sizeof(struct Map));

    map->allocator = keyAllocator;
    map->deallocator = keyDeallocator;
    map->comparator = keyComparator;
    map->root = NULL;

    return map;
}

#include <string.h>
void *MapStringKeyAllocator(void *key) {
    void *copy = malloc(strlen(key) + 1);
    strcpy(copy, key);
    return copy;
}

MapRef MapStringCreate(void) {
    return MapCreate(MapStringKeyAllocator, free, strcmp);
}

int MapIntCompare(void *key1, void *key2) {
    return (long)key1 - (long)key2;
}

MapRef MapIntCreate(void) {
    return MapCreate(NULL, NULL, MapIntCompare);
}

#include "stack.h"

void MapFree(MapRef map) {
    if (map->root) {
        StackRef stack = StackCreate(FALSE);
        StackPush(stack, map->root);

        while (StackLength(stack) > 0) {
            struct MapNode *node = StackPop(stack);

            if (node->left)
                StackPush(stack, node->left);

            if (node->right)
                StackPush(stack, node->right);

            if (map->deallocator)
                map->deallocator(node->key);

            free(node);
        }

        StackFree(stack);
    }

    free(map);
}

BOOL MapIsEmpty(MapRef map) {
    return map->root ? FALSE : TRUE;
}

void *MapSet(MapRef map, const void *key, void *data) {
    int comparison = 0;
    struct MapNode *node = map->root;
    struct MapNode *previous = NULL;

    while (node) {
        previous = node;
        comparison = map->comparator(previous->key, key);

        if (comparison < 0) {
            node = previous->left;
        } else if (comparison > 0) {
            node = previous->right;
        } else {
            void *oldData = previous->data;
            previous->data = data;
            return oldData;
        }
    }

    node = malloc(sizeof(struct MapNode));
    node->key = map->allocator ? map->allocator(key) : key;
    node->data = data;
    node->left = NULL;
    node->right = NULL;

    if (!previous)
        map->root = node;
    else if (comparison < 0)
        previous->left = node;
    else
        previous->right = node;

    return NULL;
}

void *MapGet(MapRef map, const void *key) {
    struct MapNode *node = map->root;

    while (node) {
        int comparison = map->comparator(node->key, key);

        if (comparison < 0)
            node = node->left;
        else if (comparison > 0)
            node = node->right;
        else
            return node->data;
    }

    return NULL;
}

void *MapRemove(MapRef map, const void *key) {
    struct MapNode *node = map->root;
    struct MapNode *parent = NULL;
    struct MapNode *next = NULL;

    int result = 0;

    while (node) {
        int comparison = map->comparator(node->key, key);

        if (comparison < 0) {
            parent = node;
            node = node->left;
        } else if (comparison > 0) {
            parent = node;
            node = node->right;
        } else {
            if (!node->left) {
                next = node->right;
            } else if (!node->right) {
                next = node->left;
            } else {
                struct MapNode *previous = NULL;
                next = node->right;

                while (next->left) {
                    previous = next;
                    next = next->left;
                }

                if (previous)
                    previous->left = next->right;

                next->left = node->left;
                if (previous)
                    next->right = node->right;
            }

            if (!result)
                map->root = next;
            else if (result < 0)
                parent->left = next;
            else
                parent->right = next;

            void *data = node->data;

            if (map->deallocator)
                map->deallocator(node->key);

            free(node);

            return data;
        }

        result = comparison;
    }

    return NULL;
}