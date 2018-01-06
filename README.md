# 7226ctrl
controller for 722.6 automatic transmission

- Not yet fully functional
- Use at your own risk!

Pinout is for mega2560 with SPI OLED display.

Display	Arduino Pin
D0	12
D1	11
CS	8
DC	9
RES	10

Transmission  Arduino pin (through mosfet shield)
y3	2
y4	3
y5	4
mpc	5
spc	6
tcc	7

Gearstick	Arduino pin (through voltage regulators)
white	27
blue	29
green	33
yellow  35
tccSwitch	25


Sensors	Arduino pin (not there yet)
RPM	n/a
TPS	n/a
MAP	n/a

