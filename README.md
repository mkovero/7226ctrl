# 7226ctrl
controller for 722.6 automatic transmission

- Not yet fully functional
- Use at your own risk!

work by Markus Kovero (mui@mui.fi) with regards to Tuomas Kantola.

Pinout is for mega2560 with SPI OLED display.

Display | Arduino Pin
------------ | -------------
D0|12
D1|11
CS|8
DC|9
RES|10

Transmission|Arduino pin (through mosfet shield)
------------ | -------------
y3|4
y4|5
y5|6
mpc|2
spc|7
tcc|3

Gearstick|Arduino pin (through voltage regulators)
------------ | -------------
white|27
blue|29
green|33
yellow|35
tccSwitch|25


Sensors|Arduino pin (not there yet)
------------ | -------------
RPM|n/a
TPS|n/a
MAP|n/a
N2|19
N3|20
