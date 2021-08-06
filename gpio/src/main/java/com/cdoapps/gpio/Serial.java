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

import java.io.InputStream;
import java.io.OutputStream;

/**
 * The {@code Serial} class provides control over a serial bus.
 */
public class Serial {
    static {
        System.loadLibrary("gpio");
    }

    /**
     * A file path representing the first serial communication port.
     */
    public static final String DEVICE_PORT0_PATH = "/dev/ttyS0";

    /**
     * A file path representing the second serial communication port.
     */
    public static final String DEVICE_PORT1_PATH = "/dev/ttyS1";

    /**
     * A file path representing the third serial communication port.
     */
    public static final String DEVICE_PORT2_PATH = "/dev/ttyS2";

    /**
     * A file path representing the fourth serial communication port.
     */
    public static final String DEVICE_PORT3_PATH = "/dev/ttyS3";

    /**
     * A file path representing the fifth serial communication port.
     */
    public static final String DEVICE_PORT4_PATH = "/dev/ttyS4";


    /**
     * 110 bps
     */
    public static final int BAUD_RATE_110 = 110;

    /**
     * 300 bps
     */
    public static final int BAUD_RATE_300 = 300;

    /**
     * 600 bps
     */
    public static final int BAUD_RATE_600 = 600;

    /**
     * 1.2 kbps
     */
    public static final int BAUD_RATE_1200 = 1200;

    /**
     * 2.4 kbps
     */
    public static final int BAUD_RATE_2400 = 2400;

    /**
     * 4.8 kbps
     */
    public static final int BAUD_RATE_4800 = 4800;

    /**
     * 9.6 kbps
     */
    public static final int BAUD_RATE_9600 = 9600;

    /**
     * 14.4 kbps
     */
    public static final int BAUD_RATE_14400 = 14400;

    /**
     * 19.2 kbps
     */
    public static final int BAUD_RATE_19200 = 19200;

    /**
     * 38.4 kbps
     */
    public static final int BAUD_RATE_38400 = 38400;

    /**
     * 57.6 kbps
     */
    public static final int BAUD_RATE_57600 = 57600;

    /**
     * 115.2 kbps
     */
    public static final int BAUD_RATE_115200 = 115200;

    /**
     * The {@code DataBits} enum represents the number of data bits per sent character.
     */
    public enum DataBits {
        /**
         * 5 bits
         */
        Five,

        /**
         * 6 bits
         */
        Six,

        /**
         * 7 bits
         */
        Seven,

        /**
         * 8 bits
         */
        Height
    }

    /**
     * The {@code Parity} enum represents the rule used to check for errors in one sent character.
     */
    public enum Parity {
        /**
         * No bit added.
         */
        None,

        /**
         * One bit added, set to 1 if the character is odd or 0 otherwise.
         */
        Odd,

        /**
         * One bit added, set to 1 if the character is even or 0 otherwise.
         */
        Even
    }

    /**
     * The {@code StopBits} enum represents the number of bits added to detect the end of a
     * character.
     */
    public enum StopBits {
        /**
         * 1 bit
         */
        One,

        /**
         * 2 bits
         */
        Two
    }

    private long mReserved;

    private String path;
    private InputStream inputStream;
    private OutputStream outputStream;

    /**
     * Create a new serial bus that will communicate over the given port.
     *
     * @param path the file path of the serial port which will initialize the serial communications.
     */
    public Serial(String path) {
        this.path = path;
    }

    /**
     * Initializes the serial communications.
     *
     * @param baudRate the data rate/speed in bps (bits per second).
     * @param dataBits the number of bits per character.
     * @param parity the rule of the check bit added to one character if any.
     * @param stopBits the number of stop bits added to one character.
     */
    public native void configure(int baudRate, DataBits dataBits, Parity parity, StopBits stopBits);
    /**
     * Terminates the communications with this serial bus and free the resources which were
     * associated to it.
     */
    public native void destroy();

    /**
     * Returns the {@code InputStream} associated to the serial bus after it has been configured.
     *
     * Used to receive bytes from the connected device.
     *
     * @return the {@code InputStream} associated to the serial bus.
     */
    public InputStream getInputStream() {
        return inputStream;
    }
    /**
     * Returns the {@code OutputStream} associated to the serial bus after it has been configured.
     *
     * Used to send bytes to the connected device.
     *
     * @return the {@code OutputStream} associated to the serial bus.
     */
    public OutputStream getOutputStream() {
        return outputStream;
    }
}
