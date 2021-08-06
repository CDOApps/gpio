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

#ifndef GPIO_STACK_H
#define GPIO_STACK_H

/**
 * The {@code Stack} struct represents a last-in-first-out (LIFO) or first-in-first-out (FIFO) stack
 * of objects.
 */
typedef struct Stack *StackRef;

/**
 * Returns a new {@code Stack} object.
 *
 * @param fifo if {@code TRUE}, the result will represents a FIFO stack.
 * @return a new {@code Stack} object.
 */
StackRef StackCreate(BOOL fifo);
/**
 * Destroys a stack.
 *
 * @param stack a {@code Stack} object.
 */
void StackFree(StackRef stack);

/**
 * Returns a pointer to access all the objects of a stack in insertion order.
 *
 * @param stack a {@code Stack} object.
 */
const void *StackGetBaseAddress(StackRef stack);
/**
 * Returns the number of elements contained in a stack.
 *
 * @param stack a {@code Stack} object.
 * @return the number of elements contained in a stack.
 */
unsigned int StackLength(StackRef stack);

/**
 * Adds an element in a stack.
 *
 * @param stack a {@code Stack} object.
 * @param data any object to store in {@code stack}.
 */
void StackPush(StackRef stack, void *data);
/**
 * Returns the last element of a stack if LIFO, or the first if FIFO.
 *
 * @param stack a {@code Stack} object.
 * @return the last element of {@code stack} if LIFO, or the first if FIFO.
 */
void *StackPeek(StackRef stack);
/**
 * Removes and returns the last element of a stack if LIFO, or the first if FIFO.
 *
 * @param stack a {@code Stack} object.
 * @return the last element of {@code stack} if LIFO, or the first if FIFO.
 */
void *StackPop(StackRef stack);

/**
 * Removes all the elements of a stack.
 *
 * @param stack a {@code Stack} object.
 */
void StackEmpty(StackRef stack);


#endif //GPIO_STACK_H
