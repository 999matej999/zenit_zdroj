String msg2;
#define DBG(X) msg2 = "-";\
msg2 += X;\
msg2 += "-";\
Serial.println(msg2)

// include the library code:
#include <LiquidCrystal.h>
#include <SPI.h>
#include "variables.h"
#include "scpi.h"

#define LOAD 4

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

boolean SettingU = false; // setting voltage active
boolean SettingI = false; // setting current active
boolean SettingFuse = false; // setting fuse active

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

/* EEPROM & step setting UPDATE by Nitram147 (Martin Eršek) */

#include <avr/eeprom.h>

//define EEPROM addresses (they had been choosen randomly, so they can be changed)
//addresses of preset voltage (incremented by 2 because we will store voltage as unsigned 16bit int)
//low 8bit are stored on first address, high 8bit are stored on second address 
#define CH1VOLTAGEADDR 50
#define CH2VOLTAGEADDR 52
#define CH3VOLTAGEADDR 54
#define CH4VOLTAGEADDR 56

//addresses of preset current (again incremented by 2)
#define CH1CURRENTADDR 60
#define CH2CURRENTADDR 62
#define CH3CURRENTADDR 64
#define CH4CURRENTADDR 66

//adresses of preset steps (voltage + current adjusting step size)
#define VOLTAGESTEPADDR 70
#define CURRENTSTEPADDR 71

//adresses of preset fuses
#define CH1FUSEADDR 80
#define CH2FUSEADDR 81
#define CH3FUSEADDR 82
#define CH4FUSEADDR 83

//variables needed for step adjusting
//first element in array is unused - because we want to have steps addressed from 1 not 0
volatile uint8_t voltage_step=1;
const float voltage_steps[5]={0.0,0.1,0.01,0.001,1.0};

volatile uint8_t current_step=1;
const float current_steps[5]={0.0,0.1,0.01,0.001,1.0};

//functions

//load preset values from EEPROM during startup time
void load_preset_values_from_eeprom(){

  uint16_t tmp_ch1_v, tmp_ch2_v, tmp_ch3_v, tmp_ch4_v;
  uint16_t tmp_ch1_c, tmp_ch2_c, tmp_ch3_c, tmp_ch4_c;
  uint8_t tmp_voltage_step, tmp_current_step;
  uint8_t tmp_ch1_f, tmp_ch2_f, tmp_ch3_f, tmp_ch4_f;

  tmp_ch1_v = eeprom_read_word((uint16_t*)CH1VOLTAGEADDR);
  tmp_ch2_v = eeprom_read_word((uint16_t*)CH2VOLTAGEADDR);
  tmp_ch3_v = eeprom_read_word((uint16_t*)CH3VOLTAGEADDR);
  tmp_ch4_v = eeprom_read_word((uint16_t*)CH4VOLTAGEADDR);

  tmp_ch1_c = eeprom_read_word((uint16_t*)CH1CURRENTADDR);
  tmp_ch2_c = eeprom_read_word((uint16_t*)CH2CURRENTADDR);
  tmp_ch3_c = eeprom_read_word((uint16_t*)CH3CURRENTADDR);
  tmp_ch4_c = eeprom_read_word((uint16_t*)CH4CURRENTADDR);

  tmp_voltage_step = eeprom_read_byte((uint8_t*)VOLTAGESTEPADDR);
  tmp_current_step = eeprom_read_byte((uint8_t*)CURRENTSTEPADDR);

  tmp_ch1_f = eeprom_read_byte((uint8_t*)CH1FUSEADDR);
  tmp_ch2_f = eeprom_read_byte((uint8_t*)CH2FUSEADDR);
  tmp_ch3_f = eeprom_read_byte((uint8_t*)CH3FUSEADDR);
  tmp_ch4_f = eeprom_read_byte((uint8_t*)CH4FUSEADDR);

  if(tmp_ch1_v != 0xFFFF){U1setpoint = (tmp_ch1_v/1000.000);}else{U1setpoint = 0.0;}
  if(tmp_ch2_v != 0xFFFF){U2setpoint = (tmp_ch2_v/1000.000);}else{U2setpoint = 0.0;}
  if(tmp_ch3_v != 0xFFFF){U3setpoint = (tmp_ch3_v/1000.000);}else{U3setpoint = 0.0;}
  if(tmp_ch4_v != 0xFFFF){U4setpoint = (tmp_ch4_v/1000.000);}else{U4setpoint = 0.0;}

  if(tmp_ch1_c != 0xFFFF){I1setpoint = (tmp_ch1_c/1000.000);}else{I1setpoint = 0.0;}
  if(tmp_ch2_c != 0xFFFF){I2setpoint = (tmp_ch2_c/1000.000);}else{I2setpoint = 0.0;}
  if(tmp_ch3_c != 0xFFFF){I3setpoint = (tmp_ch3_c/1000.000);}else{I3setpoint = 0.0;}
  if(tmp_ch4_c != 0xFFFF){I4setpoint = (tmp_ch4_c/1000.000);}else{I4setpoint = 0.0;}

  if(tmp_voltage_step != 0xFF){voltage_step = tmp_voltage_step;}else{tmp_voltage_step = 1;}
  if(tmp_current_step != 0xFF){current_step = tmp_current_step;}else{tmp_current_step = 1;}

  if(tmp_ch1_f != 0xFF){if(tmp_ch1_f == 1){Fuse1Ena = true;}else{Fuse1Ena = false;}}else{Fuse1Ena = false;}
  if(tmp_ch2_f != 0xFF){if(tmp_ch2_f == 1){Fuse2Ena = true;}else{Fuse2Ena = false;}}else{Fuse2Ena = false;}
  if(tmp_ch3_f != 0xFF){if(tmp_ch3_f == 1){Fuse3Ena = true;}else{Fuse3Ena = false;}}else{Fuse3Ena = false;}
  if(tmp_ch4_f != 0xFF){if(tmp_ch4_f == 1){Fuse4Ena = true;}else{Fuse4Ena = false;}}else{Fuse4Ena = false;}

}

