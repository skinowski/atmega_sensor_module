# atmega_sensor_module

A simple AtMega code to expose encoder readings via I2C protocol. The encoders
used in the project are two encoders (YUMO E6B2-CWZ3E Rotary Encoder Quadrature)
attached to robot wheels.

I2C communication allows reset of encoder readings as well as ISR counter.

This is part of a robot hobby project in which AtMega is used as a companion
microcontroller to Raspberry Pi 2 for real time tasks or reading analog inputs.

## References / Credits

* Ported from original TWI_Slave code from Atmel Corporation.
* [AVR311: Using the TWI module as I2C slave](http://www.atmel.com/images/doc2565.pdf)
* [AvrFreaks Forum](http://www.avrfreaks.net/forum/c-code-hardware-i2c-slave-atmega81632)
* See Makefile for Makefile credits

## TODO

* Add voltage reading to analog inputs and expose this in I2C as well. This
is needed as raspberry pi 2 cannot read analog inputs.
[See Measuring DC Voltage using Arduino](https://startingelectronics.org/articles/arduino/measuring-voltage-with-arduino/)
