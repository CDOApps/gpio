# Summary

This project is a GPIO controller for the Odroid-N2. It also features an implementation of the Dallas 1-Wire bus.

It can be used as a native library for Android. There's a sample project which displays the temperatures measured by digital thermometers to demonstrate how to use the controller.

The goal of this project was to build an easy-to-use interface for Odroid-N2 GPIO in an Android app (Java/Kotlin).

# Usage

## GPIO

Initialize the GPIO by calling onResume:
```java
// In Activity.onResume

GPIO.getInstance().onResume();
```

Configure one pin (use its WiringPi number):
```java
GPIO gpio = GPIO.getInstance();
gpio.export(0);

// or gpio.setMode(0, GPIO.MODE_OUTPUT);
gpio.setMode(0, GPIO.MODE_INPUT);
```

Read its value:
```java
gpio.getValue(0);
```

Write a new value (if configured as output):
```java
// or gpio.setValue(0, GPIO.VALUE_HIGH);
gpio.setValue(0, GPIO.VALUE_LOW);
```

Terminate the GPIO by calling onPause:
```java
// In Activity.onResume

GPIO gpio = GPIO.getInstance();
gpio.unexportAll();
gpio.onPause();
```

## 1-Wire

Initialize a 1-Wire bus on any pin:
```java
OneWire oneWire = new OneWire();
oneWire.configure(GPIO.getInstance(), 0);
```

Once done with the bus, terminate:
```java
oneWire.destroy();
```

## DS18S20/DS18B20 thermometers

Search for sensors over a 1-Wire bus:
```java
// OneWire oneWire
List<Dallas> sensors = Dallas.listSensors(oneWire);
```

Read temperatures measured by the connected sensors:
```java
Dallas.convert(oneWire, sensors);

for (Dallas sensor : sensors)
  Log.d(TAG, "Temperature of #" + sensor.getRom() + " is " + sensor.getTemperature());
```

Once done with the sensors, terminate:
```java
Dallas.destroyAll(sensors);
```

# Roadmap

- **UART**: add an implementation of serial communication using any GPIO rx/tx pins.

# Wiki

Get more info on [Hardkernel Odroid-N2 wiki](https://wiki.odroid.com/odroid-n2/odroid-n2).
