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
// The timings were reviewed following the '1-Wire Communication Through Software' Application
// note from Analog Devices:
// https://www.analog.com/en/technical-articles/1wire-communication-through-software.html

#include "common.h"
#include "onewire.h"

#include <stdlib.h>

struct OneWireInfo {
    GPIOInfoRef gpioInfo;
    int inputPin;
    int outputPin;

    struct OneWireDelays {
        int a; // Write 1 bit/Read bit: drive bus low delay
        int b; // Write 1 bit: release bus delay
        int c; // Write 0 bit: drive bus low delay
        int d; // Write 0 bit: release bus delay
        int e; // Read bit: release bus delay
        int f; // Read bit: recovery delay
        int g; // Reset: initial delay
        int h; // Reset: drive bus low delay
        int i; // Reset: release bus delay
        int j; // Reset: recovery delay
    } delays;
};

OneWireInfoRef OneWireInfoCreate(GPIOInfoRef gpioInfo, int pin) {
    OneWireInfoRef info = malloc(sizeof(struct OneWireInfo));

    info->gpioInfo = gpioInfo;
    info->inputPin = pin;
    info->outputPin = -1;
    info->delays = (struct OneWireDelays){
        .a = 6,
        .b = 64,
        .c = 60,
        .d = 10,
        .e = 9,
        .f = 55,
        .g = 0,
        .h = 480,
        .i = 70,
        .j = 410
    };

    GPIOInfoExport(gpioInfo, pin);

    return info;
}

OneWireInfoRef OneWireInfoCreateBuffered(GPIOInfoRef gpioInfo, int inputPin, int outputPin) {
    OneWireInfoRef info = OneWireInfoCreate(gpioInfo, inputPin);

    info->outputPin = outputPin;
    GPIOInfoExport(gpioInfo, outputPin);

    GPIOInfoSetMode(info->gpioInfo, inputPin, GPIO_PIN_MODE_INPUT);
    GPIOInfoSetMode(info->gpioInfo, outputPin, GPIO_PIN_MODE_OUTPUT);

    return info;
}

void OneWireInfoFree(OneWireInfoRef info) {
    GPIOInfoUnexport(info->gpioInfo, info->inputPin);
    if (info->outputPin != -1)
        GPIOInfoUnexport(info->gpioInfo, info->outputPin);
    free(info);
}

#include "delay.h"
void OneWireInfoPullUp(OneWireInfoRef info) {
    if (info->outputPin == -1) {
        GPIOInfoSetMode(info->gpioInfo, info->inputPin, GPIO_PIN_MODE_INPUT);
    } else {
        GPIOInfoSetValue(info->gpioInfo, info->outputPin, GPIO_PIN_VALUE_LOW);
    }
}

void OneWireInfoPullDown(OneWireInfoRef info) {
    if (info->outputPin == -1) {
        GPIOInfoSetMode(info->gpioInfo, info->inputPin, GPIO_PIN_MODE_OUTPUT);
        GPIOInfoSetValue(info->gpioInfo, info->inputPin, GPIO_PIN_VALUE_LOW);
    } else {
        GPIOInfoSetValue(info->gpioInfo, info->outputPin, GPIO_PIN_VALUE_HIGH);
    }
}

BOOL OneWireInfoReset(OneWireInfoRef info) {
    OneWireInfoPullUp(info);
    DelayMicro(info->delays.g);

    OneWireInfoPullDown(info);

    struct timespec time = { .tv_sec = 0, .tv_nsec = info->delays.h * 1000 };
    nanosleep(&time, NULL);

    OneWireInfoPullUp(info);
    DelayMicro(info->delays.i);

    if (GPIOInfoGetValue(info->gpioInfo, info->inputPin) == GPIO_PIN_VALUE_LOW) {
        DelayMicro(info->delays.j);
        return TRUE;
    }

    return FALSE;
}


void OneWireInfoWriteBit(OneWireInfoRef info, BOOL bit) {
    OneWireInfoPullDown(info);

    if (bit) {
        DelayMicro(info->delays.a);
        OneWireInfoPullUp(info);
        DelayMicro(info->delays.b);
    } else {
        DelayMicro(info->delays.c);
        OneWireInfoPullUp(info);
        DelayMicro(info->delays.d);
    }
}

void OneWireInfoWriteByte(OneWireInfoRef info, unsigned char value) {
    for (int position = 0 ; position < 8 ; position++)
        OneWireInfoWriteBit(info, (value & (0x1 << position)) ? TRUE : FALSE);

    struct timespec time = {.tv_sec = 0, .tv_nsec = 100000};
    nanosleep(&time, NULL);
}


BOOL OneWireInfoReadBit(OneWireInfoRef info) {
    OneWireInfoPullDown(info);

    DelayMicro(info->delays.a);
    OneWireInfoPullUp(info);
    DelayMicro(info->delays.e);

    BOOL bit = (GPIOInfoGetValue(info->gpioInfo, info->inputPin) != GPIO_PIN_VALUE_LOW);
    DelayMicro(info->delays.f);

    return bit;
}

unsigned char OneWireInfoReadByte(OneWireInfoRef info) {
    unsigned char byte = 0x0;

    for (int position = 0 ; position < 8 ; position++) {
        if (OneWireInfoReadBit(info))
            byte |= (0x1 << position);
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
Java_com_cdoapps_gpio_OneWire_configureBuffered(JNIEnv * env, jobject thiz,
                                                jobject gpio, jint inputPin, jint outputPin) {
    OneWireInfoRef info = (OneWireInfoRef)Java_java_lang_Object_getReserved(env, thiz);
    if (info)
        OneWireInfoFree(info);

    GPIOInfoRef gpioInfo = (GPIOInfoRef)Java_java_lang_Object_getReserved(env, gpio);
    Java_java_lang_Object_setReserved(env, thiz, (jlong)OneWireInfoCreateBuffered(gpioInfo,
                                                                                  inputPin,
                                                                                  outputPin));
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