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
#include "thermometer.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct ThermometerInfo {
    OneWireInfoRef oneWireInfo;

    char rom[17];
    ThermometerFamily family;
    BOOL parasiticPowerMode;
};

static const unsigned char THERMOMETER_CRC_TABLE[] = {
        0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
        157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
        35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
        190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
        70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
        219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
        101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
        248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
        140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
        17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
        175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
        50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
        202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
        87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
        233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
        116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53
};

BOOL ThermometerInfoCheckCRC(unsigned char *data, int size) {
    unsigned char crc = 0;

    for (int index = 0 ; index < size ; index++)
        crc = THERMOMETER_CRC_TABLE[crc ^ data[index]];

    return (0x0 == crc);
}

static inline BOOL ThermometerInfoGetRomBit(unsigned long long *rom, char position) {
    return (*rom & (0x1ull << position)) ? TRUE : FALSE;
}

static inline void ThermometerInfoSetRomBit(unsigned long long *rom, char position, BOOL bit) {
    if (bit)
        *rom |= (0x1ull << position);
    else
        *rom &= ~(0x1ull << position);
}

#define THERMOMETER_SEARCH_ROM_COMMAND 0xF0
#define THERMOMETER_SEARCH_RESULT_LEAF 0
#define THERMOMETER_SEARCH_RESULT_NODE 1
#define THERMOMETER_SEARCH_ERROR_NO_RESET -1
#define THERMOMETER_SEARCH_ERROR_NO_MATCH -2

int ThermometerInfoSearch(OneWireInfoRef oneWireInfo, unsigned long long *rom, int *lastPosition) {
    if (*lastPosition < 0)
        return THERMOMETER_SEARCH_RESULT_LEAF;

    if (*lastPosition < 64) {
        ThermometerInfoSetRomBit(rom, *lastPosition, TRUE);

        for (int position = *lastPosition + 1 ; position < 64 ; position++)
            ThermometerInfoSetRomBit(rom, position, FALSE);
    }

    *lastPosition = -1;

    if (!OneWireInfoReset(oneWireInfo))
        return THERMOMETER_SEARCH_ERROR_NO_RESET;

    BOOL bits[] = {0x0, 0x0};
    OneWireInfoWriteByte(oneWireInfo, THERMOMETER_SEARCH_ROM_COMMAND);

    for (int position = 0 ; position < 64 ; position++) {
        bits[0] = OneWireInfoReadBit(oneWireInfo);
        bits[1] = OneWireInfoReadBit(oneWireInfo);

        if (bits[0] && bits[1])
            return THERMOMETER_SEARCH_ERROR_NO_MATCH;

        if (!bits[0] && !bits[1]) {
            if (ThermometerInfoGetRomBit(rom, position)) {
                OneWireInfoWriteBit(oneWireInfo, TRUE);
            } else {
                *lastPosition = position;
                OneWireInfoWriteBit(oneWireInfo, FALSE);
            }
        } else if (!bits[0]) {
            OneWireInfoWriteBit(oneWireInfo, FALSE);
            ThermometerInfoSetRomBit(rom, position, FALSE);
        } else {
            OneWireInfoWriteBit(oneWireInfo, TRUE);
            ThermometerInfoSetRomBit(rom, position,TRUE);
        }
    }

    return THERMOMETER_SEARCH_RESULT_NODE;
}

BOOL ThermometerInfoReadPowerSupply(ThermometerInfoRef info);
void ThermometerInfoList(OneWireInfoRef oneWireInfo, StackRef stack) {
    if (!stack)
        return;

    unsigned long long previousRom = 0x0;
    int previousPosition = 64;
    int retry = 0;

    char buf[17] = "";

    while (retry < 10) {
        unsigned long long rom = previousRom;
        int position = previousPosition;

        switch (ThermometerInfoSearch(oneWireInfo, &rom, &position)) {
            case THERMOMETER_SEARCH_RESULT_LEAF:
                retry = 10;
                break;

            case THERMOMETER_SEARCH_RESULT_NODE:
                if (ThermometerInfoCheckCRC((unsigned char *) &rom, 8)) {
                    previousRom = rom;
                    previousPosition = position;

                    sprintf(buf, "%016llx", rom);
                    ThermometerInfoRef info = ThermometerInfoCreate(oneWireInfo, buf, FALSE);

                    if (info)
                        StackPush(stack, info);
                } else {
                    retry++;
                }
                break;

            default:
                retry++;
                break;
        }
    }

    const ThermometerInfoRef *info = StackGetBaseAddress(stack);
    unsigned int length = StackLength(stack);

    for (unsigned int index = 0 ; index < length ; index++)
        ThermometerInfoReadPowerSupply(info[index]);
}

