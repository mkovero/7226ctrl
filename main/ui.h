// UI STAGE
// Control for what user sees and how gearbox is used with
// 
void draw(void) {
  int atfTemp = atfRead();
  int tpsPercentValue = tpsRead();
  int oilTemp = oilRead();
  int boostPressure = boostRead();
  int rpmValue = rpmRead();
  // graphic commands to redraw the complete screen should be placed here  
  u8g.setFont(u8g_font_ncenB18);
  u8g.setPrintPos(50,20);
  u8g.print(prevgear);
  u8g.print(">");
  u8g.print(gear);
  u8g.setFont(u8g_font_fub14);
  u8g.setPrintPos(60,40);
  u8g.print(vehicleSpeed);
  u8g.setFont(u8g_font_fixed_v0);
  u8g.setPrintPos(0,10);
  u8g.print("atfTemp:");
  u8g.setPrintPos(0,20);
  u8g.print(atfTemp);
  u8g.setPrintPos(0,30);
  u8g.print("oilTemp:");
  u8g.setPrintPos(0,40);
  u8g.print(oilTemp);
  u8g.setPrintPos(0,50);
  u8g.print("Boost:");
  u8g.setPrintPos(0,60);
  u8g.print(boostPressure);
  u8g.setPrintPos(100,10);
  u8g.print("TPS:");
  u8g.setPrintPos(100,20);
  u8g.print(tpsPercentValue);
  u8g.setPrintPos(100,30);
  u8g.print("RPM:");
  u8g.setPrintPos(100,40);
  u8g.print(rpmValue);
}
// Display update
void updateDisplay() {
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
   u8g.firstPage();  
  do {
    draw();
  } while( u8g.nextPage() );
}
