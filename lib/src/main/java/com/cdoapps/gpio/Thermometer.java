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

import java.util.ArrayList;
import java.util.List;

/**
 * The {@code Thermometer} class provides communication over a 1-Wire bus to control Dallas digital
 * thermometers DS18S20 and DS18B20.
 *
 * Datasheets:
 * <ul><li><a href="https://datasheets.maximintegrated.com/en/ds/DS18S20.pdf">DS18S20</a></li>
 * <li><a href="https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf">DS18B20</a></li></ul>
 */
public class Thermometer {
    static {
        System.loadLibrary("gpio");
    }

    /**
     * The {@code Family} enum represents the Dallas family of a thermometer.
     */
    public enum Family {
        /**
         * DS18S20
         */
        DS18S20,

        /**
         * DS18B20
         */
        DS18B20
    }

    private long mReserved;
    private Thermometer() {
    }

    /**
     * Returns all the thermometers connected to a 1-Wire bus.
     *
     * @param bus a {@code OneWire} object representing a bus configured on one GPIO pin.
     * @return a @{@code List} containing all thermometers of DS18S20 family or DS18B20 family.
     */
    public static native List<Thermometer> listAll(OneWire bus);
    /**
     * Free all the resources associated to many {@code Thermometer} objects.
     *
     * @param thermometers a {@code List} containing the thermometers to be destroyed.
     */
    public static void destroyAll(List<Thermometer> thermometers) {
        for (Thermometer thermometer : thermometers)
            thermometer.destroy();
    }
    /**
     * Free all the resources associated to a {@code Thermometer} object.
     */
    public native void destroy();

    /**
     * Returns a {@code byte[]} representation of many {@code Thermometer} objects.
     *
     * This representation may be saved and loaded later without the need to call listAll().
     *
     * @return a {@code byte[]} representation of the thermometers to be serialized.
     */
    public static byte[] serializeAll(List<Thermometer> thermometers) {
        int length = 0;
        List<byte[]> allData = new ArrayList<>();
        for (Thermometer thermometer : thermometers) {
            byte[] data = thermometer.serialize();
            allData.add(data);
            length += data.length;
        }

        byte[] result = new byte[length];
        length = 0;
        for (byte[] data : allData) {
            System.arraycopy(data, 0, result, length, data.length);
            length += data.length;
        }

        return result;
    }
    /**
     * Returns a {@code byte[]} representation of this thermometer.
     *
     * @return a {@code byte[]} representation of this thermometer.
     */
    public native byte[] serialize();
    /**
     * Returns all the thermometers stored into a {@code byte[]} representation.
     *
     * @param bus a {@code OneWire} object representing a bus configured on one GPIO pin.
     * @param data a {@code byte[]} object representing thermometers that were retrieved from `bus`
     *             earlier.
     * @return a @{@code List} containing all thermometers of DS18S20 family or DS18B20 family
     * stored into `data`.
     */
    public static native List<Thermometer> deserializeAll(OneWire bus, byte[] data);
    /**
     * Returns a thermometer stored into a {@code byte[]} representation.
     *
     * @param bus a {@code OneWire} object representing a bus configured on one GPIO pin.
     * @param data a {@code byte[]} object representing a thermometer that was retrieved from `bus`
     *             earlier.
     * @return a @{@code Thermometer} object stored into `data`.
     */
    public static native Thermometer deserialize(OneWire bus, byte[] data);

    /**
     * Returns a {@code String} which identifies this thermometer.
     *
     * @return a {@code String} which identifies this thermometer.
     */
    public native String getRom();
    /**
     * Returns the device family of this thermometer.
     *
     * @return the device family of this thermometer.
     */
    public native Family getFamily();
    /**
     * Returns {@code true} if this thermometer operates in parasitic power mode.
     *
     * @return {@code true} if this thermometer operates in parasitic power mode.
     */
    public native boolean usesParasiticPowerMode();

    /**
     * Issues a temperature conversion on all the thermometers connected to a 1-Wire bus.
     *
     * This version of {@code convert} checks if all the thermometers contained in
     * {@code thermometers} are powered by an external supply.
     *
     * @param bus a {@code OneWire} object representing a bus configured on one GPIO pin.
     * @param thermometers a {@code List} containing the thermometers on the `bus`.
     */
    public static void convert(OneWire bus, List<Thermometer> thermometers) {
        boolean parasiticPowerMode = false;
        for (Thermometer thermometer : thermometers) {
            if (thermometer.usesParasiticPowerMode()) {
                parasiticPowerMode = true;
                break;
            }
        }

        convert(bus, parasiticPowerMode);
    }
    /**
     * Issues a temperature conversion on all the thermometers connected to a 1-Wire bus.
     *
     * This function blocks until the conversion is done. In parasitic power mode, the thermometers
     * will not notify if the conversion is done or not, thus it will blocks for 1s which is
     * arbitrary and should be enough (the Maxim Integrated datasheets indicate a maximum conversion
     * time of 750ms).
     *
     * @param bus a {@code OneWire} object representing a bus configured on one GPIO pin.
     * @param parasiticPowerMode if {@code true}, assume the thermometers on the 1-Wire bus are
     *                           powered using parasitic power mode.
     */
    public static native void convert(OneWire bus, boolean parasiticPowerMode);

    /**
     * Returns the temperature measured by this thermometer.
     *
     * One may call this function after issuing a conversion by calling {@code convert}.
     *
     * @return the temperature measured by this thermometer.
     */
    public native float getTemperature();
}
