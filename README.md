# 7226ctrl
controller for 722.6 automatic transmission

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

Implemented but needs real life testing:
- adaptative shift pressure (works in lab)
- External datalogging (format, method, works in lab)
- Fuel pump control on idle (initial pressure on startup, enable pumps after certain RPM, works in lab)
- evaluateGear, make sure which gear is in (works in lab)
- n2/n3 speed sensors (works in lab)
- vehicle speed reading based on tire size and diff ratio with and without engine rpm (works in lab)
- rpm reading (works in lab)
- w124 rpm meter (works in theory)

Work in progress:
- External datalogging (hardware implementation, visualization)

Not started/pending:
- oil temperature (pending sensor installation)
- PID controlled PWM for temperature stability
- percentual map reads preferably without floats (eg. value between point x and y).

Considered:
- Electric steering wheel support

work by Markus Kovero (mui@mui.fi) with regards to Tuomas Kantola.

Pinout is for mega2560 with SPI OLED display and is subject to change.
IFR540 in schematics is actually presenting 4channel mosfet board including protection circuit and logic level changer. (5usd, can be found from ebay).

![Alt text](/schematics.png?raw=true "Title")

