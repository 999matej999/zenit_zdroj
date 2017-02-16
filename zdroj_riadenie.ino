
// include the library code:
#include <LiquidCrystal.h>
#include <SPI.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(15, 14, 16, 17, 18, 19);

// button bits
#define button1  0
#define button2  1
#define button3  2
#define button4  3
#define button5  4
#define button6  5
#define button7  6
#define button8  7

// LED colors
#define none 0
#define red 1
#define green 2
#define orange 13

//float[] _setpoints = {0,0,0,0};
//_set[CHANNEL1]=;

String inputString = "";         // a string to hold incoming data
String tmpString = ""; // temp string
boolean stringComplete = false;  // whether the string is complete

float U1setpoint = 11.000; // channel 1 voltage setpoint
float U2setpoint = 12.000; 
float U3setpoint = 13.000; 
float U4setpoint = 14.000; 

float U1meas = 0.001; // channel 1 measured voltage
float U2meas = 0.002; 
float U3meas = 0.003; 
float U4meas = 0.004; 

const float Umax = 30.0001; // maximum voltage
const float Umin = 0.000; // minimum voltage

float I1setpoint = 0.500; // channel 1 current setpoint
float I2setpoint = 1.0; 
float I3setpoint = 1.5; 
float I4setpoint = 2.0; 

float I1meas = 0.010; // channel 1 measured current
float I2meas = 0.020; 
float I3meas = 0.030; 
float I4meas = 0.040; 

const float Imax = 3.0001; // maximum current
const float Imin = 0.000; // minimum current

boolean Ch1Enabled = false; // channel 1 output enable
boolean Ch2Enabled = false;
boolean Ch3Enabled = false;
boolean Ch4Enabled = false;

boolean OutEnabled = false; // general power supply output ON/OFF

boolean SettingU = false; // setting voltage active
boolean SettingI = false; // setting current active
boolean SettingFuse = false; // setting fuse active

boolean Ch1SetActive = false; // setting of U or I, channel 1 active
boolean Ch2SetActive = false;
boolean Ch3SetActive = false;
boolean Ch4SetActive = false;

boolean Ch1Status = false; // readback of the channel status
boolean Ch2Status = false;
boolean Ch3Status = false;
boolean Ch4Status = false;

boolean Ch1Ilimit = false; // readback of the current limit status
boolean Ch2Ilimit = false;
boolean Ch3Ilimit = false;
boolean Ch4Ilimit = false;

boolean Fuse1Ena = false; // channel 1 electronic fuse enabled
boolean Fuse2Ena = false;
boolean Fuse3Ena = false;
boolean Fuse4Ena = false;

boolean Fuse1Trip = false; // channel 1 electronic fuse readback
boolean Fuse2Trip = false;
boolean Fuse3Trip = false;
boolean Fuse4Trip = false;

boolean blinkphase = false; // internal for LED routine

boolean EncAprev = false; // previous state of A encoder output
boolean EncAcur = false; // current state of A encoder output
boolean EncBprev = false; // previous state of B encoder output
boolean EncBcur = false; // current state of B encoder output
boolean EncButton = false;

float EncIncrement = 0.0;

int led1 = none; int led2 = none; int led3 = none; int led4 = none; //0 off, 1 red, 2 green, 13 orange
int led5 = red; int led6 = green; int led7 = orange; int led8 = none;

byte buttons = 0;
byte moduleToSend = 0; // module to which the next packet will be sent
boolean sendingPacket = false;

void setup() { 
  // put your setup code here, to run once:
  DDRD = 0b00010000;
  PORTD |= 0b00010000; // LOAD to one

  // initialize timer 2 for keyboard sensing interrupt
  cli();//stop interrupts
  //set timer2 interrupt at 300 Hz
    TCCR2A = 0;// set entire TCCR2A register to 0
    TCCR2B = 0;// same for TCCR2B
    TCNT2  = 0;//initialize counter value to 0
    // set compare match register for 100 Hz increments
    OCR2A = 156;// 156 = (16*10^6) / (100*1024) - 1 (must be <256)
    // turn on CTC mode
    TCCR2A |= (1 << WGM21);
    // Set CS21 bit for 1024 prescaler
    TCCR2B |= (1<<CS22) | (1<<CS21) | (1<<CS20); // Set bits ;   
    // enabLE0 timer compare interrupt
    TIMSK2 |= (1 << OCIE2A);
    
  // initialize pin change interrupt for encoder
    PCMSK2 = (1<<PCINT2);
    PCICR = (1<<PCIE2);
    
  sei(); //enable interrupts
   
  // initialize SPI:
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2); 
   
  // initialize serial:
  Serial.begin(9600); 
  // reserve 30 bytes for the inputString:
  inputString.reserve(30);
  tmpString.reserve(6);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);
}


