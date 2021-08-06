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
 * The {@code GPIO} class provides control over Odroid-N2 gpio pins.
 *
 * Wiki:
 * <a href="https://wiki.odroid.com/odroid-n2/odroid-n2">Hardkernel Odroid-N2</a>
 */
public class GPIO {
    static {
        System.loadLibrary("gpio");
    }

    private static GPIO instance = new GPIO();
    /**
     * Returns a singleton representing all the pins that can be used as an input or output.
     *
     * @return a singleton representing all the pins that can be used as an input or output.
     */
    public static GPIO getInstance() {
        return instance;
    }

    private long mReserved;
    private GPIO() {
    }

    /**
     * Initializes the {@code GPIO} instance.
     */
    public native void onResume();
    /**
     * Terminates the {@code GPIO} instance and free the resources which were associated to it.
     */
    public native void onPause();

    /**
     * Creates the interface with one pin.
     *
     * @param pin the WiringPi address of the pin to export.
     */
    public native void export(int pin);
    /**
     * Destroys the interface previously bound to one pin.
     *
     * @param pin the WiringPi address of the pin to unexport.
     */
    public native void unexport(int pin);
    /**
     * Destroys all the interfaces previously bound to one pin.
     */
    public native void unexportAll();

    /**
     * One may call {@code setMode} with this value to use one pin as an input.
     */
    public static final String MODE_INPUT = "in";
    /**
     * One may call {@code setMode} with this value to use one pin as an output.
     */
    public static final String MODE_OUTPUT = "out";

    /**
     * Changes the communication mode of one pin.
     *
     * @param pin the WiringPi address of the pin.
     * @param mode should be either {@code MODE_INPUT} or {@code MODE_OUTPUT}.
     */
    public native void setMode(int pin, String mode);

    /**
     * The value returned for a pin connected to GND.
     */
    public static final int VALUE_LOW = 0;
    /**
     * The value returned for a pin connected to VDD.
     */
    public static final int VALUE_HIGH = 1;

    /**
     * Changes the value of one pin.
     *
     * @param pin the WiringPi address of the pin.
     * @param value should be either {@code VALUE_LOW} or {@code VALUE_HIGH}.
     */
    public native void setValue(int pin, int value);
    /**
     * Returns the value of one pin.
     *
     * @param pin the WiringPi address of the pin.
     * @return the value of {@code pin}
     */
    public native int getValue(int pin);
}