//parameter is for saving EEPROM deprecation - we don't need to rewrite unchanged step
//parameter 1 for voltage, 2 for current
void save_steps_into_eeprom(uint8_t tmp_which_step){

  if(tmp_which_step == 1){
    eeprom_write_byte((uint8_t*)VOLTAGESTEPADDR,voltage_step);
  }else if(tmp_which_step == 2){
    eeprom_write_byte((uint8_t*)CURRENTSTEPADDR, current_step);
  }

}

//save voltage or current value to EEPROM depending on parameters
//tmp_which_value => 1 for voltage; 2 for current
//tmp_which_channel => number of channel (1-4)
void save_values_into_eeprom(uint8_t tmp_which_value, uint8_t tmp_which_channel){
  if(tmp_which_value == 1){

    uint16_t tmp_ch1_v, tmp_ch2_v, tmp_ch3_v, tmp_ch4_v;

    if(tmp_which_channel == 1){tmp_ch1_v = (uint16_t)(U1setpoint * 1000);eeprom_write_word((uint16_t*)CH1VOLTAGEADDR, tmp_ch1_v);}
    else if(tmp_which_channel == 2){tmp_ch2_v = (uint16_t)(U2setpoint * 1000);eeprom_write_word((uint16_t*)CH2VOLTAGEADDR, tmp_ch2_v);}
    else if(tmp_which_channel == 3){tmp_ch3_v = (uint16_t)(U3setpoint * 1000);eeprom_write_word((uint16_t*)CH3VOLTAGEADDR, tmp_ch3_v);}
    else if(tmp_which_channel == 4){tmp_ch4_v = (uint16_t)(U4setpoint * 1000);eeprom_write_word((uint16_t*)CH4VOLTAGEADDR, tmp_ch4_v);}

  }else if(tmp_which_value == 2){

    uint16_t tmp_ch1_c, tmp_ch2_c, tmp_ch3_c, tmp_ch4_c;

    if(tmp_which_channel == 1){tmp_ch1_c = (uint16_t)(I1setpoint * 1000);eeprom_write_word((uint16_t*)CH1CURRENTADDR, tmp_ch1_c);}
    else if(tmp_which_channel == 2){tmp_ch2_c = (uint16_t)(I2setpoint * 1000);eeprom_write_word((uint16_t*)CH2CURRENTADDR, tmp_ch2_c);}
    else if(tmp_which_channel == 3){tmp_ch3_c = (uint16_t)(I3setpoint * 1000);eeprom_write_word((uint16_t*)CH3CURRENTADDR, tmp_ch3_c);}
    else if(tmp_which_channel == 4){tmp_ch4_c = (uint16_t)(I4setpoint * 1000);eeprom_write_word((uint16_t*)CH4CURRENTADDR, tmp_ch4_c);}

  }
}

