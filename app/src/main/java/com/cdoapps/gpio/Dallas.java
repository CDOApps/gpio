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

import java.util.List;

/**
 * The {@code Dallas} class provides communication over a 1-Wire bus to control Dallas digital
 * thermometers DS18S20 and DS18B20.
 *
 * Datasheets:
 * <ul><li><a href="https://datasheets.maximintegrated.com/en/ds/DS18S20.pdf">DS18S20</a></li>
 * <li><a href="https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf">DS18B20</a></li></ul>
 */
public class Dallas {
    static {
        System.loadLibrary("gpio");
    }

    /**
     * The {@code SensorFamily} enum represents the device family of a thermometer.
     */
    public enum SensorFamily {
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
    private Dallas() {
    }

    /**
     * Returns all the thermometers connected to a 1-Wire bus.
     *
     * @param bus a {@code OneWire} object representing a bus configured on one GPIO pin.
     * @return a @{@code List} containing all thermometers of DS18S20 family or DS18B20 family.
     */
    public static native List<Dallas> listSensors(OneWire bus);
    /**
     * Free all the resources associated to many {@code Dallas} objects.
     *
     * @param sensors a {@code List} containing the thermometers to be destroyed.
     */
    public static void destroyAll(List<Dallas> sensors) {
        for (Dallas sensor : sensors)
            sensor.destroy();
    }
    /**
     * Free all the resources associated to a {@code Dallas} object.
     */
    public native void destroy();

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
    public native SensorFamily getFamily();
    /**
     * Returns {@code true} if this thermometer operates in parasitic power mode.
     *
     * @return {@code true} if this thermometer operates in parasitic power mode.
     */
    public native boolean usesParasiticPowerMode();

    /**
     * Issues a temperature conversion on all the thermometers connected to a 1-Wire bus.
     *
     * This version of {@code convert} checks if all the thermometers contained in {@code sensors}
     * are powered by an external supply.
     *
     * @param bus a {@code OneWire} object representing a bus configured on one GPIO pin.
     * @param sensors a {@code List} containing the thermometers to be destroyed.
     */
    public static void convert(OneWire bus, List<Dallas> sensors) {
        boolean parasiticPowerMode = false;
        for (Dallas sensor : sensors) {
            if (sensor.usesParasiticPowerMode()) {
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
