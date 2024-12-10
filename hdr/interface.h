#ifndef __INTERFACE_H__
#define __INTERFACE_H__
/**************************************************************************************
SBIT DECLERATION
***************************************************************************************/
// Port 0 - Uart & other comm protocols
sbit Trigger_Car = P0 ^ 0;
sbit Echo_Car = P0 ^ 1;
sbit Trigger_Pet = P0 ^ 2;
sbit Echo_Pet = P0 ^ 3;
sbit TX = P0 ^ 4;
sbit RX = P0 ^ 5;
sbit DHT_DATA = P0 ^ 6;
// Port 1 - LEDs are connected to each wire to represent room/area light
sbit Server_OFF = P1 ^ 0;
sbit Server_ON = P1 ^ 1;
sbit Cameras = P1 ^ 2;
sbit Alarm = P1 ^ 3;
sbit MainLights = P1 ^ 4;
sbit PetFeeder = P1 ^ 5;
sbit FrontGate = P1 ^ 6;
sbit Heater = P1 ^ 7;
// Port 2 - RGB LEDs and I2C
sbit AirCon_OFF = P2 ^ 0;  // OFF = RED
sbit AirCon_WAIT = P2 ^ 1; // Waiting for Clock = BLUE
sbit AirCon_ON = P2 ^ 2;   // ON = GREEN
sbit SDA = P2 ^ 3;         // I2C DATA
sbit Garage_OFF = P2 ^ 4;  // OFF = RED
sbit Garage_WAIT = P2 ^ 5; // Waiting for Clock = BLUE
sbit Garage_ON = P2 ^ 6;   // ON = RED
sbit SCL = P2 ^ 7;         // I2C CLK
sbit FAN = P0 ^ 7;         // ON = RED
/**************************************************************************************
Function Prototypes
***************************************************************************************/
#define CPH 20 // Characters pixel height for printing calaulations
void Interface_Routine(void);
void PrintInConsole(int type, int x, int y);
/**************************************************************************************
EXTERNS
***************************************************************************************/
extern S16 ButtonNum, x, y;
extern int RemoteSwitch;
extern U8 Console_buff[200];
extern int SumOfAppliances;
extern int Curr_Line_Index;
#endif