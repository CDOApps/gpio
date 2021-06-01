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

#ifndef GPIO_MAP_H
#define GPIO_MAP_H

/**
 * The {@code Map} struct represents an object that maps keys to values.
 */
typedef struct Map *MapRef;

/**
 * Returns a new {@code Map} object.
 *
 * @param keyAllocator called to create keys from the given pointer, if {@code NULL} the pointer is
 *                     copied (should be acceptable only for keys of integer type).
 * @param keyDeallocator called to destroy a key which is passed to it, if {@code NULL} the key is
 *                       never destroyed (should be acceptable only for keys of integer type).
 * @param keyComparator called to compare keys which are passed to it, should return a negative
 *                      number if left and right keys are in ascending order, a positive numbeer
 *                      if they are in descending order or 0 if they are equal.
 * @return a new {@code Map} object.
 */
MapRef MapCreate(void *(*keyAllocator)(void *), void (*keyDeallocator)(void *), int (*keyComparator)(void *, void *));
/**
 * Returns a new {@code Map} object configured with characters string keys.
 *
 * When passing a characters string or array to {@code MapSet} a new memory is allocated for copying
 * the key, so it is safe to destroy the given parameter if needed.
 */
MapRef MapStringCreate(void);
/**
 * Returns a new {@code Map} object configured with integer keys.
 */
MapRef MapIntCreate(void);
/**
 * Destroys a map and all the keys that it retains.
 *
 * @param map a {@code Map} object.
 */
void MapFree(MapRef map);

/**
 * Returns {@code TRUE} if the map does not retain any key.
 *
 * @param map a {@code Map} object.
 * @return {@code TRUE} if the map does not retain any key.
 */
BOOL MapIsEmpty(MapRef map);

/**
 * Maps a key to a new value and returns the previous value mapped to it or {@code NULL} if no entry
 * was found for this key.
 *
 * @param map a {@code Map} object.
 * @param key a key mapped to @{code data}.
 * @param data the new value mapped to @{code key}.
 * @return the previous value mapped to @{code key} or {@code NULL} if no entry was found for it.
 */
void *MapSet(MapRef map, const void *key, void *data);
/**
 * Returns the value mapped to a key or {@code NULL} if no entry was found for this key.
 *
 * @param map a {@code Map} object.
 * @param key a key.
 * @return the value mapped to @{code key} or {@code NULL} if no entry was found for it.
 */
void *MapGet(MapRef map, const void *key);
/**
 * Removes the entry found for a key and returns the previous value mapped to a key or {@code NULL}
 * if none was found for this key.
 *
 * @param map a {@code Map} object.
 * @param key a key.
 * @return the previous value mapped to @{code key} or {@code NULL} if no entry was found for it.
 */
void *MapRemove(MapRef map, const void *key);

#endif //GPIO_MAP_H
