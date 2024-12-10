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
*************
BUFFERS DECLERATION
***************************************************************************************
************/
U8 Console_buff[200]; // holds data to be printed only by consile
/**************************************************************************************
*************
VARIEBLES DECLERATION
***************************************************************************************
************/
S16 x = 0, y = 0; // touch screen cordinations
S16 ButtonNum;    // number of touched button
int RemoteSwitch;
int SumOfAppliances = 0;
int Curr_Line_Index = 0;
/**************************************************************************************
*************
void Interface_Routine(void)
***************************************************************************************
************
* I/P Arguments: none.
* Return value : none
* description : 2nd most important function after main. the program is set here most
of time and
awaits requests in the form of buttons pressed. accordingly- calls for
reloading
a requested new page/screen or for any wifi/local function to be done.
***************************************************************************************
************/
void Interface_Routine(void)
{
    if (ScreenReady || RemoteSwitch)
    {
        delay_ms(50);
        if (!RemoteSwitch)
        {
            x = ReadTouchX();
            y = ReadTouchY();
            ButtonNum = ButtonTouch(x, y); // get currently touched position
        }
        // switch handles button numbers and respondings
        switch (ButtonNum)
        {
        case 0:
            MenuScreen();
            break;
        case 1:
            srv.cmd = AT;
            break;
        case 2:
            srv.cmd = CIPCLOSE;
            break;
        case 3:
            ClientsScreen();
            break;
        case 4:
            SensorsScreen();
            break;
        case 5:
            AppliancesScreen();
            break;
        case 6:
        {
            AirCon_OFF = ~AirCon_OFF;
            AirCon_ON = ~AirCon_ON;
            AirCon_WAIT = ~AirCon_WAIT;
            FAN = ~FAN;
        }
        break;
        case 7:
            Cameras = ~Cameras;
            break;
        case 8:
            Alarm = ~Alarm;
            break;
        case 9:
            MainLights = ~MainLights;
            break;
        case 10:
        {
            Garage_OFF = ~Garage_OFF;
            Garage_ON = ~Garage_ON;
            Garage_WAIT = ~Garage_WAIT;
        }
        break;
        case 11:
            PetFeeder = ~PetFeeder;
            break;
        case 12:
            FrontGate = ~FrontGate;
            break;
        case 13:
            Heater = ~Heater;
            break;
        case 14:
        {
            if (AirCon.st == OFF)
            {
                memset(&AirCon.time, 0, strlen(&AirCon.time) + 1);
                RTC_GetTime(&AirCon);
                AirCon.st = ON;
            }
            else
            {
                memset(&AirCon.time, 0, strlen(&AirCon.time) + 1);
                strcpy(&AirCon.time,
                       (U8 *)" Off ");
                AirCon.st = OFF;
            }
            AirCon_WAIT = ~AirCon_WAIT;
            SensorsScreen();
        }
        break;
        case 15:
        {
            if (Garage.st == OFF)
            {
                memset(&Garage.time, 0, strlen(&Garage.time) + 1);
                RTC_GetTime(&Garage);
                Garage.st = ON;
            }
            else
            {
                memset(&Garage.time, 0, strlen(&Garage.time) + 1);
                strcpy(&Garage.time,
                       (U8 *)" Off ");
                Garage.st = OFF;
            }
            Garage_WAIT = ~Garage_WAIT;
            SensorsScreen();
        }
        break;
        case 16:
        {
            Read_Sensors();
            LCD_printCenter(0, 220, (U8 *)" Posting data to server ", 3, WHITE, BLACK);
            delay_ms(1500);
            MenuScreen();
            strcpy(Console_buff, Summary_buff);
            PrintInConsole(3, 0, 50);
            id0.rqst = GetSummary;
            id1.rqst = GetSummary;
            id2.rqst = GetSummary;
            id3.rqst = GetSummary;
            if (No_Connected_Clients)
            {
                strcpy(Console_buff, (U8 *)"No Connected Users");
                PrintInConsole(2, 0, 200);
            }
            else
                srv.cmd = CIPSEND;
        }
        }
        if (ButtonNum > 5 && ButtonNum < 14) // if button is pressed change it's color.
        {
            if (ButtonColors[ButtonNum] == RED)
            {
                ButtonColors[ButtonNum] = GREEN;
                SumOfAppliances++;
            }
            else if (ButtonColors[ButtonNum] == GREEN)
            {
                ButtonColors[ButtonNum] = RED;
                SumOfAppliances--;
            }
            AppliancesScreen();
        }
        if (ButtonNum > 13 && ButtonNum < 16) // if button is pressed change it's color.
        {
            if (ButtonColors[ButtonNum] == RED)
            {
                ButtonColors[ButtonNum] = GREEN;
            }
            else if (ButtonColors[ButtonNum] == GREEN)
            {
                ButtonColors[ButtonNum] = RED;
            }
            SensorsScreen();
        }
    }
    RemoteSwitch = 0;
}
/**************************************************************************************
void PrintInConsole(int type, int x, int y, int print_len)
***************************************************************************************
* I/P Arguments: int type - used to decide the paint of the printed message
* Return value : none
* description : function is ment for smart-printing for not running over other
symbolics
x=starting point fro curser; y=always 0 unless the caller wish to enforce y position
***************************************************************************************/
void PrintInConsole(int type, int x, int y)
{
    switch (type)
    {
    case 1:
        LCD_setText2Color(GREEN, BLACK);
        break; // GOOD / OK
    case 2:
        LCD_setText2Color(RED, BLACK);
        break; // BAD / ERROR
    case 3:
        LCD_setText2Color(WHITE, BLACK);
        break; // NEUTRAL
    case 4:
        LCD_setText2Color(YELLOW, BLACK);
        break; // SMS/DATA
    }
    LCD_setCursor(x, (y) ? y : ((Curr_Line_Index > 15) ? 2 * CPH : 2 * CPH + Curr_Line_Index * CPH));
    printf("\n%s", Console_buff);
    Curr_Line_Index++;
    memset(Console_buff, 0, strlen(Console_buff) + 1);
}