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

#ifndef GPIO_THERMOMETER_H
#define GPIO_THERMOMETER_H

#include "onewire.h"
#include "stack.h"

/**
 * The {@code ThermometerFamily} enum represents the Dallas family of a thermometer.
 */
typedef enum {
    /**
     * This exists to represents a {@code null} object in Java.
     */
    ThermometerFamilyUnknown = -1,

    /**
     * DS18S20
     */
    ThermometerFamilyDS18S20,

    /**
     * DS18B20
     */
    ThermometerFamilyDS18B20
} ThermometerFamily;

/**
 * The {@code ThermometerInfo} struct provides communication over a 1-Wire bus to control Dallas
 * digital thermometers DS18S20 and DS18B20.
 *
 * Datasheets:
 * <ul><li><a href="https://datasheets.maximintegrated.com/en/ds/DS18S20.pdf">DS18S20</a></li>
 * <li><a href="https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf">DS18B20</a></li></ul>
 */
typedef struct ThermometerInfo *ThermometerInfoRef;

/**
 * Returns all the thermometers connected to a 1-Wire bus.
 *
 * @param oneWireInfo a {@code OneWireInfo} object representing a bus configured on one GPIO pin.
 * @param stack a @{@code Stack} containing all thermometers of DS18S20 family or DS18B20 family on
 *              return.
 */
void ThermometerInfoList(OneWireInfoRef oneWireInfo, StackRef stack);

/**
 * Returns a {@code ThermometerInfo} object which represents a thermometer identified by a rom.
 *
 * This function may returns {@code NULL} if {@code rom} does not contain a valid family info.
 *
 * @param oneWireInfo a {@code OneWireInfo} object representing a bus configured on one GPIO pin.
 * @param rom a characters string identifying the thermometer.
 * @param parasiticPowerMode if {@code TRUE}, the thermometer operates in parasitic power mode.
 */
ThermometerInfoRef ThermometerInfoCreate(OneWireInfoRef oneWireInfo, const char *rom,
                               BOOL parasiticPowerMode);
/**
 * Destroys the resources associated to a thermometer.
 *
 * @param info a {@code ThermometerInfo} object representing the thermometer to destroy.
 */
void ThermometerInfoFree(ThermometerInfoRef info);

/**
 * Returns a characters string which identifies this thermometer.
 *
 * @param info a {@code ThermometerInfo} object representing the thermometer.
 * @return a characters string which identifies this thermometer.
 */
const char *ThermometerInfoGetRom(ThermometerInfoRef info);
/**
 * Returns the device family of this thermometer.
 *
 * @param info a {@code ThermometerInfo} object representing the thermometer.
 * @return the device family of this thermometer.
 */
ThermometerFamily ThermometerInfoGetFamily(ThermometerInfoRef info);
/**
 * Returns {@code true} if this thermometer operates in parasitic power mode.
 *
 * @param info a {@code ThermometerInfo} object representing the thermometer.
 * @return {@code true} if this thermometer operates in parasitic power mode.
 */
BOOL ThermometerInfoUsesParasiticPowerMode(ThermometerInfoRef info);

/**
 * Issues a temperature conversion on all the thermometers connected to a 1-Wire bus.
 *
 * This function blocks until the conversion is done. In parasitic power mode, the thermometers
 * will not notify if the conversion is done or not, thus it will blocks for 1s which is
 * arbitrary and should be enough (the Maxim Integrated datasheets indicate a maximum conversion
 * time of 750ms).
 *
 * @param oneWireInfo a {@code OneWireInfo} object representing a bus configured on one GPIO pin.
 * @param parasiticPowerMode if {@code true}, assume the thermometers on the 1-Wire bus are
 *                           powered using parasitic power mode.
 */
void ThermometerInfoConvertAll(OneWireInfoRef oneWireInfo, BOOL parasiticPowerMode);

/**
 * Issues a temperature conversion on this thermometer.
 *
 * This function blocks for 1s which is arbitrary and should be enough for the conversion to be done
 * (the Maxim Integrated datasheets indicate a maximum conversion time of 750ms).
 *
 * @param info a {@code ThermometerInfo} object representing the thermometer.
 */
void ThermometerInfoConvert(ThermometerInfoRef info);

/**
 * Returns the temperature measured by this thermometer.
 *
 * One may call this function after issuing a conversion by calling {@code ThermometerInfoConvertAll}.
 *
 * @param info a {@code ThermometerInfo} object representing the thermometer.
 * @return the temperature measured by this thermometer.
 */
float ThermometerInfoGetTemperature(ThermometerInfoRef info);

#endif //GPIO_THERMOMETER_H
