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

package com.cdoapps.gpio;

/**
 * The {@code OneWire} class represents a 1-Wire bus communicating over one pin of a {@code GPIO}
 * instance.
 *
 * This implementation of the 1-Wire bus uses bit banging and spinning, thus it may produce high
 * CPU load.
 *
 * Specification:
 * <a href="https://ww1.microchip.com/downloads/en/appnotes/01199a.pdf">1-Wire Protocol</a>
 *
 */
public class OneWire {
    static {
        System.loadLibrary("gpio");
    }

    private long mReserved;

    /**
     * Initializes the 1-Wire communications over one pin.
     *
     * @param gpio a {@code GPIO} instance used to initialize and communicate with the 1-Wire bus.
     * @param pin the WiringPi address of the pin which will be exported for the 1-Wire bus.
     */
    public native void configure(GPIO gpio, int pin);
    /**
     * Initializes the 1-Wire communications over two pins.
     *
     * @param gpio a {@code GPIO} instance used to initialize and communicate with the 1-Wire bus.
     * @param inputPin the WiringPi address of the pin which will be read for the 1-Wire bus.
     * @param outputPin the WiringPi address of the pin which will be written for the 1-Wire bus.
     */
    public native void configureBuffered(GPIO gpio, int inputPin, int outputPin);
    /**
     * Terminates the communications with this 1-Wire bus and free the resources which were
     * associated to it.
     */
    public native void destroy();

    /**
     * Resets the 1-Wire bus slave devices and gets them ready for a command.
     *
     * @return {@code true} on success.
     */
    public native boolean reset();

    /**
     * Sends one bit to the 1-Wire slaves.
     *
     * @param bit if {@code true}, will send '1', otherwise will send '0'.
     */
    public native void writeBit(boolean bit);
    /**
     * Transmits a byte of data to the 1-Wire slaves.
     *
     * @param value the data which is sent to the slave devices.
     */
    public native void writeByte(byte value);

    /**
     * Reads one bit from the 1-Wire slaves.
     *
     * @return {@code true} if '1' was read, otherwise {@code false}.
     */
    public native boolean readBit();
    /**
     * Reads a complete byte from the slave devices.
     *
     * @return the data which was read from the slave devices.
     */
    public native byte readByte();
}
