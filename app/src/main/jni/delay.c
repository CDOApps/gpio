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
#include "delay.h"

void DelayNanoFrom(struct timespec *date, long delay) {
    struct timespec now;
    long t;

    if (date) {
        t = date->tv_nsec;
    } else {
        clock_gettime(CLOCK_REALTIME, &now);
        t = now.tv_nsec;
    }

    if (delay > 999999999)
        delay = 999999999;

    while (TRUE) {
        clock_gettime(CLOCK_REALTIME, &now);

        long dt = now.tv_nsec - t;
        if (dt < 0)
            dt += 1000000000;

        if (dt > delay)
            break;
    }
}

void DelayMicroFrom(struct timespec *date, int delay) {
    DelayNanoFrom(date, 1000 * delay);
}

void DelayNano(long delay) {
    DelayNanoFrom(NULL, delay);
}

void DelayMicro(int delay) {
    DelayNanoFrom(NULL, 1000 * delay);
}
