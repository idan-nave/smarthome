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
#include "stdlib.h"
/**************************************************************************************
BUFFERS DECLERATION
***************************************************************************************/
U8 Temp_Value_Str[10];
U8 Temp_State_Str[10];
U8 Hum_Value_Str[10];
U8 Hum_State_Str[10];
U8 Car_Value_Str[10];
U8 Car_State_Str[10];
U8 Pet_Value_Str[10];
U8 Pet_State_Str[10];
U8 Summary_buff[150]; // holds all sensors status summary
U16 TEMP1, TEMP2;
/**************************************************************************************
void Init_Sensors();
***************************************************************************************
* I/P Arguments: none.
* Return value : none.
* description : This function gather all sensors data to be avaulable at system
startup
***************************************************************************************/
void Init_Sensors(void)
{
    FAN = ~FAN;
    Cameras = ~Cameras;
    Alarm = ~Alarm;
    MainLights = ~MainLights;
    PetFeeder = ~PetFeeder;
    FrontGate = ~FrontGate;
    Heater = ~Heater;
    memset(Appliances, 0, strlen(Appliances) + 1);
    strcat(Appliances, (U8 *)"0 Appliances are ON.");
    // Read_Sensors();
}
/**************************************************************************************
void Read_DHT()
***************************************************************************************
* I/P Arguments: none.
* Return value : none.
* description : This function rearange Climate-Control data and pour it into strings
so it would be accessible later
***************************************************************************************/
void Read_DHT(void)
{
    int serial_bit = 0;
    // re-initialize all strings from previous use
    memset(Temp_Value_Str, 0, strlen(Temp_Value_Str) + 1);
    memset(Temp_State_Str, 0, strlen(Temp_State_Str) + 1);
    memset(Hum_Value_Str, 0, strlen(Hum_Value_Str) + 1);
    memset(Hum_State_Str, 0, strlen(Hum_State_Str) + 1);
    TEMP1 = 0, TEMP2 = 0;
    // the following process prepares sensor to send it's readings. See project documentation for further info.
    DHT_DATA = 1;   /* start as high pin */
    delay_ms(2000); /* sensor stabilizes */
    DHT_DATA = 0;   /* set to low pin */
    delay_ms(18);   /* wait for 18ms */
    DHT_DATA = 1;   /* set to high pin*/
    delay_us(20);   /* wait for 20-40us */
    while (DHT_DATA)
        ; /* DHT responds LOW for about 80us */
    while (!DHT_DATA)
        ; /* DHT responds HIGH indicates 'get ready' */
    while (serial_bit < 24)
    {
        while (!DHT_DATA)
            ;         /* wait for low bit of 54us to pass*/
        delay_us(24); /*if high pulse is greater than 24ms, else its logic HIGH */
        if (serial_bit < 8)
        {
            if (DHT_DATA == 1)                 /* if high pulse is greater than 30ms */
                TEMP1 = (TEMP1 << 1) | (0x01); /* then its logic HIGH */
            else
                TEMP1 = (TEMP1 << 1); /* otherwise its logic LOW */
        }
        if (serial_bit > 16)
        {
            if (DHT_DATA == 1)                 /* if high pulse is greater than 30ms */
                TEMP2 = (TEMP2 << 1) | (0x01); /* then its logic HIGH */
            else
                TEMP2 = (TEMP2 << 1); /* otherwise its logic LOW */
        }
        serial_bit++;
    }
    // evaluate data and prepare strings
    sprintf(Hum_Value_Str, "%u", TEMP1 >> 2); // Shifting is not the right solution - fix was
    yet to be found
        strcat(Hum_Value_Str, (U8 *)" %");
    if (TEMP1 < 30)
        strcpy(Hum_State_Str, (U8 *)"Dry");
    if (TEMP1 > 70)
        strcpy(Hum_State_Str, (U8 *)"Wet");
    else
        strcpy(Hum_State_Str, (U8 *)"Casual");
    sprintf(Temp_Value_Str, "%u `C", TEMP2 >> 2); // Shifting is not the right solution - fix
    was yet to be found if (TEMP2 < 20) strcpy(Temp_State_Str, (U8 *)"Cold");
    if (TEMP2 > 30)
        strcpy(Temp_State_Str, (U8 *)"Hot");
    else
        strcpy(Temp_State_Str, (U8 *)"Casual");
}

