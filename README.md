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
// In Activity.onPause

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

Search for thermometers over a 1-Wire bus:
```java
// OneWire oneWire
List<Thermometer> thermometers = Thermometer.listAll(oneWire);
```

Read temperatures measured by the connected thermometers:
```java
Thermometer.convert(oneWire, thermometers);

for (Thermometer thermometer : thermometers)
  Log.d(TAG, "Temperature of #" + thermometer.getRom() + " is " + thermometer.getTemperature());
```

Once done with the thermometers, terminate:
```java
Thermometer.destroyAll(thermometers);
```

## Serial

Initialize the Serial bus by calling configure:
```java
// In Activity.onResume

Serial serial = new Serial(Serial.DEVICE_PORT1_PATH);
serial.configure(9600, Serial.DataBits.Height, Serial.Parity.None, Serial.StopBits.One);
```

Send a message via the configured bus:
```java
serial.getOutputStream().write("Hello world".getBytes());
```

Receive a message from the configured bus:
```java
byte[] buffer = new byte[256];
int size = serial.getInputStream().read(buffer);
String message = new String(buffer, 0, size);
```

Terminate the Serial bus:
```java
// In Activity.onPause
serial.destroy();
```

# Roadmap

- **GPIO**: handle signals on falling/rising edge of a GPIO pin.
- **UART**: add an implementation of serial communication using bit banging on any GPIO rx/tx pins.

# Wiki

Get more info on [Hardkernel Odroid-N2 wiki](https://wiki.odroid.com/odroid-n2/odroid-n2).
