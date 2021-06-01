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

#ifndef GPIO_DALLAS_H
#define GPIO_DALLAS_H

#include "onewire.h"
#include "stack.h"

/**
 * The {@code DallasSensorFamily} enum represents the device family of a thermometer.
 */
typedef enum {
    /**
     * This exists to represents a {@code null} object in Java.
     */
    DallasSensorFamilyUnknown = -1,

    /**
     * DS18S20
     */
    DallasSensorFamilyDS18S20,

    /**
     * DS18B20
     */
    DallasSensorFamilyDS18B20
} DallasSensorFamily;

/**
 * The {@code DallasInfo} struct provides communication over a 1-Wire bus to control Dallas digital
 * thermometers DS18S20 and DS18B20.
 *
 * Datasheets:
 * <ul><li><a href="https://datasheets.maximintegrated.com/en/ds/DS18S20.pdf">DS18S20</a></li>
 * <li><a href="https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf">DS18B20</a></li></ul>
 */
typedef struct DallasInfo *DallasInfoRef;

/**
 * Returns all the thermometers connected to a 1-Wire bus.
 *
 * @param oneWireInfo a {@code OneWireInfo} object representing a bus configured on one GPIO pin.
 * @param stack a @{@code Stack} containing all thermometers of DS18S20 family or DS18B20 family on
 *              return.
 */
void DallasInfoList(OneWireInfoRef oneWireInfo, StackRef stack);

/**
 * Returns a {@code DallasInfo} object which represents a thermometer identified by a rom.
 *
 * This function may returns {@code NULL} if {@code rom} does not contain a valid family info.
 *
 * @param oneWireInfo a {@code OneWireInfo} object representing a bus configured on one GPIO pin.
 * @param rom a characters string identifying the thermometer.
 * @param parasiticPowerMode if {@code TRUE}, the thermometer operates in parasitic power mode.
 */
DallasInfoRef DallasInfoCreate(OneWireInfoRef oneWireInfo, const char *rom,
                               BOOL parasiticPowerMode);
/**
 * Destroys the resources associated to a thermometer.
 *
 * @param info a {@code DallasInfo} object representing the thermometer to destroy.
 */
void DallasInfoFree(DallasInfoRef info);

/**
 * Returns a characters string which identifies this thermometer.
 *
 * @param info a {@code DallasInfo} object representing the thermometer.
 * @return a characters string which identifies this thermometer.
 */
const char *DallasInfoGetRom(DallasInfoRef info);
/**
 * Returns the device family of this thermometer.
 *
 * @param info a {@code DallasInfo} object representing the thermometer.
 * @return the device family of this thermometer.
 */
DallasSensorFamily DallasInfoGetFamily(DallasInfoRef info);
/**
 * Returns {@code true} if this thermometer operates in parasitic power mode.
 *
 * @param info a {@code DallasInfo} object representing the thermometer.
 * @return {@code true} if this thermometer operates in parasitic power mode.
 */
BOOL DallasInfoUsesParasiticPowerMode(DallasInfoRef info);

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
void DallasInfoConvert(OneWireInfoRef oneWireInfo, BOOL parasiticPowerMode);

/**
 * Returns the temperature measured by this thermometer.
 *
 * One may call this function after issuing a conversion by calling {@code DallasInfoConvert}.
 *
 * @param info a {@code DallasInfo} object representing the thermometer.
 * @return the temperature measured by this thermometer.
 */
float DallasInfoGetTemperature(DallasInfoRef info);

#endif //GPIO_DALLAS_H