/**************************************************************************************
void Read_HC()
***************************************************************************************
* I/P Arguments: none.
* Return value : none.
* description : This function rearange Ultrasonic-sensor data and pour it into strings
so it would be accessible later. it is merged with pulsing the sensor
also
***************************************************************************************/
void Read_HC(void)
/*
TIMER2 value is equal to the time taken by the signal to go forward and comeback so we
need to take only half time.
Time required = TIMER2 VALUE/2
The speed of the ultrasonic pulse is the speed of sound which is 340.29 m/s or 34029
cm/s
Distance = speed * time = 34029 * (TIMER2) / 2 = 17015
At 11.0592 MHz, TIMER2 gets incremented for 1�s => 1*10^-6
Range = 17015 * TIMER2 X 10^-6 = TIMER2/58 cm.
but At 48 MHz, TIMER2 gets incremented for 0.25�s (faster by factor of 4)
So Range = TIMER2/58/4 = TIMER2/232 cm.
*/
{
    // re-initialize all strings from previous use
    memset(Car_Value_Str, 0, strlen(Car_Value_Str) + 1);
    memset(Car_State_Str, 0, strlen(Car_State_Str) + 1);
    memset(Pet_Value_Str, 0, strlen(Pet_Value_Str) + 1);
    memset(Pet_State_Str, 0, strlen(Pet_State_Str) + 1);
    TEMP1 = 0, TEMP2 = 0;
    // pet sensor
    TMR2 = 0;
    TF2H = 0;
    Trigger_Pet = 0; /* starting at zero for reference*/
    delay_us(10);    /* wait for 10uS */
    Trigger_Pet = 1; /* pull trigger pin HIGH */
    delay_us(20);    /* wait for at least 10uS */
    Trigger_Pet = 0; /* pull trigger pin LOW*/
    while (!Echo_Pet)
        ; /* Waiting for Echo */
    TR2 = 1;
    TF2H = 0; /* Timer Starts */
    while (Echo_Pet)
        ; /* Waiting for Echo goes LOW */
    TR2 = 0;
    TF2H = 0;           /* Stop the timer */
    TEMP1 = TMR2 / 232; // See description above
    sprintf(Pet_Value_Str, "%u Cm", TEMP1);
    // evaluate state of pet by it's range
    if (TEMP1 < 30)
        strcpy(Pet_State_Str, (U8 *)"Resting");
    else
        strcpy(Pet_State_Str, (U8 *)"Missing");
    // car sensor
    TMR2 = 0;
    TF2H = 0;
    Trigger_Car = 0; /* starting at zero for reference*/
    delay_us(10);    /* wait for 10uS */
    Trigger_Car = 1; /* pull trigger pin HIGH */
    delay_us(20);    /* wait for at least 10uS */
    Trigger_Pet = 0; /* pull trigger pin LOW*/
    while (!Echo_Car)
        ; /* Waiting for Echo */
    TR2 = 1;
    TF2H = 0; /* Timer Starts */
    while (Echo_Car)
        ; /* Waiting for Echo goes LOW */
    TR2 = 0;
    TF2H = 0;           /* Stop the timer */
    TEMP2 = TMR2 / 232; // See description above
    sprintf(Car_Value_Str, "%u Cm", TEMP2);
    // evaluate state of car by it's range
    if (TEMP2 < 30)
        strcpy(Car_State_Str, (U8 *)"Parking");
    else
        strcpy(Car_State_Str, (U8 *)"In Use");
}

/**************************************************************************************
void Read_Sensors()
***************************************************************************************
* I/P Arguments: none.
* Return value : none.
* description : This function merges all data from both sensors to a wide string,
so it will be ready to be sent on any demand.
***************************************************************************************/
void Read_Sensors() /* prepare data array to be called*/
{
    Read_DHT();
    // Read_HC();
    memset(Summary_buff, 0, strlen(Summary_buff) + 1); // re-initialize from previous use
    strcat(Summary_buff, (U8 *)"SmartHome Report:");
    strcat(Summary_buff, (U8 *)"\nTime is ");
    strcat(Summary_buff, &Real.time);
    strcat(Summary_buff, (U8 *)"\nAirCon is set to [");
    strcat(Summary_buff, &AirCon.time);
    strcat(Summary_buff, (U8 *)"]\nGarage is set to [");
    strcat(Summary_buff, &Garage.time);
    strcat(Summary_buff, (U8 *)"]\nHumidity=");
    strcat(Summary_buff, Hum_Value_Str);
    strcat(Summary_buff, (U8 *)".\nTemperature=");
    strcat(Summary_buff, Temp_Value_Str);
    strcat(Summary_buff, (U8 *)".\nCar is ");
    strcat(Summary_buff, Car_State_Str);
    strcat(Summary_buff, (U8 *)".\nPet is ");
    strcat(Summary_buff, Pet_State_Str);
    strcat(Summary_buff, (U8 *)".\n");
    strcat(Summary_buff, Appliances);
    strcat(Summary_buff, (U8 *)"\r\n");
}