//parameter = channel of fuse
void save_fuses_into_eeprom(uint8_t tmp_which){

  uint8_t tmp_state_of_fuse;

  if(tmp_which == 1){if(Fuse1Ena){tmp_state_of_fuse=1;}else{tmp_state_of_fuse=0;}eeprom_write_byte((uint8_t*)CH1FUSEADDR, tmp_state_of_fuse);}
  else if(tmp_which == 2){if(Fuse2Ena){tmp_state_of_fuse=1;}else{tmp_state_of_fuse=0;}eeprom_write_byte((uint8_t*)CH2FUSEADDR, tmp_state_of_fuse);}
  else if(tmp_which == 3){if(Fuse3Ena){tmp_state_of_fuse=1;}else{tmp_state_of_fuse=0;}eeprom_write_byte((uint8_t*)CH3FUSEADDR, tmp_state_of_fuse);}
  else if(tmp_which == 4){if(Fuse4Ena){tmp_state_of_fuse=1;}else{tmp_state_of_fuse=0;}eeprom_write_byte((uint8_t*)CH4FUSEADDR, tmp_state_of_fuse);}

}

//because of mistake in schematic (encoder button connected to ADC input instead of I/O pin)
void encoder_button_init(){
  ADMUX = (1<<MUX2) | (1<<MUX1) | (1<<MUX0) | (1<<REFS0);
  ADCSRA = (1<<ADEN) | (1<<ADSC) | (1<<ADATE) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
}

//return true when pressed
static uint8_t encoder_button_pressed(){
  return (ADC < 512);
}

//adjust step if we are in setting mode & encoder button is pressed
void encoder_check_for_adding_of_step(){
  if(encoder_button_pressed()){
    if(SettingU){if(voltage_step<4){voltage_step++;}else{voltage_step=1;}save_steps_into_eeprom(1);}
    else if(SettingI){if(current_step<4){current_step++;}else{current_step=1;}save_steps_into_eeprom(2);}
    //wait for encoder button release
    while(encoder_button_pressed());
  }
}

/* ---------------/- UPDATE -/---------------*/
/* Display cursor update by Nitram147 (Martin Eršek) */

//0 - off; anything else - on
void change_cursor(uint8_t tmp_state){if(tmp_state){lcd.cursor();}else{lcd.noCursor();}}

//goto x location on display
void display_goto_x(uint8_t tmp_x){

  uint8_t tmp_y=0;

  if(Ch1SetActive){tmp_y=0;
  }else if(Ch2SetActive){tmp_y=1;
  }else if(Ch3SetActive){tmp_y=2;
  }else if(Ch4SetActive){tmp_y=3;}

  lcd.setCursor(tmp_x, tmp_y);
}

//find if cursor should or shouldn't be enabled
//if yes navigate to the right position based on actual adjusting step & active setting mode
void check_for_cursor(){

  if((SettingI || SettingU || SettingFuse) && (Ch1SetActive || Ch2SetActive || Ch3SetActive || Ch4SetActive)){
 
    change_cursor(1);

    if(SettingU){

      if(voltage_step == 1){display_goto_x(3);}
      else if(voltage_step == 2){display_goto_x(4);}
      else if(voltage_step == 3){display_goto_x(5);}
      else if(voltage_step == 4){display_goto_x(1);}

    }else if(SettingI){

      if(current_step == 1){display_goto_x(10);}
      else if(current_step == 2){display_goto_x(11);}
      else if(current_step == 3){display_goto_x(12);}
      else if(current_step == 4){display_goto_x(8);}

    }else if(SettingFuse){
      display_goto_x(15);
    }

  }else{
    change_cursor(0);
  }

}

/* --------------/- UPDATE -/----------------*/


