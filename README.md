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
y3|47
y4|45
y5|46
mpc|44
spc|43
tcc|42

Gearstick|Arduino pin (through voltage regulators)
------------ | -------------
white|27
blue|29
green|26
yellow|28
tccSwitch|25


Sensors|Arduino pin (not there yet)
------------ | -------------
RPM|n/a
TPS|n/a
MAP|n/a
N2|19
N3|20
