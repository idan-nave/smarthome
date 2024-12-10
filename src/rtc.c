#include "compiler_defs.h"
#include "C8051F380_defs.h"
#include "lcd.h"
#include "stdio.h"
#include "string.h"
#include "draw.h"
#include "interface.h"
#include "sensors.h"
#include "uart.h"
#include "wifi.h"
#include "rtc.h"
#include "i2c.h"
/**************************************************************************************
VARIEBLES DECLERATION
***************************************************************************************/
Clock Real, AirCon, Garage;  // extern Clock Real;
unsigned char RefreshScreen; // once every 2 minuetes
/**************************************************************************************
void Init_RTC()
***************************************************************************************
* I/P Arguments: none.
* Return value : none
* description :This function is used to Initialize the Ds1307 RTC.
***************************************************************************************/
void Init_RTC(void)
{
    I2C_Start();                     // Start I2C communication
    I2C_Write(C_Ds1307WriteMode_U8); // Connect to DS1307 by sending its ID on I2c
    Bus
        I2C_Write(C_Ds1307ControlRegAddress_U8); // Select the Ds1307 ControlRegister to
    configure Ds1307
        I2C_Write(0x00); // Write 0x00 to Control register to disable
    SQW - Out
          I2C_Stop(); // Stop I2C communication after initializing
    DS1307
}
/**************************************************************************************
void InitClocks(void)
***************************************************************************************
* I/P Arguments: none.
* Return value : none.
* description :This function initializes clocks default values
***************************************************************************************/
void InitClocks(void)
{
    RTC_SetTime(&Real, ON, 0x07, 0x30, 0x00);   // 07:30:00 AM
    RTC_SetTime(&AirCon, ON, 0x07, 0x31, 0x00); // 09:00:00 AM
    RTC_SetTime(&Garage, ON, 0x09, 0x00, 0x00); // 09:00:00 AM
    RefreshScreen = Real.min + 2;
    Real.RegAddress = C_Ds1307RealRegAddress_U8;
    AirCon.RegAddress = C_Ds1307AirConRegAddress_U8;
    Garage.RegAddress = C_Ds1307GarageRegAddress_U8;
    AirCon_OFF = ~AirCon_OFF;
    // AirCon_WAIT =~AirCon_WAIT;
    // AirCon_ON =~AirCon_ON;
    Garage_OFF = ~Garage_OFF;
    // Garage_WAIT =~Garage_WAIT;
    // Garage_ON =~Garage_ON;
}

/**************************************************************************************
void RTC_SetTime(Clock clk, State st, U8 h, U8 m, U8 s)
***************************************************************************************
* I/P Arguments: U8,U8,U8-->hh,mm,ss to Initialize the time into DS1307.
* Return value : none
* description :This function is used to update the Time(hh,mm,ss) of Ds1307 RTC.
The new time is updated into the non volatile memory of Ds1307.
Note: The I/P arguments should of BCD,
like 0x12,0x39,0x26 for 12hr,39min and 26sec.
***************************************************************************************/
void RTC_SetTime(volatile Clock *clk, State st, U8 h, U8 m, U8 s)
{
    I2C_Start();                     // Start I2C communication
    I2C_Write(C_Ds1307WriteMode_U8); // connect to DS1307 by sending its ID on I2c Bus
    I2C_Write(clk->RegAddress);      // Select the clk RAM address
    I2C_Write(s);
    I2C_Write(m);
    I2C_Write(h); // Write sec,min,hour to RAM address
    I2C_Stop();   // Stop I2C communication after getting the Time
    clk->sec = s;
    clk->min = m;
    clk->hour = h;
    clk->st = st;
    memset(clk->time, 0, strlen(clk->time) + 1);
    sprintf(clk->time, "%2x:%2x:%2x", (U16)h, (U16)m, (U16)s);
    if (clk->sec < 0x0A)
        clk->time[6] = '0';
    if (clk->min < 0x0A)
        clk->time[3] = '0';
    if (clk->hour < 0x0A)
        clk->time[0] = '0';
}
/**************************************************************************************
void RTC_GetTime(Clock clk)
***************************************************************************************
* I/P Arguments: U8 *,U8 *,U8 *-->pointers to get the hh,mm,ss.
* Return value : none
* description :This function is used to get the Time(hh,mm,ss) from Ds1307 RTC.
Note: The time read from Ds1307 will be of BCD format,
like 0x12,0x39,0x26 for 12hr,39min and 26sec.
***************************************************************************************/
void RTC_GetTime(volatile Clock *clk)
{
    I2C_Start();                     // Start I2C communication
    I2C_Write(C_Ds1307WriteMode_U8); // connect to DS1307 by sending its ID on I2c Bus
    I2C_Write(clk->RegAddress);      // Select the clk RAM address
    I2C_Stop();                      // Stop I2C communication after selecting Sec Register
    I2C_Start();                     // Start I2C communication
    I2C_Write(C_Ds1307ReadMode_U8);  // connect to DS1307(Read mode) by sending its ID
    clk->sec = I2C_Read(1);          // read second and return Positive ACK
    clk->min = I2C_Read(1);          // read minute and return Positive ACK
    clk->hour = I2C_Read(0);         // read hour and return Negative/No ACK
    I2C_Stop();                      // Stop I2C communication after reading the Time
    memset(clk->time, 0, strlen(clk->time) + 1);
    sprintf(clk->time, "%2x:%2x:%2x", (U16)clk->hour, (U16)clk->min, (U16)clk->sec);
    if (clk->sec < 0x0A)
        clk->time[6] = '0';
    if (clk->min < 0x0A)
        clk->time[3] = '0';
    if (clk->hour < 0x0A)
        clk->time[0] = '0';
}
/**************************************************************************************
void Clock_Routine
***************************************************************************************
* I/P Arguments: none.
* Return value : none.
* description : Interrupt Service Routine that reads RealClock time from RTC memory
and updates display buffer, also checks for AutoClock function validity
***************************************************************************************/
void Clock_Routine(void)
{
    if (Real.min == RefreshScreen)
    {
        MenuScreen();
        RefreshScreen = Real.min + 3;
    }
    // keep reading real time from rtc
    RTC_GetTime(&Real);
    // check autoclock execution- only check "hh:mm" and that function is turned ON
    ('auto') if (Real.min == AirCon.min && Real.hour == AirCon.hour)
    {
        AirCon_OFF = ~AirCon_OFF;
        AirCon_ON = ~AirCon_ON;
        FAN = ~FAN;
    }
    if (Real.min == Garage.min && Real.hour == Garage.hour)
    {
        Garage_OFF = ~Garage_OFF;
        Garage_ON = ~Garage_ON;
    }
}