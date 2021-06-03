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

// This implementation of the Dallas 1-Wire protocol was inspired by the work of Daniel Perron:
// https://github.com/danjperron/BitBangingDS18B20

#include "common.h"
#include "onewire.h"

#include <stdlib.h>

struct OneWireInfo {
    GPIOInfoRef gpioInfo;
    int pin;
};

OneWireInfoRef OneWireInfoCreate(GPIOInfoRef gpioInfo, int pin) {
    OneWireInfoRef info = malloc(sizeof(struct OneWireInfo));

    info->gpioInfo = gpioInfo;
    info->pin = pin;

    GPIOInfoExport(gpioInfo, pin);

    return info;
}

void OneWireInfoFree(OneWireInfoRef info) {
    GPIOInfoUnexport(info->gpioInfo, info->pin);
    free(info);
}

#include "delay.h"

BOOL OneWireInfoReset(OneWireInfoRef info) {
    GPIOInfoSetMode(info->gpioInfo, info->pin, GPIO_PIN_MODE_INPUT);
    DelayMicro(10);

    GPIOInfoSetMode(info->gpioInfo, info->pin, GPIO_PIN_MODE_OUTPUT);
    GPIOInfoSetValue(info->gpioInfo, info->pin, GPIO_PIN_VALUE_LOW);

    struct timespec time = { .tv_sec = 0, .tv_nsec = 480000 };
    nanosleep(&time, NULL);

    GPIOInfoSetMode(info->gpioInfo, info->pin, GPIO_PIN_MODE_INPUT);
    DelayMicro(60);

    if (GPIOInfoGetValue(info->gpioInfo, info->pin) == GPIO_PIN_VALUE_LOW) {
        DelayMicro(420);
        return TRUE;
    }

    return FALSE;
}


void OneWireInfoWriteBit(OneWireInfoRef info, BOOL bit) {
    GPIOInfoSetMode(info->gpioInfo, info->pin, GPIO_PIN_MODE_OUTPUT);
    GPIOInfoSetValue(info->gpioInfo, info->pin, GPIO_PIN_VALUE_LOW);

    if (bit) {
        DelayMicro(1);
        GPIOInfoSetMode(info->gpioInfo, info->pin, GPIO_PIN_MODE_INPUT);
        DelayMicro(60);
    } else {
        DelayMicro(60);
        GPIOInfoSetMode(info->gpioInfo, info->pin, GPIO_PIN_MODE_INPUT);
        DelayMicro(1);
    }
}

void OneWireInfoWriteByte(OneWireInfoRef info, unsigned char value) {
    for (int position = 0 ; position < 8 ; position++) {
        OneWireInfoWriteBit(info, (value & (0x1 << position)) ? TRUE : FALSE);
        DelayMicro(60);
    }

    struct timespec time = {.tv_sec = 0, .tv_nsec = 100000};
    nanosleep(&time, NULL);
}


BOOL OneWireInfoReadBit(OneWireInfoRef info) {
    GPIOInfoSetMode(info->gpioInfo, info->pin, GPIO_PIN_MODE_OUTPUT);
    GPIOInfoSetValue(info->gpioInfo, info->pin, GPIO_PIN_VALUE_LOW);

    DelayMicro(1);
    GPIOInfoSetMode(info->gpioInfo, info->pin, GPIO_PIN_MODE_INPUT);
    DelayMicro(2);

    BOOL bit = (GPIOInfoGetValue(info->gpioInfo, info->pin) != GPIO_PIN_VALUE_LOW);
    DelayMicro(60);

    return bit;
}

unsigned char OneWireInfoReadByte(OneWireInfoRef info) {
    unsigned char byte = 0x0;

    for (int position = 0 ; position < 8 ; position++) {
        if (OneWireInfoReadBit(info))
            byte |= (0x1 << position);
        DelayMicro(60);
    }

    return byte;
}

JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_OneWire_configure(JNIEnv * env, jobject thiz, jobject gpio,
                                        jint pin) {
    OneWireInfoRef info = (OneWireInfoRef)Java_java_lang_Object_getReserved(env, thiz);
    if (info)
        OneWireInfoFree(info);

    GPIOInfoRef gpioInfo = (GPIOInfoRef)Java_java_lang_Object_getReserved(env, gpio);
    Java_java_lang_Object_setReserved(env, thiz, (jlong)OneWireInfoCreate(gpioInfo, pin));
}


JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_OneWire_destroy(JNIEnv * env, jobject thiz) {
    OneWireInfoRef info = (OneWireInfoRef)Java_java_lang_Object_getReserved(env, thiz);
    if (info) {
        OneWireInfoFree(info);
        Java_java_lang_Object_setReserved(env, thiz, 0l);
    }
}

JNIEXPORT jboolean JNICALL
Java_com_cdoapps_gpio_OneWire_reset(JNIEnv * env, jobject thiz) {
    OneWireInfoRef info = (OneWireInfoRef)Java_java_lang_Object_getReserved(env, thiz);
    if (info && OneWireInfoReset(info))
        return JNI_TRUE;

    return JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_OneWire_writeBit(JNIEnv * env, jobject thiz, jboolean bit) {
    OneWireInfoRef info = (OneWireInfoRef)Java_java_lang_Object_getReserved(env, thiz);
    if (info)
        OneWireInfoWriteBit(info, bit ? TRUE : FALSE);
}

JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_OneWire_writeByte(JNIEnv * env, jobject thiz, jbyte value) {
    OneWireInfoRef info = (OneWireInfoRef)Java_java_lang_Object_getReserved(env, thiz);
    if (info)
        OneWireInfoWriteByte(info, (unsigned char)value);
}

JNIEXPORT jboolean JNICALL
Java_com_cdoapps_gpio_OneWire_readBit(JNIEnv * env, jobject thiz) {
    OneWireInfoRef info = (OneWireInfoRef)Java_java_lang_Object_getReserved(env, thiz);
    if (info && OneWireInfoReadBit(info))
        return JNI_TRUE;

    return JNI_FALSE;
}

JNIEXPORT jbyte JNICALL
Java_com_cdoapps_gpio_OneWire_readByte(JNIEnv * env, jobject thiz) {
    OneWireInfoRef info = (OneWireInfoRef)Java_java_lang_Object_getReserved(env, thiz);
    if (info)
        return OneWireInfoReadByte(info);

    return 0x0;
}