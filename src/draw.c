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
U8 Appliances[50]; // holds Appliances data
// an array storing colors for most of buttons (index equals number id)
U16 ButtonColors[17] =
    {WHITE, GREEN, RED, BLUE, YELLOW, GREEN, RED, RED, RED, RED, RED, RED, RED, RED, GREEN, GREEN, YELLOW};
U8 Title_tmp[50];
U8 Names_tmp[50];
/**************************************************************************************
*************
void drawPage(U8 *str)
***************************************************************************************
************
* I/P Arguments: U8 *str - represent a new page title at the top of it
* Return value : none.
* description :This function draws a basic page- background, title,
constant menu button at top left, all other functions draw upon that
scheme
***************************************************************************************
************/
void drawPage(U8 *str)
{
    memset(Title_tmp, 0, strlen(Title_tmp) + 1);
    strcat(Title_tmp, (U8 *)" ");
    strncpy(Title_tmp, &Real.time, 5); // display only hour and minute on page title hh:mm
    strcat(Title_tmp, (U8 *)" ");
    strcat(Title_tmp, str);
    LCD_fillScreen(BLACK);
    LCD_fillRect(0, 0, LCD.width, 30, BLUE);
    LCD_printCenter(0, 2, Title_tmp, 3, WHITE, BLUE);
    LCD_drawButton(0, 0, 0, 60, 40, 30, ButtonColors[0], BLACK, "Menu", 2);
    Curr_Line_Index = 0;
}
/**************************************************************************************
*************
void MenuScreen(void)
***************************************************************************************
************
* I/P Arguments: none.
* Return value : none.
* description :This function draws the main menu- column of buttons at
right side of the screen, and the console theme
***************************************************************************************
************/
void MenuScreen(void)
{
    drawPage("Smart Home");
    // Curr_Page_Right_Boundary=320-5;
    LCD_fillRect(320, 30, 5, 300, WHITE);
    LCD_printCenter(0, 35, " WIFI console messages: ", 2, BLACK, WHITE);
    // (Button, x, y, w, h, r, Color, textcolor, label, textsize)
    LCD_drawButton(1, 330, 20, 150, 55, 30, ButtonColors[1], BLACK, "Start Server", 2);
    LCD_drawButton(2, 330, 80, 150, 55, 30, ButtonColors[2], WHITE, "Kill Server", 2);
    LCD_drawButton(3, 330, 140, 150, 55, 30, ButtonColors[3], WHITE, "Show Clients", 2);
    LCD_drawButton(4, 330, 200, 150, 55, 30, ButtonColors[4], BLACK, "Sensors", 2);
    LCD_drawButton(5, 330, 260, 150, 55, 30, ButtonColors[5], BLACK, "Appliances", 2);
    LCD_setText2Color(WHITE, BLACK);
    LCD_setCursor(0, 50);
    LCD_setTextSize(2);
}
/**************************************************************************************
*************
void ClientsScreen(void)
***************************************************************************************
************
* I/P Arguments: none.
* Return value : none.
* description :This function draws the Clients screen with real time data
regarding connectivity status and log of last actions that were taken
***************************************************************************************
************/
void ClientsScreen(void)
{
    drawPage("Clients Status");
    // Curr_Page_Right_Boundary=480/4-5;
    LCD_fillRect(120, 30, 5, 300, WHITE); // draw lines
    LCD_fillRect(240, 30, 5, 300, WHITE);
    LCD_fillRect(360, 30, 5, 300, WHITE);
    memset(Names_tmp, 0, strlen(Names_tmp) + 1);
    strcat(Names_tmp, (U8 *)" ");
    strcat(Names_tmp, a.name);
    strcat(Names_tmp, (U8 *)" "); // build titles with clients names
    strcat(Names_tmp, b.name);
    strcat(Names_tmp, (U8 *)" ");
    strcat(Names_tmp, c.name);
    strcat(Names_tmp, (U8 *)" ");
    strcat(Names_tmp, d.name);
    LCD_printCenter(0, 35, Names_tmp, 3, BLACK, WHITE); // draw titles
    LCD_setTextSize(1);                                 // prepare small text to print data logs
    strcpy(Console_buff, a.log);
    PrintInConsole(3, 7, 70); // print data inside columns
    strcpy(Console_buff, b.log);
    PrintInConsole(3, 127, 70);
    strcpy(Console_buff, c.log);
    PrintInConsole(3, 247, 70);
    strcpy(Console_buff, d.log);
    PrintInConsole(3, 367, 70);
    // (Button, x, y, w, h, r, Color, textcolor, label, textsize)
    LCD_drawButton(16, 30, 260, 400, 55, 30, ButtonColors[16],BLACK,"Send SmartHome Data
    Summary",2);
    LCD_setText2Color(WHITE, BLACK);
    LCD_setCursor (0, 50);
    LCD_setTextSize(2);
}
/**************************************************************************************
*************
void AppliancesScreen(void)
***************************************************************************************
************
* I/P Arguments: none.
* Return value : none.
* description :This function draws the Appliances switches dynamic buttons screen
color of each button is configurable through the array
***************************************************************************************
************/
void AppliancesScreen(void)
{
    drawPage("Appliances");
    // (Button, x, y, w, h, r, Color, textcolor, label, textsize)
    LCD_drawButton(6, 10, 70, 100, 80, 30, ButtonColors[6], WHITE, "AirCon", 2);
    LCD_drawButton(7, 130, 70, 100, 80, 30, ButtonColors[7], WHITE, "Cameras", 2);
    LCD_drawButton(8, 250, 70, 100, 80, 30, ButtonColors[8], WHITE, "Alarm", 2);
    LCD_drawButton(9, 370, 70, 100, 80, 30, ButtonColors[9], WHITE, "MainLights", 2);
    LCD_drawButton(10, 10, 170, 100, 80, 30, ButtonColors[10], WHITE, "Garage", 2);
    LCD_drawButton(11, 130, 170, 100, 80, 30, ButtonColors[11], WHITE, "PetFeeder", 2);
    LCD_drawButton(12, 250, 170, 100, 80, 30, ButtonColors[12], WHITE, "FrontGate", 2);
    LCD_drawButton(13, 370, 170, 100, 80, 30, ButtonColors[13], WHITE, "Heater", 2);
    memset(Appliances, 0, strlen(Appliances) + 1);
    sprintf(Appliances, "%d Appliances are ON.", SumOfAppliances);
    // status will be shown at bottom of the screen
    LCD_printCenter(0, 280, Appliances, 2, WHITE, BLACK);
    LCD_setText2Color(WHITE, BLACK);
}
/**************************************************************************************
*************
void SensorsScreen(void)
***************************************************************************************
************
* I/P Arguments: none.
* Return value : none.
* description :This function draws the Sensors cubes screen- which are based on
buttons
but not reactive. 2 other usable buttons let refresh the data and send
it
to clients. every time this page is drawn data is refreshed.
***************************************************************************************
************/
void SensorsScreen(void)
{
    drawPage("Online Sensors");
    Read_Sensors();
    // (Button, x, y, w, h, r, Color, textcolor, label, textsize)
    LCD_drawButton(14, 5, 50, 155, 110, 30, ButtonColors[14], WHITE, "AirCon", 3);
    LCD_setCursor(28, 60);
    memset(Title_tmp, 0, strlen(Title_tmp) + 1);
    strncpy(Title_tmp & AirCon.time, 5);
    printf("%s", Title_tmp);
    LCD_drawButton(15, 5, 180, 155, 110, 30, ButtonColors[15], WHITE, "Garage", 3);
    LCD_setCursor(28, 190);
    memset(Title_tmp, 0, strlen(Title_tmp) + 1);
    strncpy(Title_tmp, Garage.time, 5);
    printf("%s", Title_tmp);
    LCD_drawButton(17, 330, 50, 155, 110, 30, YELLOW, WHITE, "Car St.", 3);
    LCD_setCursor(345, 60);
    printf("%s", Car_Value_Str);
    LCD_setCursor(330, 120);
    printf("%s", Car_State_Str);
    LCD_drawButton(18, 165, 50, 155, 110, 30, BRED, WHITE, "Pet St.", 3);
    LCD_setCursor(189, 60);
    printf("%s", Pet_Value_Str);
    LCD_setCursor(170, 120);
    printf("%s", Pet_State_Str);
    LCD_drawButton(19, 330, 180, 155, 110, 30, ORANGE, BLACK, "Humidity", 3);
    LCD_setCursor(365, 190);
    printf("%s", Hum_Value_Str);
    LCD_setCursor(360, 250);
    printf("%s", Hum_State_Str);
    LCD_drawButton(20, 165, 180, 155, 110, 30, CYAN, BLACK, "Temprture", 3);
    LCD_setCursor(200, 190);
    printf("%s", Temp_Value_Str);
    LCD_setCursor(200, 250);
    printf("%s", Temp_State_Str);
}