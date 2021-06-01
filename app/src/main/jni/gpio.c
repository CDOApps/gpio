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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define GPIO_ROOT "/sys/class/gpio"

// This wiringPi gpio map was found here:
// https://github.com/hardkernel/wiringPi/blob/master/wiringPi/odroidn2.c
static const int GPIO_N2_PORTS[64] = {
        479, 492,	// GPIOX.3		        0	| 1	    GPIOX.16(PWM_E)
        480, 483,	// GPIOX.4		        2	| 3	    GPIOX.7(PWM_F)
        476, 477,	// GPIOX.0		        4	| 5	    GPIOX.1
        478, 473,	// GPIOX.2		        6	| 7	    GPIOA.13
        493, 494,	// GPIOX.17(I2C-2_SDA)  8	| 9     GPIOX.18(I2C-2_SCL)
        486, 464,	// GPIOX.10		       	10	| 11	GPIOA.4
        484, 485,	// GPIOX.8		    	12	| 13	GPIOX.9
        487, 488,	// GPIOX.11		    	14	| 15	GPIOX.12
        489,  -1,	// GPIOX.13		    	16	| 17
        -1,  -1,	// 				        18	| 19
        -1,  490,	// 				        20	| 21	GPIOX.14
        491, 481,	// GPIOX.15			    22	| 23	GPIOX.5(PWM_C)
        482, -1,	// GPIOX.6(PWM_D)		24	| 25	ADC.AIN3
        472, 495,	// GPIOA.12			    26	| 27	GPIOX.19
        -1,  -1,	// REF1.8V OUT			28	| 29	ADC.AIN2
        474, 475,	// GPIOA.14(I2C-3_SDA)	30	| 31	GPIOA.15(I2C-3_SCL)
        // Padding:
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 32...47
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 48...63
};

struct GPIOPin {
    int direction;
    int value;
};

struct GPIOInfo {
    int ports[64];

    int export;
    int unexport;

    struct GPIOPin pins[64];
};


GPIOInfoRef GPIOInfoAlloc(void) {
    GPIOInfoRef info = malloc(sizeof(struct GPIOInfo));

    memcpy(info->ports, GPIO_N2_PORTS, 64 * sizeof(int));
    info->export = open(GPIO_ROOT "/export", O_WRONLY);
    info->unexport = open(GPIO_ROOT "/unexport", O_WRONLY);
    for (int pin = 0 ; pin < 64 ; pin++) {
        struct GPIOPin *pinInfo = &info->pins[pin];
        pinInfo->direction = -1;
        pinInfo->value = -1;
    }

    return info;
}

void GPIOInfoFree(GPIOInfoRef info) {
    if (info->export != -1)
        close(info->export);

    if (info->unexport != -1)
        close(info->unexport);

    for (int pin = 0 ; pin < 64 ; pin++) {
        struct GPIOPin *pinInfo = &info->pins[pin];
        if (pinInfo->direction != -1)
            close(pinInfo->direction);

        if (pinInfo->value != -1)
            close(pinInfo->value);
    }

    free(info);
}


void GPIOInfoExport(GPIOInfoRef info, int pin) {
    struct GPIOPin *pinInfo = &info->pins[pin];
    if (pinInfo->direction != -1)
        return;

    int port = info->ports[pin];

    char buf[64] = "";
    sprintf(buf, "%d\n", port);

    write(info->export, buf, strlen(buf));

    sprintf(buf, GPIO_ROOT "/gpio%d/direction", port);
    for (int try = 0 ; try < 5 ; try++) {
        pinInfo->direction = open(buf, O_WRONLY);

        if (pinInfo->direction != -1)
            break;
        else if (try < 4)
            sleep(1);
    }

    if (pinInfo->direction == -1) {
        sprintf(buf, "%d\n", port);
        write(info->unexport, buf, strlen(buf));
        return;
    }

    sprintf(buf, GPIO_ROOT "/gpio%d/value", port);
    pinInfo->value = open(buf, O_RDWR);
}

void GPIOInfoUnexport(GPIOInfoRef info, int pin) {
    struct GPIOPin *pinInfo = &info->pins[pin];
    if (pinInfo->direction == -1)
        return;

    int port = info->ports[pin];

    if (pinInfo->direction != -1)
        close(pinInfo->direction);

    if (pinInfo->value != -1)
        close(pinInfo->value);

    char buf[64] = "";
    sprintf(buf, "%d\n", port);

    write(info->unexport, buf, strlen(buf));
}

