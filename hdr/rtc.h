#ifndef RTC_H
#define RTC_H
/**************************************************************************************
Function Prototypes
***************************************************************************************/
// Below values are fixed and should not be changed. Refer Ds1307 DataSheet for moreinfo * /
#define C_Ds1307ReadMode_U8 0xD1u          // DS1307 ID
#define C_Ds1307WriteMode_U8 0xD0u         // DS1307 ID
#define C_Ds1307ControlRegAddress_U8 0x07u // Address to access Ds1307 CONTROL register
#define C_Ds1307RealRegAddress_U8 0x00u    // Address to access Ds1307 RTC register
#define C_Ds1307AirConRegAddress_U8 0x0Au  // Address to access Ds1307 AirCon register
#define C_Ds1307GarageRegAddress_U8 0x0Du  // Address to access Ds1307 Garage
register typedef struct
{
    U8 time[10]; // time in string format
    unsigned char sec;
    unsigned char min;
    unsigned char hour;
    State st;
    U8 RegAddress;
} Clock;

void Init_RTC(void);
void InitClocks(void);
void RTC_SetTime(Clock *clk, State st, U8 h, U8 m, U8 s);
void RTC_GetTime(Clock *clk);
void Clock_Routine(void);
/**************************************************************************************
EXTERNS
***************************************************************************************/
extern Clock Real, AirCon, Garage;
#endif