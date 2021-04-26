#include "variables.h"

SoftwareSerial mySerial(5, 6);
char buffer[BUFFER_SIZE] = {};
size_t idx = 0;

float U1setpoint = 11.000; // channel 1 voltage setpoint
float U2setpoint = 12.000; 
float U3setpoint = 13.000; 
float U4setpoint = 14.000; 

float U1meas = 0.001; // channel 1 measured voltage
float U2meas = 0.002; 
float U3meas = 0.003; 
float U4meas = 0.004; 

float I1setpoint = 0.500; // channel 1 current setpoint
float I2setpoint = 1.0; 
float I3setpoint = 1.5; 
float I4setpoint = 2.0; 

float I1meas = 0.010; // channel 1 measured current
float I2meas = 0.020; 
float I3meas = 0.030; 
float I4meas = 0.040; 

bool Ch1Enabled = false; // channel 1 output enable
bool Ch2Enabled = false;
bool Ch3Enabled = false;
bool Ch4Enabled = false;

bool OutEnabled = false; // general power supply output ON/OFF

bool Ch1SetActive = false; // setting of U or I, channel 1 active
bool Ch2SetActive = false;
bool Ch3SetActive = false;
bool Ch4SetActive = false;

bool Ch1Status = false; // readback of the channel status
bool Ch2Status = false;
bool Ch3Status = false;
bool Ch4Status = false;

bool Ch1Ilimit = false; // readback of the current limit status
bool Ch2Ilimit = false;
bool Ch3Ilimit = false;
bool Ch4Ilimit = false;

bool Fuse1Ena = false; // channel 1 electronic fuse enabled
bool Fuse2Ena = false;
bool Fuse3Ena = false;
bool Fuse4Ena = false;

bool Fuse1Trip = false; // channel 1 electronic fuse readback
bool Fuse2Trip = false;
bool Fuse3Trip = false;
bool Fuse4Trip = false;

bool Fuse1Reset = false; // channel 1 electronic fuse readback
bool Fuse2Reset = false;
bool Fuse3Reset = false;
bool Fuse4Reset = false;