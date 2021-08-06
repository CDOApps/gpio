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

#ifndef GPIO_SERIAL_H
#define GPIO_SERIAL_H

/**
 * The {@code SerialDataBits} enum represents the number of data bits per sent character.
 */
typedef enum {
    /**
     * 5 bits
     */
    SerialDataBitsFive,

    /**
     * 6 bits
     */
    SerialDataBitsSix,

    /**
     * 7 bits
     */
    SerialDataBitsSeven,

    /**
     * 8 bits
     */
    SerialDataBitsHeight
} SerialDataBits;

/**
 * The {@code SerialParity} enum represents the rule used to check for errors in one sent character.
 */
typedef enum {
    /**
     * No bit added.
     */
    SerialParityNone,

    /**
     * One bit added, set to 1 if the character is odd or 0 otherwise.
     */
    SerialParityOdd,

    /**
     * One bit added, set to 1 if the character is even or 0 otherwise.
     */
    SerialParityEven
} SerialParity;

/**
 * The {@code SerialStopBits} enum represents the number of bits added to detect the end of a
 * character.
 */
typedef enum {
    /**
     * 1 bit
     */
    SerialStopBitsOne,

    /**
     * 2 bits
     */
    SerialStopBitsTwo
} SerialStopBits;

/**
 * The {@code SerialInfo} struct provides control over a serial bus.
 */
typedef struct SerialInfo *SerialInfoRef;

/**
 * Returns a {@code SerialInfo} object representing a new serial bus that will communicate over the
 * given port.
 *
 * @param path the file path of the serial port which will initialize the serial communications.
 * @param baudRate the data rate/speed in bps (bits per second).
 * @param dataBits the number of bits per character.
 * @param parity the rule of the check bit added to one character if any.
 * @param stopBits the number of stop bits added to one character.
 * @return a {@code SerialInfo} object representing a new serial bus.
 */
SerialInfoRef SerialInfoCreate(const char *path, int baudRate, SerialDataBits dataBits,
                               SerialParity parity, SerialStopBits stopBits);
/**
 * Destroys the resources associated to a serial bus.
 *
 * @param info a {@code SerialInfo} object representing the serial bus to destroy.
 */
void SerialInfoFree(SerialInfoRef info);

/**
 * Initializes the serial communications.
 *
 * @param info a {@code SerialInfo} object representing the serial bus.
 * @param baudRate the data rate/speed in bps (bits per second).
 * @param dataBits the number of bits per character.
 * @param parity the rule of the check bit added to one character if any.
 * @param stopBits the number of stop bits added to one character.
 */
void SerialInfoConfigure(SerialInfoRef info, int baudRate, SerialDataBits dataBits,
                         SerialParity parity, SerialStopBits stopBits);

/**
 * Send a message to a device connected to the serial bus.
 *
 * The returned value may be less than {@code size}.
 *
 * @param info a {@code SerialInfo} object representing the serial bus.
 * @param buffer the characters to write.
 * @param size the number of characters to write.
 * @return the number of written characters.
 */
int SerialInfoWrite(SerialInfoRef info, const char *buffer, int size);
/**
 * Receive a message from a device connected to the serial bus.
 *
 * The returned value may be less than {@code size}.
 *
 * @param info a {@code SerialInfo} object representing the serial bus.
 * @param buffer the memory where the read characters will be written.
 * @param size the maximum number of characters to read.
 * @return the number of read characters.
 */
int SerialInfoRead(SerialInfoRef info, char *buffer, int size);

#endif //GPIO_SERIAL_H