void setup() { 
  // put your setup code here, to run once:
  pinMode(LOAD, OUTPUT);
  digitalWrite(LOAD, HIGH);

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
  PCMSK2 |= (1<<PCINT2);
  PCICR |= (1<<PCIE2);
  
  /*     UPDATE     */  
  load_preset_values_from_eeprom();
  encoder_button_init();
  /* -/- UPDATE -/- */

  sei(); //enable interrupts
   
  // initialize SPI:
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2); 
   
  // initialize serial:
  Serial.begin(9600); 
  mySerial.begin(baudrate); // TODO: SAVE baudrate to EEPROM

  // reserve 30 bytes for the inputString:
  inputString.reserve(30);
  tmpString.reserve(6);
  
  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);

  updateOutEnabled = true;
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
    digitalWrite(LOAD, LOW);
    digitalWrite(LOAD, HIGH);
    
    SPIrcv = SPI.transfer16(SPItx);
    buttons = highByte(SPIrcv);

} // end interrupt service routine timer


// interrupt service routine pin change for group D0 to D7
// 
ISR(PCINT2_vect)
{
  SoftwareSerial::handle_interrupt();

  if(!rw_lock)
  {
    EncAprev = EncAcur;
    EncBprev = EncBcur;
    EncAcur = PIND & (1<<PD2);  
    EncBcur = PIND & (1<<PD3);

    if (EncAprev == false && EncAcur == true) // rising edge of A
    {
     if (EncBcur == true) 
      { // decrement

        --EncIncrement;
      }
      else
      { // increment

        ++EncIncrement;
      }
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

    if(!rw_lock) keyboard();    // keyboard routine. Check if any key was depressed and process it

    if(!rw_lock) setVoltageCurrentFuse(); // check if setting mode is active, if yes, modify the trimmed parameter

    LEDs(); // prepare data for button LEDs
    
    if (stringComplete) { decodePacket(); }  // if packet received decode it
    
    DisplayData();  // prepare and write all data to the LCD screen

    check_for_cursor();

  if(mySerial.available())
  {
    buffer[idx] = mySerial.read();
    if(buffer[idx] == '\n')
    {
      buffer[idx] = '\0';
      idx = 0;

      cmd_arrived(RECEIVER::UART);
    }
    else
    {
      ++idx;
    }
  }

  if(updateOutEnabled)
    {
      updateOutEnabled = false;
      if(OutEnabled) sendAllOn(); // switch on all outputs
      else sendAllOff(); // switch off all outputs
    }
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
          int fuseReset = 0;
          switch (Addr) {
          case 0: {Utmp = U1setpoint; Itmp = I1setpoint; fuse = Fuse1Ena; output = Ch1Enabled; fuseReset = Fuse1Reset;}  break;
          case 1: {Utmp = U2setpoint; Itmp = I2setpoint; fuse = Fuse2Ena; output = Ch2Enabled; fuseReset = Fuse2Reset;}  break;
          case 2: {Utmp = U3setpoint; Itmp = I3setpoint; fuse = Fuse3Ena; output = Ch3Enabled; fuseReset = Fuse3Reset;}  break;
          case 3: {Utmp = U4setpoint; Itmp = I4setpoint; fuse = Fuse4Ena; output = Ch4Enabled; fuseReset = Fuse4Reset;}  break;
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
          Serial.print(fuseReset,1); // clear fuse
          if(fuseReset == 1)
          switch (Addr) {
          case 0: {Fuse1Reset = false;}  break;
          case 1: {Fuse2Reset = false;}  break;
          case 2: {Fuse3Reset = false;}  break;
          case 3: {Fuse4Reset = false;}  break;
          default: break;
          }
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
        
          delay(50);
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
        
          delay(50);
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
        if (Ch1Ilimit || Fuse1Trip) {led1 = red;} else {led1 = green;} // when channel enabled use green if ok, or red if current over limit
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
        if (Ch2Ilimit || Fuse2Trip) {led2 = red;} else {led2 = green;} // when channel enabled use green if ok, or red if current over limit
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
        if (Ch3Ilimit || Fuse3Trip) {led3 = red;} else {led3 = green;} // when channel enabled use green if ok, or red if current over limit
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
        if (Ch4Ilimit || Fuse4Trip) {led4 = red;} else {led4 = green;} // when channel enabled use green if ok, or red if current over limit
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

       if (inputString.substring(4, 6) == "P0") 
        {
          //fuse tripped
        switch (tmpAddr) {
        case 0: Fuse1Trip = true; break;
        case 1: Fuse2Trip = true; break;
        case 2: Fuse3Trip = true; break;
        case 3: Fuse4Trip = true; break;
        default: break;}
        }
        else
        {
          //fuse not tripped
        switch (tmpAddr) {
        case 0: Fuse1Trip = false; break;
        case 1: Fuse2Trip = false; break;
        case 2: Fuse3Trip = false; break;
        case 3: Fuse4Trip = false; break;
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
    //DBG(inputString);
    inputString = "";
    stringComplete = false;

  }  // end decode received packet
  
  
  // prepare text for display
void DisplayData(){

change_cursor(0); //temporary disable cursor while writing on to display

float U = 0.0; // temporary variable to hold U value for display
float I = 0.0; // temporary variable to hold I value for display
float P = 0.0; // temporary variable to hold P value for display
boolean fuse = false;
boolean limit = false;
boolean trip = false;

  for (int riadok=0; riadok <= 3; riadok++){
    switch (riadok) {
      case 0: 
        if (!Ch1Enabled | Ch1SetActive) 
          { U = U1setpoint; I = I1setpoint;}
         else
          { U = U1meas; I = I1meas;}  
        fuse = Fuse1Ena; 
        limit = Ch1Ilimit;
        trip = Fuse1Trip;
        break;
      case 1: 
        if (!Ch2Enabled | Ch2SetActive)
          {U = U2setpoint; I = I2setpoint;}  
          else
          {U = U2meas; I = I2meas;} 
        fuse = Fuse2Ena;  
        limit = Ch2Ilimit;
        trip = Fuse2Trip;
        break;
      case 2: 
        if (!Ch3Enabled | Ch3SetActive)
          {U = U3setpoint; I = I3setpoint;}   
          else
          {U = U3meas; I = I3meas;}
        fuse = Fuse3Ena;  
        limit = Ch3Ilimit;
        trip = Fuse3Trip;
        break;
      case 3: 
        if (!Ch4Enabled | Ch4SetActive) 
          {U = U4setpoint; I = I4setpoint;}
          else
          {U = U4meas; I = I4meas;} 
        fuse = Fuse4Ena;  
        limit = Ch4Ilimit;
        trip = Fuse4Trip;
        break;
      default: U = 99.9; I = 9.9; break;
      }
    
    lcd.setCursor(0, riadok);
    if (U < 10.000) 
      {lcd.print(" "); lcd.print(U,3); lcd.print("V");}
    else
      {lcd.print(U,3); lcd.print("V");}
    lcd.print(" ");
    
    lcd.setCursor(8, riadok); lcd.print(I,3); lcd.print("A");
    
    lcd.setCursor(15, riadok);
    if (fuse) {
      if (trip) {
        lcd.print("Tr");
      }
      else {
        lcd.print("Fu");
      }
    }
    else {
      lcd.print("  ");
    } 
      
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
        U1setpoint = U1setpoint + (EncIncrement*voltage_steps[voltage_step]);
        if(U1setpoint > Umax){
            U1setpoint = Umax;
        }else if(U1setpoint < Umin){
          U1setpoint = Umin;
        }
        save_values_into_eeprom(1,1);
      }
      
      if (Ch2SetActive == true)
      {
        U2setpoint = U2setpoint + (EncIncrement*voltage_steps[voltage_step]);
        if(U2setpoint > Umax){
            U2setpoint = Umax;
        }else if(U2setpoint < Umin){
          U2setpoint = Umin;
        }
        save_values_into_eeprom(1,2);
      }

      if (Ch3SetActive == true)
      {
        U3setpoint = U3setpoint + (EncIncrement*voltage_steps[voltage_step]);
        if(U3setpoint > Umax){
            U3setpoint = Umax;
        }else if(U3setpoint < Umin){
          U3setpoint = Umin;
        }
        save_values_into_eeprom(1,3);
      }
      
      if (Ch4SetActive == true)
      {
        U4setpoint = U4setpoint + (EncIncrement*voltage_steps[voltage_step]);
        if(U4setpoint > Umax){
            U4setpoint = Umax;
        }else if(U4setpoint < Umin){
          U4setpoint = Umin;
        }
        save_values_into_eeprom(1,4);
      }

      EncIncrement = 0.0;
    }
    
    if (SettingI) // setting current active
    {
      if (Ch1SetActive == true)
      {
        I1setpoint = I1setpoint + (EncIncrement*current_steps[current_step]);
        if(I1setpoint > Imax){
            I1setpoint = Imax;
        }else if(I1setpoint < Imin){
          I1setpoint = Imin;
        }
        save_values_into_eeprom(2,1);
      }
      if (Ch2SetActive == true)
      {
        I2setpoint = I2setpoint + (EncIncrement*current_steps[current_step]);
        if(I2setpoint > Imax){
            I2setpoint = Imax;
        }else if(I2setpoint < Imin){
          I2setpoint = Imin;
        }
        save_values_into_eeprom(2,2);
      }
      if (Ch3SetActive == true)
      {
        I3setpoint = I3setpoint + (EncIncrement*current_steps[current_step]);
        if(I3setpoint > Imax){
            I3setpoint = Imax;
        }else if(I3setpoint < Imin){
          I3setpoint = Imin;
        }
        save_values_into_eeprom(2,3);
      }
      if (Ch4SetActive == true)
      {
        I4setpoint = I4setpoint + (EncIncrement*current_steps[current_step]);
        if(I4setpoint > Imax){
            I4setpoint = Imax;
        }else if(I4setpoint < Imin){
          I4setpoint = Imin;
        }
        save_values_into_eeprom(2,4);
      }
      
      EncIncrement = 0.0; // clear the latest increment
    }    

    if (SettingFuse) // setting fuse active
    {
      if (Ch1SetActive == true)
      {
        if (EncIncrement > 0.0) // enable fuse
        {Fuse1Ena = true;save_fuses_into_eeprom(1);}
        if (EncIncrement < 0.0) // disable fuse
        {Fuse1Ena = false;save_fuses_into_eeprom(1);}
      }
      if (Ch2SetActive == true)
      {
        if (EncIncrement > 0.0) // enable fuse
        {Fuse2Ena = true;save_fuses_into_eeprom(2);}
        if (EncIncrement < 0.0) // disable fuse
        {Fuse2Ena = false;save_fuses_into_eeprom(2);}
      }      
      if (Ch3SetActive == true)
      {
        if (EncIncrement > 0.0) // enable fuse
        {Fuse3Ena = true;save_fuses_into_eeprom(3);}
        if (EncIncrement < 0.0) // disable fuse
        {Fuse3Ena = false;save_fuses_into_eeprom(3);}
      }
      if (Ch4SetActive == true)
      {
        if (EncIncrement > 0.0) // enable fuse
        {Fuse4Ena = true;save_fuses_into_eeprom(4);}
        if (EncIncrement < 0.0) // disable fuse
        {Fuse4Ena = false;save_fuses_into_eeprom(4);}
      }     
      EncIncrement = 0.0; // clear the used-up increment

    } // end setting parameters
    
    EncIncrement = 0.0; // if setting mode is not active keep increment 0
} // end set voltage current and fuse


// process keyboard

void keyboard() {
  
  encoder_check_for_adding_of_step();
  
  // key Channel 1
    if ((buttons & (1 << button1)) != 0)  // button1 depressed
    {
      DBG("DBG");
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
          if (!Ch1Enabled) Fuse1Reset = true;
        }
    }
    
    // key Channel 2
    if ((buttons & (1 << button2)) != 0)  
    { 
      DBG("DBG");
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
          if (!Ch2Enabled) Fuse2Reset = true;
        }
    }

    if ((buttons & (1 << button3)) != 0)  // key Channel 3
    { 
      DBG("DBG");
      while ((buttons & (1 << button3)) != 0)
      {
        delay(1); //wait until the button is released
      }
      if (SettingU == true || SettingI == true || SettingFuse == true) // if U,I setting mode active
        { 
          Ch1SetActive = false;
          Ch2SetActive = false;
          Ch3SetActive = true;
          Ch4SetActive = false;
        }
        else
        {          // U,I setting mode not active, toggle Output enable
          Ch3SetActive = false;      
          Ch3Enabled = !Ch3Enabled; // toggle enable status
          if (!Ch3Enabled) Fuse3Reset = true;
        }
    }

    if ((buttons & (1 << button4)) != 0)  // key Channel 4
    { 
      DBG("DBG");
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
          if (!Ch4Enabled) Fuse4Reset = true;
        }
    }

    if ((buttons & (1 << button5)) != 0)  // key U setpoint
    { 
      DBG("DBG");
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
      DBG("DBG");
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
      DBG("DBG");
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
      DBG("DBG");
      while ((buttons & (1 << button8)) != 0) 
      {
        delay(1); //wait until the button is released
      }
       OutEnabled = !OutEnabled; // enable/disable output
       updateOutEnabled = true;
    }
    
} // end keyboard routine