ThermometerInfoRef ThermometerInfoCreate(OneWireInfoRef oneWireInfo, const char *rom,
                               BOOL parasiticPowerMode) {
    int byte = 0x0;
    sscanf(&rom[14], "%02x", &byte);

    ThermometerFamily family = ThermometerFamilyUnknown;
    switch (byte) {
        case 0x10:
            family = ThermometerFamilyDS18S20;
            break;

        case 0x28:
            family = ThermometerFamilyDS18B20;
            break;

        default:
            return NULL;
    }

    ThermometerInfoRef info = malloc(sizeof(struct ThermometerInfo));

    info->oneWireInfo = oneWireInfo;
    strcpy(info->rom, rom);
    info->family = family;
    info->parasiticPowerMode = parasiticPowerMode;

    return info;
}

void ThermometerInfoFree(ThermometerInfoRef info) {
    free(info);
}

const char *ThermometerInfoGetRom(ThermometerInfoRef info) {
    return info->rom;
}

ThermometerFamily ThermometerInfoGetFamily(ThermometerInfoRef info) {
    return info->family;
}

BOOL ThermometerInfoUsesParasiticPowerMode(ThermometerInfoRef info) {
    return info->parasiticPowerMode;
}

#define THERMOMETER_SELECT_COMMAND 0x55

void ThermometerInfoSelect(ThermometerInfoRef info) {
    unsigned long long rom = 0x0;
    sscanf(info->rom, "%016llx", &rom);

    OneWireInfoWriteByte(info->oneWireInfo, THERMOMETER_SELECT_COMMAND);

    for (int position = 0 ; position < 64 ; position++)
        OneWireInfoWriteBit(info->oneWireInfo, ThermometerInfoGetRomBit(&rom, position));
}

#include <time.h>

#define THERMOMETER_SKIP_ROM_COMMAND 0xCC
#define THERMOMETER_CONVERT_T_COMMAND 0x44

void ThermometerInfoConvert(OneWireInfoRef oneWireInfo, BOOL parasiticPowerMode) {
    if (!OneWireInfoReset(oneWireInfo))
        return;

    OneWireInfoWriteByte(oneWireInfo, THERMOMETER_SKIP_ROM_COMMAND);
    OneWireInfoWriteByte(oneWireInfo, THERMOMETER_CONVERT_T_COMMAND);

    if (parasiticPowerMode) {
        struct timespec time = { .tv_sec = 1, .tv_nsec = 0 };
        nanosleep(&time, NULL);
    } else {
        struct timespec time = { .tv_sec = 0, .tv_nsec = 500000000 };
        nanosleep(&time, NULL);

        do {} while (!OneWireInfoReadBit(oneWireInfo));
    }
}

#define THERMOMETER_READ_SCRATCHPAD_COMMAND 0xBE

void ThermometerInfoReadScratchpad(ThermometerInfoRef info, unsigned char *scratchpad) {
    ThermometerInfoSelect(info);
    OneWireInfoWriteByte(info->oneWireInfo, THERMOMETER_READ_SCRATCHPAD_COMMAND);

    for (int position = 0 ; position < 9 ; position++)
        scratchpad[position] = OneWireInfoReadByte(info->oneWireInfo);
}

float ThermometerInfoGetTemperature(ThermometerInfoRef info) {
    if (!OneWireInfoReset(info->oneWireInfo))
        return HUGE_VALF;

    unsigned char scratchpad[] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
    ThermometerInfoReadScratchpad(info, scratchpad);

    if (!ThermometerInfoCheckCRC(scratchpad, 9))
        return HUGE_VALF;

    float value = 0.f;
    short temperature = scratchpad[0];
    unsigned char sign = scratchpad[1];

    switch (info->family) {
        case ThermometerFamilyDS18S20: {
            unsigned char remain = scratchpad[6];
            unsigned char cpc = scratchpad[7];

            if (sign)
                temperature = -((0x100 - temperature) >> 1);
            else
                temperature >>= 1;

            value = (float)(temperature - .25f + (float)(cpc - remain)/(float)cpc);
            break;
        }

        case ThermometerFamilyDS18B20:
            temperature = (sign << 8) | temperature;

            value = temperature / 16.f;
            temperature >>= 4;
            break;

        default:
            break;
    }

    if (0x55 == temperature)
        return HUGE_VALF;

    return value;
}

#define THERMOMETER_READ_POWER_SUPPLY_COMMAND 0xB4