// interrupt service routine timer
// 
ISR(TIMER2_COMPA_vect)        
{   
    int SPIrcv = 0; // received from SPI
    int SPItx = 0; // send to SPI
    int txMask = 0; // toggle mask for orange colour

    SPItx = (led8 & 3) << 14; // take only 2 LSBs
    SPItx |= ((led7 & 3) << 12);
    SPItx |= ((led6 & 3) << 10);
    SPItx |= ((led5 & 3) << 8);
    SPItx |= ((led4 & 3) << 6);
    SPItx |= ((led3 & 3) << 4);
    SPItx |= ((led2 & 3) << 2);
    SPItx |= ((led1 & 3));

    txMask = (led8 & 12) << 12;
    txMask |= ((led7 & 12) << 10);
    txMask |= ((led6 & 12) << 8);
    txMask |= ((led5 & 12) << 6);
    txMask |= ((led4 & 12) << 4);
    txMask |= ((led3 & 12) << 2);
    txMask |= ((led2 & 12));
    txMask |= ((led1 & 12) >>2);
    if (blinkphase)
            {SPItx = SPItx ^ txMask;}
    blinkphase = !blinkphase;
    
    // load the parallel to serial register
    PORTD &= 0b11101111; // LOAD to zero        
    PORTD |= 0b00010000; // LOAD to one
    
    SPIrcv = SPI.transfer16(SPItx);
    buttons = highByte(SPIrcv);

} // end interrupt service routine timer


// interrupt service routine pin change for group D0 to D7
// 
ISR(PCINT2_vect)        
{ 
  EncAprev = EncAcur;
  EncBprev = EncBcur;
  EncAcur = PIND & (1<<PD2);  
  EncBcur = PIND & (1<<PD3);

  if (EncAprev == false && EncAcur == true) // rising edge of A
  {
   if (EncBcur == true) 
    { // decrement
      EncIncrement = -0.1;
    }
    else
    { // increment
      EncIncrement = +0.1;
    }
  }

  
} // end interrupt service routine pin change for group D0 to D7



// ------------------------------------------------------------------------------
// put your main code here, to run repeatedly:

void loop() {
    //lcd.clear(); lcd.setCursor(0, 0);
  // send set point values to all power supplies and read back the status.
    sendSetpoint(moduleToSend);  // 30ms delay inside, can be removed and eventually triggered by an interrupt
    if (moduleToSend < 3) // prepare address for the next module
       { moduleToSend ++; }
      else
       { moduleToSend = 0; }
    //lcd.setCursor(0, 0); lcd.print(moduleToSend);  
    
    keyboard();    // keyboard routine. Check if any key was depressed and process it

    setVoltageCurrentFuse(); // check if setting mode is active, if yes, modify the trimmed parameter

    LEDs(); // prepare data for button LEDs
    
    if (stringComplete) { decodePacket(); }  // if packet received decode it
    
    DisplayData();  // prepare and write all data to the LCD screen
    
} // end main program



// ------------------ various functions ----------------------

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
  
}

// send setpoint to the power supply modules
void sendSetpoint(byte Addr) 
    {
          float Utmp = 0;
          float Itmp = 0;
          int fuse = 0;
          int output = 0;
          switch (Addr) {
          case 0: {Utmp = U1setpoint; Itmp = I1setpoint; fuse = Fuse1Ena; output = Ch1Enabled;}  break;
          case 1: {Utmp = U2setpoint; Itmp = I2setpoint; fuse = Fuse2Ena; output = Ch2Enabled;}  break;
          case 2: {Utmp = U3setpoint; Itmp = I3setpoint; fuse = Fuse3Ena; output = Ch3Enabled;}  break;
          case 3: {Utmp = U4setpoint; Itmp = I4setpoint; fuse = Fuse4Ena; output = Ch4Enabled;}  break;
          default: break;
          }
        
        // telegram format *3V1P0R0U12.345I09.999
        // *3 module address
        // V1 output ON (V0 output OFF)
        // P1 electronic fuse ON (P0 electronic fuse OFF)
        // R1 reset the fuse
        // U12.345 voltage setpoint. Leading zero necessary
        // I09.999 current setpoint. Leading zero necessary
        
          Serial.print('*');
          Serial.print(Addr); // addr
          Serial.print('V');
          //Serial.print('1');          
          Serial.print(output,1);
          Serial.print('P');
          //Serial.print('1');
          Serial.print(fuse,1);
          Serial.print('R');
          Serial.print('0'); // clear fuse
          Serial.print('U');
          if (Utmp < 10.0) {Serial.print('0');} // add zero to conserve format 00.000
          Serial.print(Utmp,3);
          Serial.print('I');
          if (Itmp < 10.0) {Serial.print('0');} // add zero to conserve format 00.000
          Serial.println(Itmp,3); 

          //Serial.println("*3V1P0R0U12.345I09.999"); 

          delay(30);
    }

    
