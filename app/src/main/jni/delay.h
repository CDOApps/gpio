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

#ifndef GPIO_DELAY_H
#define GPIO_DELAY_H

#include <time.h>

/**
 * Busy waits for a specific duration.
 *
 * @param date if not {@code NULL}, set the start date of the spinning.
 * @param delay a duration in nanoseconds.
 */
void DelayNanoFrom(struct timespec *date, long delay);
/**
 * Busy waits for a specific duration.
 *
 * @param date if not {@code NULL}, set the start date of the spinning.
 * @param delay a duration in microseconds.
 */
void DelayMicroFrom(struct timespec *date, int delay);

/**
 * Busy waits for a specific duration.
 *
 * @param delay a duration in nanoseconds.
 */
void DelayNano(long delay);
/**
 * Busy waits for a specific duration.
 *
 * @param delay a duration in microseconds.
 */
void DelayMicro(int delay);

#endif //GPIO_DELAY_H
