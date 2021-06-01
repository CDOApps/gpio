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

#ifndef GPIO_GPIO_H
#define GPIO_GPIO_H

/**
 * The {@code GPIOInfo} struct provides control over Odroid-N2 gpio pins.
 *
 * Wiki:
 * <a href="https://wiki.odroid.com/odroid-n2/odroid-n2">Hardkernel Odroid-N2</a>
 */
typedef struct GPIOInfo *GPIOInfoRef;

/**
 * Returns a {@code GPIOInfo} object which represents a GPIO controller for the Odroid-N2.
 *
 * @return a {@code GPIOInfo} object which represents a GPIO controller for the Odroid-N2.
 */
GPIOInfoRef GPIOInfoAlloc(void);
/**
 * Destroys the resources associated to a GPIO controller.
 *
 * @param info a {@code GPIOInfo} object representing the GPIO controller to destroy.
 */
void GPIOInfoFree(GPIOInfoRef info);

/**
 * Creates the interface with one pin.
 *
 * @param info a {@code GPIOInfo} object representing the GPIO controller.
 * @param pin the WiringPi address of the pin to export.
 */
void GPIOInfoExport(GPIOInfoRef info, int pin);
/**
 * Destroys the interface previously bound to one pin.
 *
 * @param info a {@code GPIOInfo} object representing the GPIO controller.
 * @param pin the WiringPi address of the pin to unexport.
 */
void GPIOInfoUnexport(GPIOInfoRef info, int pin);
/**
 * Destroys all the interfaces previously bound to one pin.
 *
 * @param info a {@code GPIOInfo} object representing the GPIO controller.
 */
void GPIOInfoUnexportAll(GPIOInfoRef info);

/**
 * "in"
 * One may call {@code GPIOInfoSetMode} with this value to use one pin as an input.
 */
extern const char *GPIO_PIN_MODE_INPUT;
/**
 * "out"
 * One may call {@code GPIOInfoSetMode} with this value to use one pin as an output.
 */
extern const char *GPIO_PIN_MODE_OUTPUT;

/**
 * Changes the communication mode of one pin.
 *
 * @param info a {@code GPIOInfo} object representing the GPIO controller.
 * @param pin the WiringPi address of the pin.
 * @param mode should be either {@code GPIO_PIN_MODE_INPUT} or {@code GPIO_PIN_MODE_OUTPUT}.
 */
void GPIOInfoSetMode(GPIOInfoRef info, int pin, const char *mode);

/**
 * The value returned for a pin connected to GND.
 */
#define GPIO_PIN_VALUE_LOW 0
/**
 * The value returned for a pin connected to VDD.
 */
#define GPIO_PIN_VALUE_HIGH 1

/**
 * Changes the value of one pin.
 *
 * @param info a {@code GPIOInfo} object representing the GPIO controller.
 * @param pin the WiringPi address of the pin.
 * @param value should be either {@code GPIO_PIN_VALUE_LOW} or {@code GPIO_PIN_VALUE_HIGH}.
 */
void GPIOInfoSetValue(GPIOInfoRef info, int pin, int value);
/**
 * Returns the value of one pin.
 *
 * @param info a {@code GPIOInfo} object representing the GPIO controller.
 * @param pin the WiringPi address of the pin.
 * @return the value of {@code pin}
 */
int GPIOInfoGetValue(GPIOInfoRef info, int pin);

#endif //GPIO_GPIO_H
