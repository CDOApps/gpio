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

#ifndef GPIO_ONEWIRE_H
#define GPIO_ONEWIRE_H

#include "gpio.h"

/**
 * The {@code OneWireInfo} struct represents a 1-Wire bus communicating over one pin of a
 * {@code GPIOInfo} instance. It also presents a buffered version which uses two pins (input/output)
 * to allow communication over long line 1-wire network.
 *
 * This implementation of the 1-Wire bus uses bit banging and spinning, thus it may produce high
 * CPU load.
 *
 * Specification:
 * <a href="https://ww1.microchip.com/downloads/en/appnotes/01199a.pdf">1-Wire Protocol</a>
 */
typedef struct OneWireInfo *OneWireInfoRef;

/**
 * Returns a {@code OneWireInfo} object representing a 1-Wire bus which is initialized to
 * communicate over one pin.
 *
 * @param gpioInfo a {@code GPIOInfo} instance used to initialize and communicate with the 1-Wire
 *                 bus.
 * @param pin the WiringPi address of the pin which will be exported for the 1-Wire bus.
 * @return a {@code OneWireInfo} object representing a 1-Wire bus which is initialized to
 *         communicate over one pin.
 */
OneWireInfoRef OneWireInfoCreate(GPIOInfoRef gpioInfo, int pin);
/**
 * Returns a {@code OneWireInfo} object representing a 1-Wire bus which is initialized to
 * communicate over two pins.
 *
 * @param gpioInfo a {@code GPIOInfo} instance used to initialize and communicate with the 1-Wire
 *                 bus.
 * @param inputPin the WiringPi address of the pin which will be read for the 1-Wire bus.
 * @param outputPin the WiringPi address of the pin which will be written for the 1-Wire bus.
 * @return a {@code OneWireInfo} object representing a 1-Wire bus which is initialized to
 *         communicate over two pins.
 */
OneWireInfoRef OneWireInfoCreateBuffered(GPIOInfoRef gpioInfo, int inputPin, int outputPin);
/**
 * Destroys the resources associated to a 1-Wire bus.
 *
 * @param info a {@code OneWireInfo} object representing the 1-Wire bus to destroy.
 */
void OneWireInfoFree(OneWireInfoRef info);

/**
 * Resets the 1-Wire bus slave devices and gets them ready for a command.
 *
 * @param info a {@code OneWireInfo} object representing the 1-Wire bus.
 * @return {@code TRUE} on success.
 */
BOOL OneWireInfoReset(OneWireInfoRef info);

/**
 * Sends one bit to the 1-Wire slaves.
 *
 * @param info a {@code OneWireInfo} object representing the 1-Wire bus.
 * @param bit if {@code true}, will send '1', otherwise will send '0'.
 */
void OneWireInfoWriteBit(OneWireInfoRef info, BOOL bit);
/**
 * Transmits a byte of data to the 1-Wire slaves.
 *
 * @param info a {@code OneWireInfo} object representing the 1-Wire bus.
 * @param value the data which is sent to the slave devices.
 */
void OneWireInfoWriteByte(OneWireInfoRef info, unsigned char value);

/**
 * Reads one bit from the 1-Wire slaves.
 *
 * @param info a {@code OneWireInfo} object representing the 1-Wire bus.
 * @return {@code TRUE} if '1' was read, otherwise {@code FALSE}.
 */
BOOL OneWireInfoReadBit(OneWireInfoRef info);
/**
 * Reads a complete byte from the slave devices.
 *
 * @param info a {@code OneWireInfo} object representing the 1-Wire bus.
 * @return the data which was read from the slave devices.
 */
unsigned char OneWireInfoReadByte(OneWireInfoRef info);

#endif //GPIO_ONEWIRE_H
