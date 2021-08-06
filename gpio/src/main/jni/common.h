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

#ifndef GPIO_COMMON_H
#define GPIO_COMMON_H

#include <jni.h>
#include <android/log.h>

#define TAG "GPIO"
#define LOG_INFO(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOG_WARN(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOG_ERROR(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

#ifndef NULL
#define NULL (void *)0
#endif

#ifndef BOOL
#define BOOL unsigned char
#define FALSE (BOOL)0
#define TRUE (BOOL)1
#endif

#ifndef HUGE_VALF
static union { unsigned int x; float y; } HUGE_VAL = { .x = 0x7f800000 };
#define HUGE_VALF (HUGE_VAL.y)
#endif

static inline jlong
Java_java_lang_Object_getReserved(JNIEnv * env, jobject thiz) {
    jclass clazz = (*env)->GetObjectClass(env, thiz);
    jfieldID reservedID = (*env)->GetFieldID(env, clazz, "mReserved", "J");
    if (!reservedID)
        return 0l;

    return (*env)->GetLongField(env, thiz, reservedID);
}

static inline void
Java_java_lang_Object_setReserved(JNIEnv * env, jobject thiz, jlong value) {
    jclass clazz = (*env)->GetObjectClass(env, thiz);
    jfieldID reservedID = (*env)->GetFieldID(env, clazz, "mReserved", "J");
    if (!reservedID)
        return;

    return (*env)->SetLongField(env, thiz, reservedID, value);
}

#endif //GPIO_COMMON_H
