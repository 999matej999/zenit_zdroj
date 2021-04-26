#ifndef __VARIABLES_H__
#define __VARIABLES_H__

extern float U1setpoint; // channel 1 voltage setpoint
extern float U2setpoint; 
extern float U3setpoint; 
extern float U4setpoint; 

extern float U1meas; // channel 1 measured voltage
extern float U2meas; 
extern float U3meas; 
extern float U4meas; 

const float Umax = 30.0001; // maximum voltage
const float Umin = 0.000; // minimum voltage

extern float I1setpoint; // channel 1 current setpoint
extern float I2setpoint; 
extern float I3setpoint; 
extern float I4setpoint; 

extern float I1meas; // channel 1 measured current
extern float I2meas; 
extern float I3meas; 
extern float I4meas; 

const float Imax = 3.0001; // maximum current
const float Imin = 0.000; // minimum current

extern bool Ch1Enabled; // channel 1 output enable
extern bool Ch2Enabled;
extern bool Ch3Enabled;
extern bool Ch4Enabled;

extern bool OutEnabled; // general power supply output ON/OFF

extern bool Ch1SetActive; // setting of U or I, channel 1 active
extern bool Ch2SetActive;
extern bool Ch3SetActive;
extern bool Ch4SetActive;

extern bool Ch1Status; // readback of the channel status
extern bool Ch2Status;
extern bool Ch3Status;
extern bool Ch4Status;

extern bool Ch1Ilimit; // readback of the current limit status
extern bool Ch2Ilimit;
extern bool Ch3Ilimit;
extern bool Ch4Ilimit;

extern bool Fuse1Ena; // channel 1 electronic fuse enabled
extern bool Fuse2Ena;
extern bool Fuse3Ena;
extern bool Fuse4Ena;

extern bool Fuse1Trip; // channel 1 electronic fuse readback
extern bool Fuse2Trip;
extern bool Fuse3Trip;
extern bool Fuse4Trip;

extern bool Fuse1Reset; // channel 1 electronic fuse readback
extern bool Fuse2Reset;
extern bool Fuse3Reset;
extern bool Fuse4Reset;

#endif // __VARIABLES_H__ //
