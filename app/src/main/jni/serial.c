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
#include "serial.h"

#include <errno.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

struct SerialInfo {
    int device;

    int baudRate;
    SerialDataBits dataBits;
    SerialParity parity;
    SerialStopBits stopBits;
};

SerialInfoRef SerialInfoCreate(const char *path, int baudRate, SerialDataBits dataBits,
                               SerialParity parity, SerialStopBits stopBits) {
    SerialInfoRef info = malloc(sizeof(struct  SerialInfo));

    info->device = open(path, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (-1 == info->device)
        LOG_ERROR("Error opening %s: %d", path, errno);
    info->baudRate = -1;
    info->dataBits = SerialDataBitsHeight;
    info->parity = SerialParityNone;
    info->stopBits = SerialStopBitsOne;
    SerialInfoConfigure(info, baudRate, dataBits, parity, stopBits);

    return info;
}

void SerialInfoFree(SerialInfoRef info) {
    if (info->device != -1)
        close(info->device);

    free(info);
}

static inline speed_t SerialInfoBaudRateToSpeed(int baudRate) {
    if (baudRate < 50)
        return B0;

    if (baudRate < 75)
        return B50;

    if (baudRate < 110)
        return B75;

    if (baudRate < 134)
        return B110;

    if (baudRate < 150)
        return B134;

    if (baudRate < 200)
        return B150;

    if (baudRate < 300)
        return B200;

    if (baudRate < 600)
        return B300;

    if (baudRate < 1200)
        return B600;

    if (baudRate < 1800)
        return B1200;

    if (baudRate < 2400)
        return B1800;

    if (baudRate < 4800)
        return B2400;

    if (baudRate < 9600)
        return B4800;

    if (baudRate < 19200)
        return B9600;

    if (baudRate < 38400)
        return B19200;

    if (baudRate < 57600)
        return B38400;

    if (baudRate < 115200)
        return B57600;

    if (baudRate < 230400)
        return B115200;

    if (baudRate < 460800)
        return B230400;

    if (baudRate < 500000)
        return B460800;

    if (baudRate < 576000)
        return B500000;

    if (baudRate < 921600)
        return B576000;

    if (baudRate < 1000000)
        return B921600;

    if (baudRate < 1152000)
        return B1000000;

    if (baudRate < 1500000)
        return B1152000;

    if (baudRate < 2000000)
        return B1500000;

    if (baudRate < 2500000)
        return B2000000;

    if (baudRate < 3000000)
        return B2500000;

    if (baudRate < 3500000)
        return B3000000;

    if (baudRate < 4000000)
        return B3500000;

    return B4000000;
}

static inline SerialDataBits SerialInfoDataBitsFromJava(JNIEnv *env, jobject dataBits) {
    jclass clazz = (*env)->FindClass(env, "com/cdoapps/gpio/Serial$DataBits");

    if (dataBits == (*env)->GetStaticObjectField(env,
                                                 clazz,
                                                 (*env)->GetStaticFieldID(
                                                         env,
                                                         clazz,
                                                         "Five",
                                                         "Lcom/cdoapps/gpio/Serial$DataBits;")))
        return SerialDataBitsFive;

    if (dataBits == (*env)->GetStaticObjectField(env,
                                                 clazz,
                                                 (*env)->GetStaticFieldID(
                                                         env,
                                                         clazz,
                                                         "Six",
                                                         "Lcom/cdoapps/gpio/Serial$DataBits;")))
        return SerialDataBitsSix;

    if (dataBits == (*env)->GetStaticObjectField(env,
                                                 clazz,
                                                 (*env)->GetStaticFieldID(
                                                         env,
                                                         clazz,
                                                         "Seven",
                                                         "Lcom/cdoapps/gpio/Serial$DataBits;")))
        return SerialDataBitsSeven;

    if (dataBits == (*env)->GetStaticObjectField(env,
                                                 clazz,
                                                 (*env)->GetStaticFieldID(
                                                         env,
                                                         clazz,
                                                         "Height",
                                                         "Lcom/cdoapps/gpio/Serial$DataBits;")))
        return SerialDataBitsHeight;

    return (SerialDataBits)-1;
}

static inline SerialParity SerialInfoParityFromJava(JNIEnv *env, jobject parity) {
    jclass clazz = (*env)->FindClass(env, "com/cdoapps/gpio/Serial$Parity");

    if (parity == (*env)->GetStaticObjectField(env,
                                                 clazz,
                                                 (*env)->GetStaticFieldID(
                                                         env,
                                                         clazz,
                                                         "None",
                                                         "Lcom/cdoapps/gpio/Serial$Parity;")))
        return SerialParityNone;

    if (parity == (*env)->GetStaticObjectField(env,
                                               clazz,
                                               (*env)->GetStaticFieldID(
                                                       env,
                                                       clazz,
                                                       "Even",
                                                       "Lcom/cdoapps/gpio/Serial$Parity;")))
        return SerialParityEven;

    if (parity == (*env)->GetStaticObjectField(env,
                                               clazz,
                                               (*env)->GetStaticFieldID(
                                                       env,
                                                       clazz,
                                                       "Odd",
                                                       "Lcom/cdoapps/gpio/Serial$Parity;")))
        return SerialParityOdd;

    return (SerialParity)-1;
}

static inline SerialStopBits SerialInfoStopBitsFromJava(JNIEnv *env, jobject stopBits) {
    jclass clazz = (*env)->FindClass(env, "com/cdoapps/gpio/Serial$StopBits");

    if (stopBits == (*env)->GetStaticObjectField(env,
                                                 clazz,
                                                 (*env)->GetStaticFieldID(
                                                         env,
                                                         clazz,
                                                         "One",
                                                         "Lcom/cdoapps/gpio/Serial$StopBits;")))
        return SerialStopBitsOne;

    if (stopBits == (*env)->GetStaticObjectField(env,
                                                 clazz,
                                                 (*env)->GetStaticFieldID(
                                                         env,
                                                         clazz,
                                                         "Two",
                                                         "Lcom/cdoapps/gpio/Serial$StopBits;")))
        return SerialStopBitsTwo;

    return (SerialStopBits)-1;
}

void SerialInfoConfigure(SerialInfoRef info, int baudRate, SerialDataBits dataBits,
                         SerialParity parity, SerialStopBits stopBits) {
    if (info->baudRate == baudRate &&
        info->dataBits == dataBits &&
        info->parity == parity &&
        info->stopBits == stopBits)
        return;

    info->baudRate = baudRate;
    info->dataBits = dataBits;
    info->parity = parity;
    info->stopBits = stopBits;

    struct termios configuration;
    if (tcgetattr(info->device, &configuration)) {
        LOG_ERROR("Unable to get configuration of serial device");
        return;
    }

    speed_t speed = SerialInfoBaudRateToSpeed(info->baudRate);
    cfmakeraw(&configuration);
    cfsetispeed(&configuration, speed);
    cfsetospeed(&configuration, speed);

    if (SerialDataBitsHeight != info->dataBits) {
        configuration.c_cflag &= ~CS8;
        switch (info->dataBits) {
            case SerialDataBitsFive:
                configuration.c_cflag |= ~CS5;
                break;

            case SerialDataBitsSix:
                configuration.c_cflag |= ~CS6;
                break;

            case SerialDataBitsSeven:
                configuration.c_cflag |= ~CS7;
                break;

            default:
                // not acceptable, put back 8 data bits
                configuration.c_cflag |= CS8;
                break;
        }
    }

    if (SerialParityNone != info->parity) {
        configuration.c_cflag |= PARENB;
        switch (info->parity) {
            case SerialParityOdd:
                configuration.c_cflag |= PARODD;
                break;

            case SerialParityEven:
                break;

            default:
                // not acceptable, disable parity
                configuration.c_cflag &= ~PARENB;
                break;
        }
    }

    switch (info->stopBits) {
        case SerialStopBitsOne:
            configuration.c_cflag &= ~CSTOPB;
            break;

        case SerialStopBitsTwo:
            configuration.c_cflag |= CSTOPB;
            break;

        default:
            break;
    }

    if (tcsetattr(info->device, TCSANOW, &configuration))
        LOG_ERROR("Unable to set configuration of serial device");
}

int SerialInfoWrite(SerialInfoRef info, const char *buffer, int size) {
    return write(info->device, buffer, size);
}

int SerialInfoRead(SerialInfoRef info, char *buffer, int size) {
    return read(info->device, buffer, size);
}

static inline jstring
Java_com_cdoapps_gpio_Serial_getPath(JNIEnv * env, jobject thiz) {
    jclass clazz = (*env)->GetObjectClass(env, thiz);
    jfieldID pathID = (*env)->GetFieldID(env, clazz, "path", "Ljava/lang/String;");
    if (!pathID)
        return NULL;

    return (*env)->GetObjectField(env, thiz, pathID);
}

static inline void
Java_com_cdoapps_gpio_Serial_setInputStream(JNIEnv * env, jobject thiz, jobject inputStream) {
    jclass clazz = (*env)->GetObjectClass(env, thiz);
    jfieldID inputStreamID = (*env)->GetFieldID(env, clazz, "inputStream", "Ljava/io/InputStream;");
    if (!inputStreamID)
        return;

    (*env)->SetObjectField(env, thiz, inputStreamID, inputStream);
}

static inline void
Java_com_cdoapps_gpio_Serial_setOutputStream(JNIEnv * env, jobject thiz, jobject outputStream) {
    jclass clazz = (*env)->GetObjectClass(env, thiz);
    jfieldID outputStreamID = (*env)->GetFieldID(env,
                                                 clazz,
                                                 "outputStream",
                                                 "Ljava/io/OutputStream;");
    if (!outputStreamID)
        return;

    (*env)->SetObjectField(env, thiz, outputStreamID, outputStream);
}


JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_Serial_configure(JNIEnv * env, jobject thiz, jint baudRate,
                                       jobject dataBits, jobject parity,
                                       jobject stopBits) {
    SerialInfoRef info = (SerialInfoRef )Java_java_lang_Object_getReserved(env, thiz);

    SerialDataBits serialDataBits = SerialInfoDataBitsFromJava(env, dataBits);
    SerialParity serialParity = SerialInfoParityFromJava(env, parity);
    SerialStopBits serialStopBits = SerialInfoStopBitsFromJava(env, stopBits);

    if (info) {
        SerialInfoConfigure(info,
                            baudRate,
                            serialDataBits,
                            serialParity,
                            serialStopBits);
        return;
    }

    jstring path = Java_com_cdoapps_gpio_Serial_getPath(env, thiz);
    const char *utf8Path = (*env)->GetStringUTFChars(env, path, NULL);

    info = SerialInfoCreate(utf8Path,
                            baudRate,
                            serialDataBits,
                            serialParity,
                            serialStopBits);
    (*env)->ReleaseStringUTFChars(env, path, utf8Path);

    Java_java_lang_Object_setReserved(env, thiz, (jlong)info);

    jclass fileDescriptorClazz = (*env)->FindClass(env, "java/io/FileDescriptor");
    jmethodID fileDescriptorConstructor = (*env)->GetMethodID(env,
                                                              fileDescriptorClazz,
                                                              "<init>",
                                                              "()V");

    jfieldID descriptorID = (*env)->GetFieldID(env, fileDescriptorClazz, "descriptor", "I");
    if (!descriptorID)
        descriptorID = (*env)->GetFieldID(env, fileDescriptorClazz, "fd", "I");

    if (!descriptorID)
        return;

    jobject fileDescriptor = (*env)->NewObject(env, fileDescriptorClazz, fileDescriptorConstructor);
    (*env)->SetIntField(env, fileDescriptor, descriptorID, info->device);

    jclass inputStreamClazz = (*env)->FindClass(env, "java/io/FileInputStream");
    jmethodID inputStreamConstructor = (*env)->GetMethodID(env,
                                                           inputStreamClazz,
                                                           "<init>",
                                                           "(Ljava/io/FileDescriptor;)V");
    jobject inputStream = (*env)->NewObject(env,
                                            inputStreamClazz,
                                            inputStreamConstructor,
                                            fileDescriptor);
    Java_com_cdoapps_gpio_Serial_setInputStream(env, thiz, inputStream);

    jclass outputStreamClazz = (*env)->FindClass(env, "java/io/FileOutputStream");
    jmethodID outputStreamConstructor = (*env)->GetMethodID(env,
                                                           outputStreamClazz,
                                                           "<init>",
                                                           "(Ljava/io/FileDescriptor;)V");
    jobject outputStream = (*env)->NewObject(env,
                                            outputStreamClazz,
                                            outputStreamConstructor,
                                            fileDescriptor);
    Java_com_cdoapps_gpio_Serial_setOutputStream(env, thiz, outputStream);
}

JNIEXPORT void JNICALL
Java_com_cdoapps_gpio_Serial_destroy(JNIEnv * env, jobject thiz) {
    SerialInfoRef info = (SerialInfoRef )Java_java_lang_Object_getReserved(env, thiz);
    if (info) {
        SerialInfoFree(info);
        Java_java_lang_Object_setReserved(env, thiz, 0l);
    }
}