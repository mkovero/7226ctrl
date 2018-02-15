#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// UI STAGE
// Control for what user sees and how gearbox is used with
// 

// Display update
void updateDisplay() {
  int atfTemp = atfRead();
  int tpsPercentValue = tpsRead();
/*  display.clearDisplay();
  display.setCursor(3,0);
  display.setTextSize(5);
  if ( ! fullAuto ) {
    if ( prevgear <= 5 ) { display.print(prevgear); };
    if ( prevgear == 6 ) { display.print("N"); };
    if ( prevgear == 7 ) { display.print("R"); };
    if ( prevgear == 8 ) { display.print("P"); };
    display.print("->");
    if ( gear <= 5 ) { display.print(gear); };
    if ( gear == 6 ) { display.print("N"); };
    if ( gear == 7 ) { display.print("R"); };
    if ( gear == 8 ) { display.print("P"); }; 
  } else {
    if ( gear == 5 ) { display.print("D"); };
    if ( gear == 6 ) { display.print("N"); };
    if ( gear == 7 ) { display.print("R"); };
    if ( gear == 8 ) { display.print("P"); }; 
  }
  display.setTextSize(1);
  display.println(atfTemp);
  display.print(", ");
  display.print(vehicleSpeed);
  display.print(", ");
  display.print(tpsPercentValue);
  display.display();*/
}
