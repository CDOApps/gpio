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
#include "dallas.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct DallasInfo {
    OneWireInfoRef oneWireInfo;

    char rom[17];
    DallasSensorFamily family;
    BOOL parasiticPowerMode;
};

static unsigned char DALLAS_CRC_TABLE[] = {
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

BOOL DallasInfoCheckCRC(unsigned char *data, int size) {
    unsigned char crc = 0;

    for (int index = 0 ; index < size ; index++)
        crc = DALLAS_CRC_TABLE[crc ^ data[index]];

    return (0x0 == crc);
}

static inline BOOL DallasInfoGetRomBit(unsigned long long *rom, char position) {
    return (*rom & (0x1ull << position)) ? TRUE : FALSE;
}

static inline void DallasInfoSetRomBit(unsigned long long *rom, char position, BOOL bit) {
    if (bit)
        *rom |= (0x1ull << position);
    else
        *rom &= ~(0x1ull << position);
}

#define DALLAS_SEARCH_ROM_COMMAND 0xF0
#define DALLAS_SEARCH_SENSOR_RESULT_LEAF 0
#define DALLAS_SEARCH_SENSOR_RESULT_NODE 1
#define DALLAS_SEARCH_SENSOR_ERROR_NO_RESET -1
#define DALLAS_SEARCH_SENSOR_ERROR_NO_MATCH -2

int DallasInfoSearchSensor(OneWireInfoRef oneWireInfo, unsigned long long *rom, int *lastPosition) {
    if (*lastPosition < 0)
        return DALLAS_SEARCH_SENSOR_RESULT_LEAF;

    if (*lastPosition < 64) {
        DallasInfoSetRomBit(rom, *lastPosition, TRUE);

        for (int position = *lastPosition + 1 ; position < 64 ; position++)
            DallasInfoSetRomBit(rom, position, FALSE);
    }

    *lastPosition = -1;

    if (!OneWireInfoReset(oneWireInfo))
        return DALLAS_SEARCH_SENSOR_ERROR_NO_RESET;

    BOOL bits[] = {0x0, 0x0};
    OneWireInfoWriteByte(oneWireInfo, DALLAS_SEARCH_ROM_COMMAND);

    for (int position = 0 ; position < 64 ; position++) {
        bits[0] = OneWireInfoReadBit(oneWireInfo);
        bits[1] = OneWireInfoReadBit(oneWireInfo);

        if (bits[0] && bits[1])
            return DALLAS_SEARCH_SENSOR_ERROR_NO_MATCH;

        if (!bits[0] && !bits[1]) {
            if (DallasInfoGetRomBit(rom, position)) {
                OneWireInfoWriteBit(oneWireInfo, TRUE);
            } else {
                *lastPosition = position;
                OneWireInfoWriteBit(oneWireInfo, FALSE);
            }
        } else if (!bits[0]) {
            OneWireInfoWriteBit(oneWireInfo, FALSE);
            DallasInfoSetRomBit(rom, position, FALSE);
        } else {
            OneWireInfoWriteBit(oneWireInfo, TRUE);
            DallasInfoSetRomBit(rom, position,TRUE);
        }
    }

    return DALLAS_SEARCH_SENSOR_RESULT_NODE;
}

BOOL DallasInfoReadPowerSupply(DallasInfoRef info);
void DallasInfoList(OneWireInfoRef oneWireInfo, StackRef stack) {
    if (!stack)
        return;

    unsigned long long previousRom = 0x0;
    int previousPosition = 64;
    int retry = 0;

    char buf[17] = "";

    while (retry < 10) {
        unsigned long long rom = previousRom;
        int position = previousPosition;

        switch (DallasInfoSearchSensor(oneWireInfo, &rom, &position)) {
            case DALLAS_SEARCH_SENSOR_RESULT_LEAF:
                retry = 10;
                break;

            case DALLAS_SEARCH_SENSOR_RESULT_NODE:
                if (DallasInfoCheckCRC((unsigned char *) &rom, 8)) {
                    previousRom = rom;
                    previousPosition = position;

                    sprintf(buf, "%016llx", rom);
                    DallasInfoRef info = DallasInfoCreate(oneWireInfo, buf, FALSE);

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

    const DallasInfoRef *info = StackGetBaseAddress(stack);
    unsigned int length = StackLength(stack);

    for (unsigned int index = 0 ; index < length ; index++)
        DallasInfoReadPowerSupply(info[index]);
}

DallasInfoRef DallasInfoCreate(OneWireInfoRef oneWireInfo, const char *rom,
                               BOOL parasiticPowerMode) {
    int byte = 0x0;
    sscanf(&rom[14], "%02x", &byte);

    DallasSensorFamily family = DallasSensorFamilyUnknown;
    switch (byte) {
        case 0x10:
            family = DallasSensorFamilyDS18S20;
            break;

        case 0x28:
            family = DallasSensorFamilyDS18B20;
            break;

        default:
            return NULL;
    }

    DallasInfoRef info = malloc(sizeof(struct DallasInfo));

    info->oneWireInfo = oneWireInfo;
    strcpy(info->rom, rom);
    info->family = family;
    info->parasiticPowerMode = parasiticPowerMode;

    return info;
}

void DallasInfoFree(DallasInfoRef info) {
    free(info);
}

const char *DallasInfoGetRom(DallasInfoRef info) {
    return info->rom;
}

DallasSensorFamily DallasInfoGetFamily(DallasInfoRef info) {
    return info->family;
}

BOOL DallasInfoUsesParasiticPowerMode(DallasInfoRef info) {
    return info->parasiticPowerMode;
}

#define DALLAS_SELECT_COMMAND 0x55

void DallasInfoSelect(DallasInfoRef info) {
    unsigned long long rom = 0x0;
    sscanf(info->rom, "%016llx", &rom);

    OneWireInfoWriteByte(info->oneWireInfo, DALLAS_SELECT_COMMAND);

    for (int position = 0 ; position < 64 ; position++)
        OneWireInfoWriteBit(info->oneWireInfo, DallasInfoGetRomBit(&rom, position));
}

#include <time.h>

#define DALLAS_SKIP_ROM_COMMAND 0xCC
#define DALLAS_CONVERT_T_COMMAND 0x44

void DallasInfoConvert(OneWireInfoRef oneWireInfo, BOOL parasiticPowerMode) {
    if (!OneWireInfoReset(oneWireInfo))
        return;

    OneWireInfoWriteByte(oneWireInfo, DALLAS_SKIP_ROM_COMMAND);
    OneWireInfoWriteByte(oneWireInfo, DALLAS_CONVERT_T_COMMAND);

    if (parasiticPowerMode) {
        struct timespec time = { .tv_sec = 1, .tv_nsec = 0 };
        nanosleep(&time, NULL);
    } else {
        do {} while (!OneWireInfoReadBit(oneWireInfo));
    }
}

#define DALLAS_READ_SCRATCHPAD_COMMAND 0xBE

void DallasInfoReadScratchpad(DallasInfoRef info, unsigned char *scratchpad) {
    DallasInfoSelect(info);
    OneWireInfoWriteByte(info->oneWireInfo, DALLAS_READ_SCRATCHPAD_COMMAND);

    for (int position = 0 ; position < 9 ; position++)
        scratchpad[position] = OneWireInfoReadByte(info->oneWireInfo);
}

float DallasInfoGetTemperature(DallasInfoRef info) {
    if (!OneWireInfoReset(info->oneWireInfo))
        return HUGE_VALF;

    unsigned char scratchpad[] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
    DallasInfoReadScratchpad(info, scratchpad);

    if (!DallasInfoCheckCRC(scratchpad, 9))
        return HUGE_VALF;

    float value = 0.f;
    short temperature = scratchpad[0];
    unsigned char sign = scratchpad[1];

    switch (info->family) {
        case DallasSensorFamilyDS18S20: {
            unsigned char remain = scratchpad[6];
            unsigned char cpc = scratchpad[7];

            if (sign)
                temperature = -((0x100 - temperature) >> 1);
            else
                temperature >>= 1;

            value = (float)(temperature - .25f + (float)(cpc - remain)/(float)cpc);
            break;
        }

        case DallasSensorFamilyDS18B20:
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

#define DALLAS_READ_POWER_SUPPLY_COMMAND 0xB4

BOOL DallasInfoReadPowerSupply(DallasInfoRef info) {
    if (!OneWireInfoReset(info->oneWireInfo))
        return TRUE;

    DallasInfoSelect(info);
    OneWireInfoWriteByte(info->oneWireInfo, DALLAS_READ_POWER_SUPPLY_COMMAND);

    info->parasiticPowerMode = OneWireInfoReadBit(info->oneWireInfo) ? FALSE : TRUE;
    return info->parasiticPowerMode;
}

#include "map.h"

static MapRef dallas = NULL;

JNIEXPORT jint JNICALL
Java_com_cdoapps_gpio_Dallas_getHashCode(JNIEnv * env, jobject thiz) {
    static jmethodID hashCodeMethodID = 0;

    if (!hashCodeMethodID) {
        jclass clazz = (*env)->GetObjectClass(env, thiz);
        hashCodeMethodID = (*env)->GetMethodID(env, clazz, "hashCode", "()I");
    }

    return (*env)->CallIntMethod(env, thiz, hashCodeMethodID);
}

JNIEXPORT jlong JNICALL
Java_com_cdoapps_gpio_Dallas_getInfo(JNIEnv * env, jobject thiz) {
    return (jlong)MapGet(dallas, Java_com_cdoapps_gpio_Dallas_getHashCode(env, thiz));
}

JNIEXPORT jlong JNICALL
Java_com_cdoapps_gpio_OneWire_getInfo(JNIEnv * env, jobject thiz);

JNIEXPORT jobject JNICALL
Java_com_cdoapps_gpio_Dallas_listSensors(JNIEnv *env, jclass clazz, jobject bus) {
    StackRef stack = StackCreate(FALSE);
    DallasInfoList(Java_com_cdoapps_gpio_OneWire_getInfo(env, bus), stack);

    jclass listClazz = (*env)->FindClass(env, "java/util/ArrayList");
    jmethodID listConstructor = (*env)->GetMethodID(env, listClazz, "<init>", "()V");
    jobject list = (*env)->NewObject(env, listClazz, listConstructor);

    jmethodID add = (*env)->GetMethodID(env, listClazz, "add", "(Ljava/lang/Object;)Z");
    jmethodID dallasConstructor = (*env)->GetMethodID(env, clazz, "<init>", "()V");

    DallasInfoRef info = NULL;
    while ((info = StackPop(stack))) {
        jobject thiz = (*env)->NewObject(env, clazz, dallasConstructor);
        (*env)->CallBooleanMethod(env, list, add, thiz);

        if (!dallas)
            dallas = MapIntCreate();

        MapSet(dallas, Java_com_cdoapps_gpio_Dallas_getHashCode(env, thiz), info);
    }

    StackFree(stack);

    return list;
}

JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_Dallas_destroy(JNIEnv * env, jobject thiz) {
    DallasInfoFree(MapRemove(dallas, Java_com_cdoapps_gpio_Dallas_getHashCode(env, thiz)));

    if (MapIsEmpty(dallas)) {
        MapFree(dallas);
        dallas = NULL;
    }
}

JNIEXPORT jstring JNICALL
Java_com_cdoapps_gpio_Dallas_getRom(JNIEnv *env, jobject thiz) {
    return (*env)->NewStringUTF(env, DallasInfoGetRom(
            Java_com_cdoapps_gpio_Dallas_getInfo(env, thiz)));
}

JNIEXPORT jobject JNICALL
Java_com_cdoapps_gpio_Dallas_getFamily(JNIEnv *env, jobject thiz) {
    jclass clazz = (*env)->FindClass(env, "com/cdoapps/gpio/Dallas$SensorFamily");

    switch (DallasInfoGetFamily(Java_com_cdoapps_gpio_Dallas_getInfo(env, thiz))) {
        case DallasSensorFamilyDS18S20: {
            jfieldID fieldID = (*env)->GetStaticFieldID(
                    env,
                    clazz ,
                    "DS18S20",
                    "Lcom/cdoapps/gpio/Dallas$SensorFamily;");

            return (*env)->GetStaticObjectField(env, clazz, fieldID);
        }

        case DallasSensorFamilyDS18B20: {
            jfieldID fieldID = (*env)->GetStaticFieldID(
                    env,
                    clazz ,
                    "DS18B20",
                    "Lcom/cdoapps/gpio/Dallas$SensorFamily;");

            return (*env)->GetStaticObjectField(env, clazz, fieldID);
        }

        default:
            break;
    }

    return NULL;
}

JNIEXPORT jboolean JNICALL
Java_com_cdoapps_gpio_Dallas_usesParasiticPowerMode(JNIEnv *env, jobject thiz) {
    return DallasInfoUsesParasiticPowerMode(Java_com_cdoapps_gpio_Dallas_getInfo(env, thiz)) ?
    JNI_TRUE :
    JNI_FALSE;
}

JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_Dallas_convert(JNIEnv * env, jclass clazz, jobject bus,
                                     jboolean parasiticPowerMode) {
    DallasInfoConvert(Java_com_cdoapps_gpio_OneWire_getInfo(env, bus), parasiticPowerMode);
}

JNIEXPORT jfloat JNICALL
Java_com_cdoapps_gpio_Dallas_getTemperature(JNIEnv *env, jobject thiz) {
    return (jfloat)DallasInfoGetTemperature(Java_com_cdoapps_gpio_Dallas_getInfo(env, thiz));
}