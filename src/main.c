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
#include "micro.h"
/**************************************************************************************
void main(void)
***************************************************************************************
* I/P Arguments: none.
* Return value : none
* description : This main function is kept simple as shold be. only 2 main processes
are being
constantly monitoring the outside - Interface_Routine() - for LCD
display and
Clients_Routine() for PhoneApp requests.
also- outside loop all main system ingredients are initialized,
especially:
Init_Device() which handles all special registers, timers, uart, etc.
***************************************************************************************/
void main(void)
{
    // iniitialisation of microcontroller.c with Configuration Wizard 2:
    Init_Device();
    // Welcome Screen - also flag for unwanted reset
    LCD_printCenter(0, 140, (U8 *)"Welcome", 5, WHITE, BLACK);
    // iniitialisation of system functions:
    Init_Wifi();
    Init_Sensors();        //----------if sensors are disconneted this should becommented-- -- -- -- - //
   
    // iniitialisation of clock:
    Init_RTC();
    InitClocks();

    while (1)
    {
        // LCD display requests
        Interface_Routine();
        // Server requests and reactions
        Server_Routine();
        // Constant real time read and check AutoClock
        Clock_Routine();
    }
}