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
#include "stack.h"

#include <stdlib.h>
#include <string.h>

struct Stack {
    BOOL fifo;
    unsigned int capacity;
    unsigned int index;
    void **data;
};

StackRef StackCreate(BOOL fifo) {
    StackRef stack = malloc(sizeof(struct Stack));

    stack->fifo = fifo;
    stack->capacity = 2048;
    stack->index = 0;
    stack->data = malloc(sizeof(void *) * stack->capacity);

    return stack;
}

void StackFree(StackRef stack) {
    free(stack->data);
    free(stack);
}

const void *StackGetBaseAddress(StackRef stack) {
    return stack->data;
}

unsigned int StackLength(StackRef stack) {
    return stack->index;
}

void StackPush(StackRef stack, void *data) {
    stack->data[stack->index++] = data;
    if (stack->index == stack->capacity) {
        stack->capacity *= 2;
        stack->data = realloc(stack->data, sizeof(void *) * stack->capacity);
    }
}

void *StackPop(StackRef stack) {
    if (stack->index == 0)
        return NULL;

    if (stack->fifo) {
        void *first = stack->data[0];
        stack->index--;
        if (stack->index > 0)
            memmove(stack->data, &stack->data[1], sizeof(void *) * stack->index);
        return first;
    }

    return stack->data[--stack->index];
}

void *StackPeek(StackRef stack) {
    if (stack->index == 0)
        return NULL;

    if (stack->fifo)
        return stack->data[0];

    return stack->data[stack->index - 1];
}

void StackEmpty(StackRef stack) {
    stack->index = 0;
}