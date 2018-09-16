# 7226ctrl
"controller for 722.6 automatic transmission"

In a nutshell this was supposed to be reverse engineered transmission controller for 722.6 gearboxes found in many mercedes and chrysler vehicles, design is similar with more recent (7gtronic++) gear boxes, so in theory this could be applied with gentle care to work with those as well.

As with most things of life, this got bit out of hand and currently also controls boost, speedometer, rpm meter, fuelpumps and even horn, its also able to communicate with electrical steering wheel for menus and manual button shifts.
Canbus interface is also supported on pcb but I have not yet utilized in any meaningful fashion.

First revision of working pcb is done:

<http://www.mui.fi/pcb-valmis.jpg> 

and gerber files are available under pcb/ with bom.

Assembly instructions are not included, but I would be happy if someone would wish to write those.

- Not yet fully functional
- Use at your own risk!

Currently should work:
- Automatic downshift / upshift based on vehicle speed and throttle position (tested and works)
- Manual shifting (tested and works)
- Throttle position sensor (tested and works)
- Pressure maps for shifts and modulation pressure based on oil temperature and load, even when P/N/R (tested and works)
- W202 gearstick can be read (tested and works)
- SPI OLED for data output (tested and works)
- Boost limit maps (tested and works)
- Boost limiting (eg. drop boost on shifts) (tested and works)
- w124 wire-speedometer (tested and works)
- Fuel pump control on idle (tested and works)
- vehicle speed reading based on tire size and diff ratio with and without engine rpm (tested and works)
- rpm reading (tested and works)
- Electric steering wheel support (manual shifts, menu control)
- oil temperature (tested and works)
- n2/n3 speed sensors (tested and works)
- evaluateGear, make sure which gear is in (tested and works)

Implemented but needs real life testing:
- adaptative shift pressure (works in lab)
- External datalogging (format, method, works in lab)
- w124 rpm meter (works in theory)

Work in progress:
- External datalogging (hardware implementation, visualization)

Not started/pending:
- PID controlled PWM for temperature stability
- percentual map reads preferably without floats (eg. value between point x and y).



work by Markus Kovero (mui@mui.fi) with regards to Tuomas Kantola.

Schematics are for teensy, can be utilized with mega as well to some extent.
I'm in progress of designing PCB for this to make things easier, currently its just too much wire to handle in reasonable manner.

![Alt text](/schematics.png?raw=true "Schematics for teensy")

