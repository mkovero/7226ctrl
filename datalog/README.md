# Datalog
This document describes datalogging format and method.

## Hardware
arduino/teensy vs pi with $SOME_NICE_DISPLAY (maybe official 7" touchscreen?)

## Method
Going to start with usb option and then proceed using I2C for now.

One option could be just use usb;

https://oscarliang.com/connect-raspberry-pi-and-arduino-usb-cable

pros:
- easy
- works
- allows fw updates from pi

cons: 
- usb cable & connectors are big and bulky in car environment
- need to remove every time doing modifications from laptop

Other option would be do I2C

https://oscarliang.com/raspberry-pi-arduino-connected-i2c/

pros:
- just two wires
- works

cons:
- useless master/slave configuration

Epic option would be to use CANBUS (we're in the car anyway)

https://www.cooking-hacks.com/documentation/tutorials/can-bus-module-shield-tutorial-for-arduino-raspberry-pi-intel-galileo/

pros:
- leetpoints
- works?

cons:
- complexity


## Message format

`vehicleSpeed(kmh);rpmSensor(rpm);tpsSensor(%);gear(1-5);oilTemp(celcius);atfTemp(celcius);load(%);boostSensor(kPa);boostLimit(kPa);lastSPC(int);modVal(int);looptime(ms)`

For example:

`120;4300;30;4;80;90;50;200;250;110;5;2`