BOOL ThermometerInfoReadPowerSupply(ThermometerInfoRef info) {
    if (!OneWireInfoReset(info->oneWireInfo))
        return TRUE;

    ThermometerInfoSelect(info);
    OneWireInfoWriteByte(info->oneWireInfo, THERMOMETER_READ_POWER_SUPPLY_COMMAND);

    info->parasiticPowerMode = OneWireInfoReadBit(info->oneWireInfo) ? FALSE : TRUE;
    return info->parasiticPowerMode;
}

JNIEXPORT jobject JNICALL
Java_com_cdoapps_gpio_Thermometer_listAll(JNIEnv *env, jclass clazz, jobject bus) {
    StackRef stack = StackCreate(FALSE);
    OneWireInfoRef oneWireInfo = (OneWireInfoRef)Java_java_lang_Object_getReserved(env, bus);
    if (!oneWireInfo)
        return NULL;

    ThermometerInfoList(oneWireInfo, stack);

    jclass listClazz = (*env)->FindClass(env, "java/util/ArrayList");
    jmethodID listConstructor = (*env)->GetMethodID(env, listClazz, "<init>", "()V");
    jobject list = (*env)->NewObject(env, listClazz, listConstructor);

    jmethodID add = (*env)->GetMethodID(env, listClazz, "add", "(Ljava/lang/Object;)Z");
    jmethodID thermometerConstructor = (*env)->GetMethodID(env, clazz, "<init>", "()V");

    ThermometerInfoRef info = NULL;
    while ((info = StackPop(stack))) {
        jobject thiz = (*env)->NewObject(env, clazz, thermometerConstructor);
        (*env)->CallBooleanMethod(env, list, add, thiz);
        Java_java_lang_Object_setReserved(env, thiz, (jlong)info);
    }

    StackFree(stack);

    return list;
}

JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_Thermometer_destroy(JNIEnv * env, jobject thiz) {
    ThermometerInfoRef info = (ThermometerInfoRef)Java_java_lang_Object_getReserved(env, thiz);
    if (info) {
        ThermometerInfoFree(info);
        Java_java_lang_Object_setReserved(env, thiz, 0l);
    }
}

JNIEXPORT jstring JNICALL
Java_com_cdoapps_gpio_Thermometer_getRom(JNIEnv *env, jobject thiz) {
    ThermometerInfoRef info = (ThermometerInfoRef)Java_java_lang_Object_getReserved(env, thiz);
    if (info)
        return (*env)->NewStringUTF(env, ThermometerInfoGetRom(info));

    return NULL;
}

JNIEXPORT jobject JNICALL
Java_com_cdoapps_gpio_Thermometer_getFamily(JNIEnv *env, jobject thiz) {
    ThermometerInfoRef info = (ThermometerInfoRef)Java_java_lang_Object_getReserved(env, thiz);
    if (!info)
        return NULL;

    jclass clazz = (*env)->FindClass(env, "com/cdoapps/gpio/Thermometer$Family");

    switch (ThermometerInfoGetFamily(info)) {
        case ThermometerFamilyDS18S20: {
            jfieldID fieldID = (*env)->GetStaticFieldID(
                    env,
                    clazz ,
                    "DS18S20",
                    "Lcom/cdoapps/gpio/Thermometer$Family;");

            return (*env)->GetStaticObjectField(env, clazz, fieldID);
        }

        case ThermometerFamilyDS18B20: {
            jfieldID fieldID = (*env)->GetStaticFieldID(
                    env,
                    clazz ,
                    "DS18B20",
                    "Lcom/cdoapps/gpio/Thermometer$Family;");

            return (*env)->GetStaticObjectField(env, clazz, fieldID);
        }

        default:
            break;
    }

    return NULL;
}

JNIEXPORT jboolean JNICALL
Java_com_cdoapps_gpio_Thermometer_usesParasiticPowerMode(JNIEnv *env, jobject thiz) {
    ThermometerInfoRef info = (ThermometerInfoRef)Java_java_lang_Object_getReserved(env, thiz);
    if (info && ThermometerInfoUsesParasiticPowerMode(info))
        return JNI_TRUE;

    return JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_Thermometer_convert(JNIEnv * env, jclass clazz, jobject bus,
                                     jboolean parasiticPowerMode) {
    OneWireInfoRef oneWireInfo = (OneWireInfoRef)Java_java_lang_Object_getReserved(env, bus);
    if (oneWireInfo)
        ThermometerInfoConvert(oneWireInfo, parasiticPowerMode);
}

JNIEXPORT jfloat JNICALL
Java_com_cdoapps_gpio_Thermometer_getTemperature(JNIEnv *env, jobject thiz) {
    ThermometerInfoRef info = (ThermometerInfoRef)Java_java_lang_Object_getReserved(env, thiz);
    if (info)
        return (jfloat)ThermometerInfoGetTemperature(info);

    return HUGE_VALF;
}