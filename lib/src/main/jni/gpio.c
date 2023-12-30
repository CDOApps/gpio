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
#include "gpio.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#define GPIO_REGISTERS_N2_BASE 0xff634000
#define GPIO_REGISTERS_N2_MEMORY_SIZE 4096
#define GPIO_REGISTERS_MEMORY "/dev/mem"
#define GPIO_REGISTERS_GPIO_MEMORY "/dev/gpiomem"

#define GPIO_SYSFS_ROOT "/sys/class/gpio"


struct GPIOPin {
    int number;

    struct GPIOPinRegisters {
        int offset;
        int target;

        int set;
        int input;
        int pullUpDownEnable;
        int pullUpDown;
        int function;
        int mux;
    } registers;

    struct GPIOPinSysfs {
        int direction;
        int pull;
        int value;
    } sysfs;
};

// This wiringPi gpio map was found here:
// https://github.com/hardkernel/wiringPi/blob/master/wiringPi/odroidn2.c
static const struct GPIOPin GPIO_N2_PINS[] = {
        {.number = 479, .registers = {.offset = 3, .target = 12, .set = 279, .input = 280, .pullUpDownEnable = 330, .pullUpDown = 316, .function = 278, .mux = 435}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 492, .registers = {.offset = 16, .target = 64, .set = 279, .input = 280, .pullUpDownEnable = 330, .pullUpDown = 316, .function = 278, .mux = 437}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 480, .registers = {.offset = 4, .target = 16, .set = 279, .input = 280, .pullUpDownEnable = 330, .pullUpDown = 316, .function = 278, .mux = 435}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 483, .registers = {.offset = 7, .target = 28, .set = 279, .input = 280, .pullUpDownEnable = 330, .pullUpDown = 316, .function = 278, .mux = 435}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 476, .registers = {.offset = 0, .target = 0, .set = 279, .input = 280, .pullUpDownEnable = 330, .pullUpDown = 316, .function = 278, .mux = 435}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 477, .registers = {.offset = 1, .target = 4, .set = 279, .input = 280, .pullUpDownEnable = 330, .pullUpDown = 316, .function = 278, .mux = 435}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 478, .registers = {.offset = 2, .target = 8, .set = 279, .input = 280, .pullUpDownEnable = 330, .pullUpDown = 316, .function = 278, .mux = 435}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 473, .registers = {.offset = 13, .target = 52, .set = 289, .input = 290, .pullUpDownEnable = 333, .pullUpDown = 319, .function = 288, .mux = 446}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 493, .registers = {.offset = 17, .target = 68, .set = 279, .input = 280, .pullUpDownEnable = 330, .pullUpDown = 316, .function = 278, .mux = 437}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 494, .registers = {.offset = 18, .target = 72, .set = 279, .input = 280, .pullUpDownEnable = 330, .pullUpDown = 316, .function = 278, .mux = 437}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 486, .registers = {.offset = 10, .target = 40, .set = 279, .input = 280, .pullUpDownEnable = 330, .pullUpDown = 316, .function = 278, .mux = 436}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 464, .registers = {.offset = 4, .target = 16, .set = 289, .input = 290, .pullUpDownEnable = 333, .pullUpDown = 319, .function = 288, .mux = 445}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 484, .registers = {.offset = 8, .target = 32, .set = 279, .input = 280, .pullUpDownEnable = 330, .pullUpDown = 316, .function = 278, .mux = 436}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 485, .registers = {.offset = 9, .target = 36, .set = 279, .input = 280, .pullUpDownEnable = 330, .pullUpDown = 316, .function = 278, .mux = 436}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 487, .registers = {.offset = 11, .target = 44, .set = 279, .input = 280, .pullUpDownEnable = 330, .pullUpDown = 316, .function = 278, .mux = 436}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 488, .registers = {.offset = 12, .target = 48, .set = 279, .input = 280, .pullUpDownEnable = 330, .pullUpDown = 316, .function = 278, .mux = 436}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 489, .registers = {.offset = 13, .target = 52, .set = 279, .input = 280, .pullUpDownEnable = 330, .pullUpDown = 316, .function = 278, .mux = 436}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 490, .registers = {.offset = 14, .target = 56, .set = 279, .input = 280, .pullUpDownEnable = 330, .pullUpDown = 316, .function = 278, .mux = 436}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 491, .registers = {.offset = 15, .target = 60, .set = 279, .input = 280, .pullUpDownEnable = 330, .pullUpDown = 316, .function = 278, .mux = 436}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 481, .registers = {.offset = 5, .target = 20, .set = 279, .input = 280, .pullUpDownEnable = 330, .pullUpDown = 316, .function = 278, .mux = 435}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 482, .registers = {.offset = 6, .target = 24, .set = 279, .input = 280, .pullUpDownEnable = 330, .pullUpDown = 316, .function = 278, .mux = 435}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 472, .registers = {.offset = 12, .target = 48, .set = 289, .input = 290, .pullUpDownEnable = 333, .pullUpDown = 319, .function = 288, .mux = 446}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 495, .registers = {.offset = 19, .target = 76, .set = 279, .input = 280, .pullUpDownEnable = 330, .pullUpDown = 316, .function = 278, .mux = 437}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 474, .registers = {.offset = 14, .target = 56, .set = 289, .input = 290, .pullUpDownEnable = 333, .pullUpDown = 319, .function = 288, .mux = 446}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = 475, .registers = {.offset = 15, .target = 60, .set = 289, .input = 290, .pullUpDownEnable = 333, .pullUpDown = 319, .function = 288, .mux = 446}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
        {.number = -1, .registers = {.offset = -1, .target = -1, .set = -1, .input = -1, .pullUpDownEnable = -1, .pullUpDown = -1, .function = -1, .mux = -1}, .sysfs = {.direction = -1, .pull = -1, .value = -1}},
};

struct GPIOInfo {
    enum GPIOMode {
        GPIOModeRegisters,
        GPIOModeSysfs
    } mode;

    struct GPIORegisters {
        int file;
        volatile uint32_t *memory;
    } registers;

    struct GPIOSysfs {
        int export;
        int unexport;
    } sysfs;

    struct GPIOPin pins[64];
};


GPIOInfoRef GPIOInfoAlloc(void) {
    GPIOInfoRef info = malloc(sizeof(struct GPIOInfo));

    info->mode = GPIOModeRegisters;

    info->registers.file = -1;
    info->registers.memory = NULL;

    if (!getuid())
        info->registers.file = open(GPIO_REGISTERS_MEMORY, O_RDWR | O_SYNC | O_CLOEXEC);
    else if (access(GPIO_REGISTERS_GPIO_MEMORY, 0) == 0)
        info->registers.file = open (GPIO_REGISTERS_GPIO_MEMORY, O_RDWR | O_SYNC | O_CLOEXEC);

    if (info->registers.file >= 0) {
#if defined(__aarch64__)
        void *memory = mmap(0, GPIO_REGISTERS_N2_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, info->registers.file, GPIO_REGISTERS_N2_BASE);
#else
        void *memory = mmap64(0, GPIO_REGISTERS_N2_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, info->registers.file, (off64_t)GPIO_REGISTERS_N2_BASE);
#endif

        if (memory == MAP_FAILED) {
            close(info->registers.file);
            info->mode = GPIOModeSysfs;
        } else {
            info->registers.memory = memory;
        }
    }

    if (info->mode == GPIOModeSysfs) {
        info->sysfs.export = open(GPIO_SYSFS_ROOT "/export", O_WRONLY);
        info->sysfs.unexport = open(GPIO_SYSFS_ROOT "/unexport", O_WRONLY);
    } else {
        info->sysfs.export = -1;
        info->sysfs.unexport = -1;
    }

    memcpy(info->pins, GPIO_N2_PINS, sizeof(GPIO_N2_PINS));

    return info;
}

void GPIOInfoFree(GPIOInfoRef info) {
    switch (info->mode) {
        case GPIOModeRegisters:
            if (info->registers.memory)
                munmap(info->registers.memory, GPIO_REGISTERS_N2_MEMORY_SIZE);

            if (info->registers.file != -1)
                close(info->registers.file);
            break;

        case GPIOModeSysfs:
            if (info->sysfs.export != -1)
                close(info->sysfs.export);

            if (info->sysfs.unexport != -1)
                close(info->sysfs.unexport);

            for (int pin = 0 ; pin < 64 ; pin++) {
                struct GPIOPin *pinInfo = &info->pins[pin];
                if (pinInfo->sysfs.direction != -1)
                    close(pinInfo->sysfs.direction);

                if (pinInfo->sysfs.pull != -1)
                    close(pinInfo->sysfs.pull);

                if (pinInfo->sysfs.value != -1)
                    close(pinInfo->sysfs.value);
            }
            break;

        default:
            break;
    }

    free(info);
}


void GPIOInfoExport(GPIOInfoRef info, int pin) {
    if (info->mode != GPIOModeSysfs)
        return;

    struct GPIOPin *pinInfo = &info->pins[pin];
    if (pinInfo->sysfs.direction != -1)
        return;

    char buf[64] = "";
    sprintf(buf, "%d\n", pinInfo->number);

    write(info->sysfs.export, buf, strlen(buf));

    sprintf(buf, GPIO_SYSFS_ROOT "/gpio%d/direction", pinInfo->number);
    for (int try = 0 ; try < 5 ; try++) {
        pinInfo->sysfs.direction = open(buf, O_WRONLY);

        if (pinInfo->sysfs.direction != -1)
            break;
        else if (try < 4)
            sleep(1);
    }

    if (pinInfo->sysfs.direction == -1) {
        sprintf(buf, "%d\n", pinInfo->number);
        write(info->sysfs.unexport, buf, strlen(buf));
        return;
    }

    sprintf(buf, GPIO_SYSFS_ROOT "/gpio%d/pull", pinInfo->number);
    pinInfo->sysfs.pull = open(buf, O_WRONLY);

    sprintf(buf, GPIO_SYSFS_ROOT "/gpio%d/value", pinInfo->number);
    pinInfo->sysfs.value = open(buf, O_RDWR);
}

void GPIOInfoUnexport(GPIOInfoRef info, int pin) {
    if (info->mode != GPIOModeSysfs)
        return;

    struct GPIOPin *pinInfo = &info->pins[pin];
    if (pinInfo->sysfs.direction == -1)
        return;

    if (pinInfo->sysfs.direction != -1)
        close(pinInfo->sysfs.direction);

    if (pinInfo->sysfs.pull != -1)
        close(pinInfo->sysfs.pull);

    if (pinInfo->sysfs.value != -1)
        close(pinInfo->sysfs.value);

    char buf[64] = "";
    sprintf(buf, "%d\n", pinInfo->number);

    write(info->sysfs.unexport, buf, strlen(buf));
}

void GPIOInfoUnexportAll(GPIOInfoRef info) {
    if (info->mode != GPIOModeSysfs)
        return;

    for (int pin = 0 ; pin < 64 ; pin++)
        GPIOInfoUnexport(info, pin);
}

const char *GPIO_PIN_MODE_INPUT = "in";
const char *GPIO_PIN_MODE_OUTPUT = "out";

#define GPIO_SELECT() info->registers.memory[pinInfo->registers.mux] &= ~(0xF << pinInfo->registers.target)
#define GPIO_ENABLE_REGISTER(name) info->registers.memory[pinInfo->registers.name] |= (1 << pinInfo->registers.offset)
#define GPIO_DISABLE_REGISTER(name) info->registers.memory[pinInfo->registers.name] &= ~(1 << pinInfo->registers.offset)
#define GPIO_READ_REGISTER(name) (info->registers.memory[pinInfo->registers.name] & (1 << pinInfo->registers.offset))

void GPIOInfoSetMode(GPIOInfoRef info, int pin, const char *mode) {
    struct GPIOPin *pinInfo = &info->pins[pin];

    switch (info->mode) {
        case GPIOModeRegisters:
            GPIO_SELECT();

            if (strcmp(GPIO_PIN_MODE_INPUT, mode) == 0)
                GPIO_ENABLE_REGISTER(function);
            else if (strcmp(GPIO_PIN_MODE_OUTPUT, mode) == 0)
                GPIO_DISABLE_REGISTER(function);
            break;

        case GPIOModeSysfs:
            if (pinInfo->sysfs.direction == -1)
                return;

            char buf[64] = "";
            sprintf(buf, "%s\n", mode);

            write(pinInfo->sysfs.direction, buf, strlen(buf));
            break;

        default:
            break;
    }
}

const char *GPIO_PIN_PULL_DOWN = "down";
const char *GPIO_PIN_PULL_UP = "up";
const char *GPIO_PIN_PULL_OFF = "disable";

void GPIOInfoSetPullState(GPIOInfoRef info, int pin, const char *state) {
    struct GPIOPin *pinInfo = &info->pins[pin];

    switch (info->mode) {
        case GPIOModeRegisters:
            if (strcmp(GPIO_PIN_PULL_OFF, state) == 0) {
                GPIO_DISABLE_REGISTER(pullUpDownEnable);
            } else {
                GPIO_ENABLE_REGISTER(pullUpDownEnable);

                if (strcmp(GPIO_PIN_PULL_DOWN, state) == 0)
                    GPIO_DISABLE_REGISTER(pullUpDown);
                else if (strcmp(GPIO_PIN_PULL_UP, state) == 0)
                    GPIO_ENABLE_REGISTER(pullUpDown);
            }
            break;

        case GPIOModeSysfs:
            if (pinInfo->sysfs.pull == -1)
                return;

            char buf[64] = "";
            sprintf(buf, "%s\n", state);

            write(pinInfo->sysfs.pull, buf, strlen(buf));
            break;

        default:
            break;
    }
}

void GPIOInfoSetValue(GPIOInfoRef info, int pin, int value) {
    struct GPIOPin *pinInfo = &info->pins[pin];

    switch (info->mode) {
        case GPIOModeRegisters:
            if (value)
                GPIO_ENABLE_REGISTER(set);
            else
                GPIO_DISABLE_REGISTER(set);
            break;

        case GPIOModeSysfs:
            if (pinInfo->sysfs.value == -1)
                return;

            char buf[64] = "";
            sprintf(buf, "%d\n", value);

            write(pinInfo->sysfs.value, buf, strlen(buf));
            break;

        default:
            break;
    }
}

int GPIOInfoGetValue(GPIOInfoRef info, int pin) {
    struct GPIOPin *pinInfo = &info->pins[pin];

    switch (info->mode) {
        case GPIOModeRegisters:
            if (GPIO_READ_REGISTER(input) != 0)
                return GPIO_PIN_VALUE_HIGH;

            return GPIO_PIN_VALUE_LOW;

        case GPIOModeSysfs:
            if (pinInfo->sysfs.value == -1)
                return -1;

            lseek(pinInfo->sysfs.value, 0, SEEK_SET);

            char value = 0x0;
            if (read(pinInfo->sysfs.value, &value, 1) < 0)
                return -1;

            return (value == '0') ? GPIO_PIN_VALUE_LOW : GPIO_PIN_VALUE_HIGH;

        default:
            break;
    }

    return -1;
}

JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_GPIO_onResume(JNIEnv * env, jobject thiz) {
    GPIOInfoRef info = (GPIOInfoRef)Java_java_lang_Object_getReserved(env, thiz);
    if (info)
        GPIOInfoFree(info);

    Java_java_lang_Object_setReserved(env, thiz, (jlong)GPIOInfoAlloc());
}


JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_GPIO_onPause(JNIEnv * env, jobject thiz) {
    GPIOInfoRef info = (GPIOInfoRef)Java_java_lang_Object_getReserved(env, thiz);
    if (info) {
        GPIOInfoFree(info);
        Java_java_lang_Object_setReserved(env, thiz, 0l);
    }
}

JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_GPIO_export(JNIEnv * env, jobject thiz, jint pin) {
    GPIOInfoRef info = (GPIOInfoRef)Java_java_lang_Object_getReserved(env, thiz);
    if (info)
        GPIOInfoExport(info, pin);
}

JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_GPIO_unexport(JNIEnv * env, jobject thiz, jint pin) {
    GPIOInfoRef info = (GPIOInfoRef)Java_java_lang_Object_getReserved(env, thiz);
    if (info)
        GPIOInfoUnexport(info, pin);
}

JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_GPIO_unexportAll(JNIEnv * env, jobject thiz) {
    GPIOInfoRef info = (GPIOInfoRef)Java_java_lang_Object_getReserved(env, thiz);
    if (info)
        GPIOInfoUnexportAll(info);
}

JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_GPIO_setMode(JNIEnv * env, jobject thiz, jint pin, jstring mode) {
    GPIOInfoRef info = (GPIOInfoRef)Java_java_lang_Object_getReserved(env, thiz);
    if (info) {
        const char *utf8Mode = (*env)->GetStringUTFChars(env, mode, NULL);
        GPIOInfoSetMode(info, pin, utf8Mode);
        (*env)->ReleaseStringUTFChars(env, mode, utf8Mode);
    }
}

JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_GPIO_setPullState(JNIEnv * env, jobject thiz, jint pin, jstring state) {
    GPIOInfoRef info = (GPIOInfoRef)Java_java_lang_Object_getReserved(env, thiz);
    if (info) {
        const char *utf8State = (*env)->GetStringUTFChars(env, state, NULL);
        GPIOInfoSetPullState(info, pin, utf8State);
        (*env)->ReleaseStringUTFChars(env, state, utf8State);
    }
}

JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_GPIO_setValue(JNIEnv * env, jobject thiz, jint pin, jint value) {
    GPIOInfoRef info = (GPIOInfoRef)Java_java_lang_Object_getReserved(env, thiz);
    if (info)
        GPIOInfoSetValue(info, pin, value);
}

JNIEXPORT jint JNICALL
Java_com_cdoapps_gpio_GPIO_getValue(JNIEnv *env, jobject thiz, jint pin) {
    GPIOInfoRef info = (GPIOInfoRef)Java_java_lang_Object_getReserved(env, thiz);
    if (info)
        return GPIOInfoGetValue(info, pin);

    return -1;
}