// send command to all power supply modules
void sendAllOn() 
    {
      
        // telegram format *FVZ
        // *F module address
        // VZ all outputs ON, VV all outputs OFF
        
          Serial.print("*F");
          Serial.println("VZ");
          delay(30);
    }    

// send command to all power supply modules
void sendAllOff() 
    {
      
        // telegram format *FVZ
        // *F module address
        // VZ all outputs ON, VV all outputs OFF
        
          Serial.print("*F"); // addr for all is *F
          Serial.println("VV");
          delay(30);
    } 
 
    
 // front panel leds    
 void LEDs() {   
   // LED channel 1
    if (Ch1SetActive)
    {led1 = orange;} // setpoint setting active, orange color
    else
    {    
      if (Ch1Enabled) {
        if (Ch1Ilimit) {led1 = red;} else {led1 = green;} // when channel enabled use green if ok, or red if current limiting
        } 
      else 
        {led1 = none;} // channel not enabled, button led off
    }
    
    // LED channel 2
    if (Ch2SetActive)
    {led2 = orange;} // setpoint setting active, orange color
    else
    {    
      if (Ch2Enabled) {
        if (Ch2Ilimit) {led2 = red;} else {led2 = green;} // when channel enabled use green if ok, or red if current limiting
        } 
      else 
        {led2 = none;} // channel not enabled, button led off
     }

    // LED channel 3
    if (Ch3SetActive)
    {led3 = orange;} // setpoint setting active, orange color
    else
    {    
      if (Ch3Enabled) {
        if (Ch3Ilimit) {led3 = red;} else {led3 = green;} // when channel enabled use green if ok, or red if current limiting
        } 
      else 
        {led3 = none;} // channel not enabled, button led off    
     }
       
    // LED channel 4
    if (Ch4SetActive)
    {led4 = orange;} // setpoint setting active, orange color
    else
    {    
      if (Ch4Enabled) {
        if (Ch4Ilimit) {led4 = red;} else {led4 = green;} // when channel enabled use green if ok, or red if current limiting
        } 
      else 
        {led4 = none;} // channel not enabled, button led off    
     }

    // LED voltage setting button
    if (SettingU) {led5 = orange;} else {led5 = none;}

    // LED current setting button
    if (SettingI) {led6 = orange;} else {led6 = none;}

    // LED fuse setting button
    if (SettingFuse) {led7 = orange;} else {led7 = none;}

    // LED output ON/OFF button
    if (OutEnabled) {led8 = green;} else {led8 = none;}
    
 }  // end prepare button LEDs routine
 
 
 
 // decode packet received from the plug-in module
 void decodePacket()
 {

// received packet with status data *3V1P0R0U12.345I09.999  
// *3 module address (*0 .. *3)
// V1 output 1 = on/ 0 = off
// P0 fuse status 0 = ok/ 1 = tripped    
// R0 current mode 1 = limiting/ 0 = voltage mode
// U12.345 measured voltage, format xx.xxx
// I09.999 measured current, format xx.xxx

    //lcd.print(inputString); while(1) {};

    int tmpAddr = 255;
    if (inputString.substring(0,2) == "*0") {tmpAddr = 0;}
    if (inputString.substring(0,2) == "*1") {tmpAddr = 1;}
    if (inputString.substring(0,2) == "*2") {tmpAddr = 2;}
    if (inputString.substring(0,2) == "*3") {tmpAddr = 3;} 
   
    if (tmpAddr != 255)
    { // prijaty telegram ma platnu adresu modulu
        
      if (inputString.substring(2, 4) == "V1") 
        {
          //out ON
        switch (tmpAddr) {
        case 0: Ch1Status = true; break;
        case 1: Ch2Status = true; break;
        case 2: Ch3Status = true; break;
        case 3: Ch4Status = true; break;
        default: break;}
        } 
        else
        {
          //out OFF
        switch (tmpAddr) {
        case 0: Ch1Status = false; break;
        case 1: Ch2Status = false; break;
        case 2: Ch3Status = false; break;
        case 3: Ch4Status = false; break;
        default: break;}          
        } 

       if (inputString.substring(4, 6) == "P1") 
        {
          //fuse tripped
        switch (tmpAddr) {
        case 0: Fuse1Trip = true; break;
        case 1: Fuse1Trip = true; break;
        case 2: Fuse1Trip = true; break;
        case 3: Fuse1Trip = true; break;
        default: break;}        } 
        else
        {
          //fuse not tripped
        switch (tmpAddr) {
        case 0: Fuse1Trip = false; break;
        case 1: Fuse1Trip = false; break;
        case 2: Fuse1Trip = false; break;
        case 3: Fuse1Trip = false; break;
        default: break;}          
        } 
    
       if (inputString.substring(6, 8) == "R1") 
        {
        //current limiting
        switch (tmpAddr) {
        case 0: Ch1Ilimit = true; break;
        case 1: Ch2Ilimit = true; break;
        case 2: Ch3Ilimit = true; break;
        case 3: Ch4Ilimit = true; break;
        default: break;}
        }
        else
        {
        //current not limiting
        switch (tmpAddr) {
        case 0: Ch1Ilimit = false; break;
        case 1: Ch2Ilimit = false; break;
        case 2: Ch3Ilimit = false; break;
        case 3: Ch4Ilimit = false; break;
        default: break;}
        }
        
       if (inputString.substring(8, 9) == "U") 
        { tmpString = inputString.substring(9, 15); 
          
        switch (tmpAddr) {
        case 0: U1meas = tmpString.toFloat(); break;
        case 1: U2meas = tmpString.toFloat(); break;
        case 2: U3meas = tmpString.toFloat(); break;
        case 3: U4meas = tmpString.toFloat(); break;
        default: break;}

        } // meranie napatia

       if (inputString.substring(15, 16) == "I") 
        { tmpString = inputString.substring(16, 22); 
        switch (tmpAddr) {
        case 0: I1meas = tmpString.toFloat(); break;
        case 1: I2meas = tmpString.toFloat(); break;
        case 2: I3meas = tmpString.toFloat(); break;
        case 3: I4meas = tmpString.toFloat(); break;
        default: break;}
          } // meranie prudu
   
    }    
    // clear the string:
    inputString = "";
    stringComplete = false;

  }  // end decode received packet
  
  
  // prepare text for display
