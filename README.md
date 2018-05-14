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
- Boost limiting (eg. drop boost on shifts)
- adaptative shift pressure
- External datalogging (format, method)
- Fuel pump control on idle (initial pressure on startup, enable pumps after certain RPM)

Work in progress:
- evaluateGear, make sure which gear is in (pending hallsensor testing)
- External datalogging (hardware implementation, visualization)
- vehicle speed reading (static calculation should work)
- rpm reading (pending sensor installation, started)

Not started/pending:
- n2/n3 speed sensors (pending moving car)
- oil temperature (pending sensor installation)
- PID controlled PWM for temperature stability
- percentual map reads preferably without floats (eg. value between point x and y).

Considered:
- Electric steering wheel support

work by Markus Kovero (mui@mui.fi) with regards to Tuomas Kantola.

Pinout is for mega2560 with SPI OLED display and is subject to change.
IFR540 in schematics is actually presenting 4channel mosfet board including protection circuit and logic level changer. (5usd, can be found from ebay).

![Alt text](/schematics.png?raw=true "Title")

