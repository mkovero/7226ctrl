# 7226ctrl
controller for 722.6 automatic transmission

- Not yet fully functional
- Use at your own risk!

Currently should work:
- Automatic downshift / upshift based on vehicle speed and throttle position
- Manual shifting
- Throttle position sensor
- Pressure maps for shifts and modulation pressure based on oil temperature and load, even when P/N/R
- W202 gearstick can be read
- SPI OLED for data output
- Boost limit maps

Work in progress:
- Boost limiting (eg. drop boost on shifts)
- evaluateGear, make sure which gear is in (pending hallsensor testing)

Not started/pending:
- vehicle speed reading (pending powertrain to be installed, going to read abs hallsensor from diff)
- rpm reading (pending sensor installation, going to read original om606 rpm hallsensor)
- n2/n3 speed sensors (pending moving car)
- oil temperature (pending sensor installation)
- PID controlled PWM for temperature stability
- percentual map reads preferably without floats (eg. value between point x and y).

Considered:
- Fuel pump control on idle
- Electric steering sheel support

work by Markus Kovero (mui@mui.fi) with regards to Tuomas Kantola.

Pinout is for mega2560 with SPI OLED display and is subject to change.
IFR540 in schematics is actually presenting 4channel mosfet board including protection circuit and logic level changer. (5usd, can be foud from ebay).

![Alt text](/schematics.png?raw=true "Title")

