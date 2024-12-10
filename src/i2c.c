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
void I2C_Start()
***************************************************************************************
* I/P Arguments: none.
* Return value : none
* description :This function is used to generate I2C Start Condition.
Start Condition: SDA goes low when SCL is High.
***************************************************************************************/
void I2C_Start()
{
    SCL = 0; // Pull SCL low
    SDA = 1; // Pull SDA High
    delay_us(1);
    SCL = 1; // Pull SCL high
    delay_us(1);
    SDA = 0; // Now Pull SDA LOW, to generate the Start Condition
    delay_us(1);
    SCL = 0; // Finally Clear the SCL to complete the cycle
}
/**************************************************************************************
void I2C_Stop()
***************************************************************************************
* I/P Arguments: none.
* Return value : none
* description :This function is used to generate I2C Stop Condition.
Stop Condition: SDA goes High when SCL is High.
***************************************************************************************/
void I2C_Stop(void)
{
    SCL = 0; // Pull SCL low
    delay_us(1);
    SDA = 0; // Pull SDA low
    delay_us(1);
    SCL = 1; // Pull SCL High
    delay_us(1);
    SDA = 1; // Now Pull SDA High, to generate the Stop Condition
}
/**************************************************************************************
void I2C_Write(U8 var_i2cData_u8)
***************************************************************************************
* I/P Arguments: U8-->8bit data to be sent.
* Return value : none
* description :This function is used to send a byte on SDA line using I2C protocol
8bit data is sent bit-by-bit on each clock cycle.
MSB(bit) is sent first and LSB(bit) is sent at last.
Data is sent when SCL is low.
***************************************************************************************/
void I2C_Write(U8 var_i2cData_u8)
{
    U8 i;
    for (i = 0; i < 8; i++) // loop 8 times to send 1-byte of data
    {
        SDA = var_i2cData_u8 & 0x80;          // Send Bit by Bit on SDA line
        i2c_Clock();                          // Generate Clock at SCL
        var_i2cData_u8 = var_i2cData_u8 << 1; // Bring the next bit to be transmitted to MSB
        position
    }
    i2c_Clock();
}
/**************************************************************************************
U8 I2C_Read(U8 var_ackOption_u8)
***************************************************************************************
* I/P Arguments: none.
* Return value : U8(received byte)
* description :This fun is used to receive a byte on SDA line using I2C protocol.
8bit data is received bit-by-bit each clock and finally packed into
Byte.
MSB(bit) is received first and LSB(bit) is received at last.
***************************************************************************************/
U8 I2C_Read(U8 var_ackOption_u8)
{
    U8 i, var_i2cData_u8 = 0x00;
    SDA = 1;                // Make SDA as I/P
    for (i = 0; i < 8; i++) // loop 8times read 1-byte of data
    {
        delay_us(1);
        SCL = 1; // Pull SCL High
        delay_us(1);
        var_i2cData_u8 = var_i2cData_u8 << 1;  // var_i2cData_u8 is Shifted each time and
        var_i2cData_u8 = var_i2cData_u8 | SDA; // ORed with the received bit to pack into
        byte
            SCL = 0; // Clear SCL to complete the Clock
    }
    if (var_ackOption_u8 == 1) /*Send the Ack/NoAck depending on the user option*/
    {
        i2c_Ack();
    }
    else
    {
        i2c_NoAck();
    }
    return var_i2cData_u8; // Finally return the received Byte*
}
/**************************************************************************************
static void i2c_Clock()
***************************************************************************************
* I/P Arguments: none.
* Return value : none
* description :This function is used to generate a clock pulse on SCL line.
***************************************************************************************/
static void i2c_Clock(void)
{
    delay_us(1);
    SCL = 1;     // Wait for Some time and Pull the SCL line High
    delay_us(1); // Wait for Some time
    SCL = 0;     // Pull back the SCL line low to Generate a clock pulse
}
/**************************************************************************************
static void i2c_Ack()
***************************************************************************************
* I/P Arguments: none.
* Return value : none
* description :This function is used to generate a the Positive ACK
pulse on SDA after receiving a byte.
***************************************************************************************/
static void i2c_Ack()
{
    SDA = 0;     // Pull SDA low to indicate Positive ACK
    i2c_Clock(); // Generate the Clock
    SDA = 1;     // Pull SDA back to High(IDLE state)
}
/**************************************************************************************
static void i2c_NoAck()
***************************************************************************************
* I/P Arguments: none.
* Return value : none
* description :This function is used to generate a the Negative/NO ACK
pulse on SDA after receiving all bytes.
***************************************************************************************/
static void i2c_NoAck()
{
    SDA = 1;     // Pull SDA high to indicate Negative/NO ACK
    i2c_Clock(); // Generate the Clock
    SCL = 1;     // Set SCL
}