void DisplayData(){
float U = 0.0; // temporary variable to hold U value for display
float I = 0.0; // temporary variable to hold I value for display
float P = 0.0; // temporary variable to hold P value for display
boolean fuse = false;
boolean limit = false;

  for (int riadok=0; riadok <= 3; riadok++){
    switch (riadok) {
      case 0: 
        if (!Ch1Enabled | Ch1SetActive) 
          { U = U1setpoint; I = I1setpoint;}
         else
          { U = U1meas; I = I1meas;}  
        fuse = Fuse1Ena; 
        limit = Ch1Ilimit; 
        break;
      case 1: 
        if (!Ch2Enabled | Ch2SetActive)
          {U = U2setpoint; I = I2setpoint;}  
          else
          {U = U2meas; I = I2meas;} 
        fuse = Fuse2Ena;  
        limit = Ch2Ilimit; 
        break;
      case 2: 
        if (!Ch3Enabled | Ch3SetActive)
          {U = U3setpoint; I = I3setpoint;}   
          else
          {U = U3meas; I = I3meas;}
        fuse = Fuse3Ena;  
        limit = Ch3Ilimit; 
        break;
      case 3: 
        if (!Ch4Enabled | Ch4SetActive) 
          {U = U4setpoint; I = I4setpoint;}
          else
          {U = U4meas; I = I4meas;} 
        fuse = Fuse4Ena;  
        limit = Ch4Ilimit; 
        break;
      default: U = 99.9; I = 9.9; break;
      }
    
    lcd.setCursor(0, riadok);
    if (U < 10.000) 
      {lcd.print(" "); lcd.print(U,3); lcd.print("V");}
    else
      {lcd.print(U,3); lcd.print("V");}
      
    lcd.setCursor(8, riadok); lcd.print(I,3); lcd.print("A");
    
    lcd.setCursor(15, riadok);
    if (fuse) 
      {lcd.print("Fu");}
      else
      {lcd.print("  ");} 
      
    lcd.setCursor(18, riadok);
    if (limit) 
      {lcd.print("Li");}
      else
      {lcd.print("  ");}       
    } 
} // end LCD print routine


