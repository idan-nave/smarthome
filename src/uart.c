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
BUFFERS DECLERATION
***************************************************************************************/
U8 Tx_buff[150]; // buffer to restore strings for transmitting
U8 Rx_buff[75];  // buffer to restore strings for receiving
/**************************************************************************************
char UART_RxChar()
***************************************************************************************
* I/P Arguments: none.
* Return value : char: Ascii value of the character received
* description :This function is used to receive a char from UART module.
It waits till a char is received and returns it after reception.
***************************************************************************************/
U8 UART_RxChar()
{
    while (!RI0 && !TF5H)
        ;           // Wait till the data is received
    RI0 = 0;        // Clear Receive Interrupt Flag for next cycle
    return (SBUF0); // return the received char
}
/**************************************************************************************
void UART_TxChar(char var_uartData_u8)
***************************************************************************************
* I/P Arguments: char-. Ascii value of the character to be transmitted.
* Return value : none.
* description :This function is used to transmit a char through UART module.
***************************************************************************************/
void UART_TxChar(U8 var_uartData_u8)
{
    SBUF0 = var_uartData_u8; // Load the data to be transmitted
    while (!TI0)
        ;    // Wait till the data is trasmitted
    TI0 = 0; // Clear the Tx flag for next cycle.
}
/**************************************************************************************
void UART_TxString()
***************************************************************************************

* I/P Arguments: uses TxBuff to send bytes from
* Return value : none
* description :This function is used to transmit a NULL terminated string through
UART.
1.The ptr_stringPointer_u8 points to the first char of the string
and traverses till the end(NULL CHAR) and transmits a char each
time
**************************************************************************************/

void UART_TxString()
{
    U8 *ptrTx = Tx_buff;
    while (*ptrTx)
        UART_TxChar(*ptrTx++); // Loop through the string and transmit char by char
    memset(Tx_buff, 0, strlen(Tx_buff) + 1);
}
/**************************************************************************************
void UART_RxString()
***************************************************************************************

* I/P Arguments: uses RxBuff to store received bytes
* Return value : none
* description :
1.This function is used to receive a ASCII string through UART till the
carriage_return/New_line
2.The stream of data is copied to the buffer till carriage_return/New_line is
encountered.
3. Once the Carriage_return/New_Line is received the string will be null terminated.
***************************************************************************************/
int NewLineTolerance()
{
    switch (srv.cmd)
    {
    case AT:
        return 1;
    case RST:
        return 1;
    case CWMODE:
        return 1;
    case CIPMUX:
        return 1;
    case CIPSERVER:
        return 1;
    case CIPSEND:
        return 2;
    case CIPCLOSE:
        return 1;
    case AwaitingAck:
        return 1;
    }
    return 3;
}
void UART_RxString()
{
    int nl = 0;
    U8 *ptrRx = Rx_buff;
    memset(Rx_buff, 0, strlen(Rx_buff));
    TR5 = 0;
    TMR5 = 0;
    TF5H = 0;
    TR5 = 1; /* Timer Config */
    // ~200ms window to receive inputs- enough for receiving ~ 75 chars in the time
    window.
        // 115200bps/200ms/8bits=~75; 1 char = 8 bit
        while (nl < NewLineTolerance())
    {
        *(ptrRx++) = UART_RxChar();
        if (*ptrRx == '\n')
            nl++;
        if (TF5H)
            return;
    }
}