void GPIOInfoUnexportAll(GPIOInfoRef info) {
    for (int pin = 0 ; pin < 64 ; pin++)
        GPIOInfoUnexport(info, pin);
}

const char *GPIO_PIN_MODE_INPUT = "in";
const char *GPIO_PIN_MODE_OUTPUT = "out";

void GPIOInfoSetMode(GPIOInfoRef info, int pin, const char *mode) {
    struct GPIOPin *pinInfo = &info->pins[pin];
    if (pinInfo->direction == -1)
        return;

    char buf[64] = "";
    sprintf(buf, "%s\n", mode);

    write(pinInfo->direction, buf, strlen(buf));
}

void GPIOInfoSetValue(GPIOInfoRef info, int pin, int value) {
    struct GPIOPin *pinInfo = &info->pins[pin];
    if (pinInfo->value == -1)
        return;

    char buf[64] = "";
    sprintf(buf, "%d\n", value);

    write(pinInfo->value, buf, strlen(buf));
}

int GPIOInfoGetValue(GPIOInfoRef info, int pin) {
    struct GPIOPin *pinInfo = &info->pins[pin];
    if (pinInfo->value == -1)
        return -1;

    lseek(pinInfo->value, 0, SEEK_SET);

    char value = 0x0;
    if (read(pinInfo->value, &value, 1) < 0)
        return -1;

    return (value == '0') ? GPIO_PIN_VALUE_LOW : GPIO_PIN_VALUE_HIGH;
}

#include "map.h"

static MapRef gpios = NULL;

JNIEXPORT jint JNICALL
Java_com_cdoapps_gpio_GPIO_getHashCode(JNIEnv * env, jobject thiz) {
    static jmethodID hashCodeMethodID = 0;

    if (!hashCodeMethodID) {
        jclass clazz = (*env)->GetObjectClass(env, thiz);
        hashCodeMethodID = (*env)->GetMethodID(env, clazz, "hashCode", "()I");
    }

    return (*env)->CallIntMethod(env, thiz, hashCodeMethodID);
}

JNIEXPORT jlong JNICALL
Java_com_cdoapps_gpio_GPIO_getInfo(JNIEnv * env, jobject thiz) {
    return (jlong)MapGet(gpios, Java_com_cdoapps_gpio_GPIO_getHashCode(env, thiz));
}


JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_GPIO_onResume(JNIEnv * env, jobject thiz) {
    if (!gpios)
        gpios = MapIntCreate();

    MapSet(gpios, Java_com_cdoapps_gpio_GPIO_getHashCode(env, thiz), GPIOInfoAlloc());
}


JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_GPIO_onPause(JNIEnv * env, jobject thiz) {
    GPIOInfoFree(MapRemove(gpios, Java_com_cdoapps_gpio_GPIO_getHashCode(env, thiz)));

    if (MapIsEmpty(gpios)) {
        MapFree(gpios);
        gpios = NULL;
    }
}

JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_GPIO_export(JNIEnv * env, jobject thiz, jint pin) {
    GPIOInfoExport(Java_com_cdoapps_gpio_GPIO_getInfo(env, thiz), pin);
}

JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_GPIO_unexport(JNIEnv * env, jobject thiz, jint pin) {
    GPIOInfoUnexport(Java_com_cdoapps_gpio_GPIO_getInfo(env, thiz), pin);
}

JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_GPIO_unexportAll(JNIEnv * env, jobject thiz) {
    GPIOInfoUnexportAll(Java_com_cdoapps_gpio_GPIO_getInfo(env, thiz));
}

JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_GPIO_setMode(JNIEnv * env, jobject thiz, jint pin, jstring mode) {
    const char *utf8Mode = (*env)->GetStringUTFChars(env, mode, NULL);
    GPIOInfoSetMode(Java_com_cdoapps_gpio_GPIO_getInfo(env, thiz), pin, utf8Mode);
    (*env)->ReleaseStringUTFChars(env, mode, utf8Mode);
}

JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_GPIO_setValue(JNIEnv * env, jobject thiz, jint pin, jint value) {
    GPIOInfoSetValue(Java_com_cdoapps_gpio_GPIO_getInfo(env, thiz), pin, value);
}

JNIEXPORT jint JNICALL
Java_com_cdoapps_gpio_GPIO_getValue(JNIEnv *env, jobject thiz, jint pin) {
    return GPIOInfoGetValue(Java_com_cdoapps_gpio_GPIO_getInfo(env, thiz), pin);
}