// set voltage, current and fuse  
void setVoltageCurrentFuse(){
    if (SettingU) // setting voltage active
    {
      if (Ch1SetActive == true)
      {
        if ((U1setpoint + EncIncrement <= Umax) && (U1setpoint + EncIncrement >= Umin)) // check min/max
        {U1setpoint = U1setpoint + EncIncrement;}
      }
      
      if (Ch2SetActive == true)
      {
        if ((U2setpoint + EncIncrement <= Umax) && (U2setpoint + EncIncrement >= Umin)) // check min/max
        {U2setpoint = U2setpoint + EncIncrement;}
      }

      if (Ch3SetActive == true)
      {
        if ((U3setpoint + EncIncrement <= Umax) && (U3setpoint + EncIncrement >= Umin)) // check min/max
        {U3setpoint = U3setpoint + EncIncrement;}
      }
      
      if (Ch4SetActive == true)
      {
        if ((U4setpoint + EncIncrement <= Umax) && (U4setpoint + EncIncrement >= Umin)) // check min/max
        {U4setpoint = U4setpoint + EncIncrement;}
      }
      EncIncrement = 0.0;
    }
    
    if (SettingI) // setting current active
    {
      if (Ch1SetActive == true)
      {
        if ((I1setpoint + EncIncrement <= Imax) && (I1setpoint + EncIncrement >= Imin)) // check min/max
        {I1setpoint = I1setpoint + EncIncrement;}
      }
      if (Ch2SetActive == true)
      {
        if ((I2setpoint + EncIncrement <= Imax) && (I2setpoint + EncIncrement >= Imin)) // check min/max
        {I2setpoint = I2setpoint + EncIncrement;}
      }
      if (Ch3SetActive == true)
      {
        if ((I3setpoint + EncIncrement <= Imax) && (I3setpoint + EncIncrement >= Imin)) // check min/max
        {I3setpoint = I3setpoint + EncIncrement;}
      }
      if (Ch4SetActive == true)
      {
        if ((I4setpoint + EncIncrement <= Imax) && (I4setpoint + EncIncrement >= Imin)) // check min/max
        {I4setpoint = I4setpoint + EncIncrement;}
      }
      
      EncIncrement = 0.0; // clear the latest increment
    }    

    if (SettingFuse) // setting fuse active
    {
      if (Ch1SetActive == true)
      {
        if (EncIncrement > 0.0) // enable fuse
        {Fuse1Ena = true;}
        if (EncIncrement < 0.0) // disable fuse
        {Fuse1Ena = false;}
      }
      if (Ch2SetActive == true)
      {
        if (EncIncrement > 0.0) // enable fuse
        {Fuse2Ena = true;}
        if (EncIncrement < 0.0) // disable fuse
        {Fuse2Ena = false;}
      }      
      if (Ch3SetActive == true)
      {
        if (EncIncrement > 0.0) // enable fuse
        {Fuse3Ena = true;}
        if (EncIncrement < 0.0) // disable fuse
        {Fuse3Ena = false;}
      }
      if (Ch4SetActive == true)
      {
        if (EncIncrement > 0.0) // enable fuse
        {Fuse4Ena = true;}
        if (EncIncrement < 0.0) // disable fuse
        {Fuse4Ena = false;}
      }     
      EncIncrement = 0.0; // clear the used-up increment

    } // end setting parameters
    
    EncIncrement = 0.0; // if setting mode is not active keep increment 0
} // end set voltage current and fuse


// process keyboard

void keyboard() {
  // key Channel 1
    if ((buttons & (1 << button1)) != 0)  // button1 depressed
    {     
      while ((buttons & (1 << button1)) != 0)
      {  
        delay(1); //wait until the button is released
      }
      
      if (SettingU == true || SettingI == true || SettingFuse == true) // if U,I setting mode active
        { 
          Ch1SetActive = true;
          Ch2SetActive = false;
          Ch3SetActive = false;
          Ch4SetActive = false;
         }
        else
        {          // U,I setting mode not active, toggle Output enable
          Ch1SetActive = false;
          Ch1Enabled = !Ch1Enabled; // toggle enable status
        }
    }
    
    // key Channel 2
    if ((buttons & (1 << button2)) != 0)  
    { 
      while ((buttons & (1 << button2)) != 0) 
      {
        delay(1); //wait until the button is released
      }
      if (SettingU == true || SettingI == true || SettingFuse == true) // if U,I setting mode active
        { 
          Ch1SetActive = false;
          Ch2SetActive = true;
          Ch3SetActive = false;
          Ch4SetActive = false;         }
        else
        {          // U,I setting mode not active, toggle Output enable
          Ch2SetActive = false;      
          Ch2Enabled = !Ch2Enabled; // toggle enable status
        }
    }

    if ((buttons & (1 << button3)) != 0)  // key Channel 3
    { 
      while ((buttons & (1 << button3)) != 0)
      {
        delay(1); //wait until the button is released
      }
      if (SettingU == true || SettingI == true || SettingFuse == true) // if U,I setting mode active
        { 
          Ch1SetActive = false;
          Ch2SetActive = false;
          Ch3SetActive = true;
          Ch4SetActive = false;         }
        else
        {          // U,I setting mode not active, toggle Output enable
          Ch3SetActive = false;      
          Ch3Enabled = !Ch3Enabled; // toggle enable status
        }
    }

    if ((buttons & (1 << button4)) != 0)  // key Channel 4
    { 
      while ((buttons & (1 << button4)) != 0) 
      {
        delay(1); //wait until the button is released
      }
      if (SettingU == true || SettingI == true || SettingFuse == true) // if U,I setting mode active
        { 
          Ch1SetActive = false;
          Ch2SetActive = false;
          Ch3SetActive = false;
          Ch4SetActive = true;         }
        else
        {          // U,I setting mode not active, toggle Output enable
          Ch4SetActive = false;
          Ch4Enabled = !Ch4Enabled; // toggle enable status
        }
    }

    if ((buttons & (1 << button5)) != 0)  // key U setpoint
    { 
      while ((buttons & (1 << button5)) != 0) 
      {
        delay(1); //wait until the button is released
      }
       SettingU = !SettingU;  // toggle U setting mode
       if (SettingU == false) // finished setting U, clear channel flags
       {
         Ch1SetActive = false;
         Ch2SetActive = false;
         Ch3SetActive = false;
         Ch4SetActive = false;         
       }
       if (SettingU == true) 
       {
         SettingI = false;
         SettingFuse = false;
       } 
       
    }

    if ((buttons & (1 << button6)) != 0)  // key I setpoint
    { 
      while ((buttons & (1 << button6)) != 0) 
      {
        delay(1); //wait until the button is released
      }
       SettingI = !SettingI;  // toggle I setting mode
       if (SettingI == false) // finished setting I, clear channel flags
       {
         Ch1SetActive = false;
         Ch2SetActive = false;
         Ch3SetActive = false;
         Ch4SetActive = false;         
       } 
       
       if (SettingI == true) 
       {
         SettingU = false;
         SettingFuse = false;
       }
       
    }

    if ((buttons & (1 << button7)) != 0)  // key Fuse setting
    { 
      while ((buttons & (1 << button7)) != 0) 
      {
        delay(1); //wait until the button is released
      }
       SettingFuse = !SettingFuse;  // toggle fuse setting mode
       if (SettingFuse == false) // finished setting fuse, clear channel flags
       {
         Ch1SetActive = false;
         Ch2SetActive = false;
         Ch3SetActive = false;
         Ch4SetActive = false;         
       } 
       
       if (SettingFuse == true) 
       {
         SettingU = false; 
         SettingI = false;
       }
    }
    
    if ((buttons & (1 << button8)) != 0)  // key Out enable
    { 
      while ((buttons & (1 << button8)) != 0) 
      {
        delay(1); //wait until the button is released
      }
       OutEnabled = !OutEnabled; // enable/disable output
       if (OutEnabled) 
         {sendAllOn();}  // switch on all outputs
       else 
         {sendAllOff();} // switch off all outputs
    }
    
} // end keyboard routine
