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
VARIEBLES DECLERATION
***************************************************************************************
************/
Server srv;
Client a, b, c, d;
Channel id0, id1, id2, id3;
/**************************************************************************************
*************
BUFFERS DECLERATION
***************************************************************************************
************/
U8 SMS_buff[75]; // buffer to restore strings for SMS by all clients
/**************************************************************************************
*************
void Init_Wifi(void)
***************************************************************************************
************
* I/P Arguments: none.
* Return value : none
* description :This function initiaizes basic states of the system
***************************************************************************************
************/
void Init_Wifi(void)
{
    // Init server
    srv.cmd = CIPCLOSE;
    srv.rsp = ShutDown;
    // srv.st = OFF;
    Server_OFF = ~Server_OFF;
    // Init Channels
    id0.st = OFF;
    id0.rqst = None;
    id0.id = 0;
    id1.st = OFF;
    id1.rqst = None;
    id1.id = 1;
    id2.st = OFF;
    id2.rqst = None;
    id2.id = 2;
    id3.st = OFF;
    id3.rqst = None;
    id3.id = 3;
    // Init Clients
    memset(a.name, 0, strlen(a.name) + 1);
    memset(a.pass, 0, strlen(a.pass) + 1);
    memset(a.log, 0, strlen(a.log) + 1);
    memset(b.name, 0, strlen(b.name) + 1);
    memset(b.pass, 0, strlen(b.pass) + 1);
    memset(b.log, 0, strlen(b.log) + 1);
    memset(c.name, 0, strlen(c.name) + 1);
    memset(c.pass, 0, strlen(c.pass) + 1);
    memset(c.log, 0, strlen(c.log) + 1);
    memset(d.name, 0, strlen(d.name) + 1);
    memset(d.pass, 0, strlen(d.pass) + 1);
    memset(d.log, 0, strlen(d.log) + 1);
    strcpy(a.name, (U8 *)a_name);
    strcpy(a.pass, (U8 *)a_pass);
    strcpy(a.log(U8 *) "Disconnected");
    strcpy(b.name, (U8 *)b_name);
    strcpy(b.pass, (U8 *)b_pass);
    strcpy(b.log(U8 *) "Disconnected");
    strcpy(c.name, (U8 *)c_name);
    strcpy(c.pass, (U8 *)c_pass);
    strcpy(c.log(U8 *) "Disconnected");
    strcpy(d.name, (U8 *)d_name);
    strcpy(d.pass, (U8 *)d_pass);
    strcpy(d.log(U8 *) "Disconnected");
}
/**************************************************************************************
void Server_Routine(void)
***************************************************************************************
* I/P Arguments: none.
* Return value : none
* description : Server Interrupt Service Routine uses UART to sends the AT Commands in
accordance
with the state machine's present state, then calls for response check
***************************************************************************************/
void Server_Routine(void)
{
    if (srv.rsp == None && srv.cmd == NOP && srv.curr_id == NULL)
        return;
    switch (srv.cmd)
    {
    case AT: // Test if system works correctly
    {
        MenuScreen();
        if (srv.st == ON)
        {
            strcpy(Console_buff, (U8 *)"Server is up");
            PrintInConsole(3, 0, 0);
            srv.cmd = NOP;
            return;
        }
        else
            strcpy(Tx_buff, (U8 *)"ATE0\r\n");
    }
    break;
    case RST: // Reset the module
    {
        strcpy(Tx_buff, (U8 *)"AT+RST\r\n");
    }
    break;
    case CWMODE: // Set WIFI Station Mode
    {
        strcpy(Tx_buff, (U8 *)"AT+CWMODE=3\r\n");
    }
    break;
    case CIPMUX: // Enable / disable multiplex mode (up to 4 conenctions)
    {
        strcpy(Tx_buff, (U8 *)"AT+CIPMUX=1\r\n");
    }
    break;
    case CIPSERVER: // Create a TCP server
    {
        strcpy(Tx_buff, (U8 *)"AT+CIPSERVER=1\r\n");
    }
    break;
    case CIPSEND: // Prepare ESP for transmiting data from server
    {
        if (id0.st == ON)
            srv.curr_id = &id0;
        else if (id1.st == ON)
            srv.curr_id = &id1 else if (id2.st == ON) srv.curr_id = &id2;
        else if (id3.st == ON)
            srv.curr_id = &id3 else if (id0.st == ON || id1.st == ON || id2.st == ON || id3.st == ON)
            {
                strcpy(Console_buff, (U8 *)"Done sending to\nall connected clients");
                PrintInConsole(1, 0, 0);
                srv.cmd = NOP;
                return;
            }
        if (srv.curr_id->rqst == GetSMS)
            sprintf(Tx_buff, "AT+CIPSEND=%d,%d\r\n", srv.curr_id->id, strlen(SMS_buff) + 1);
        else if (srv.curr_id->rqst == GetSummary)
            sprintf(Tx_buff, "AT+CIPSEND=%d,%d\r\n", srv.curr_id->id,
                    strlen(Summary_buff) + 1);
        // else {strcpy(Console_buff, (U8*)"\n\nNo Connected Users");
        PrintInConsole(2, 0, 0);
        return;
    }
    }
    break;
case CIPCLOSE: // ShutDown Server
{
    MenuScreen();
    if (srv.st == OFF)
    {
        strcpy(Console_buff, (U8 *)"Server is down");
        PrintInConsole(3, 0, 0);
        srv.cmd = NOP;
        return;
    }
    else
        strcpy(Tx_buff, (U8 *)"AT+CIPCLOSE=0\r\n");
}
break;
case AwaitingAck:
{
    if (srv.curr_id->rqst == GetSMS)
        strncpy(Tx_buff, SMS_buff, strlen(SMS_buff) + 1);
    else if (srv.curr_id->rqst == GetSummary)
        strncpy(Tx_buff, Summary_buff,
                strlen(Summary_buff) + 1);
}
break;
}
if (srv.cmd != NOP)
    UART_TxString();
Lisen_For_Events();
Reply_To_Response();
}
/**************************************************************************************
*************
Lisen_For_Events()
***************************************************************************************
*************
* I/P Arguments: none
* Return value : none
* description : This function uses UART_RxString() to capture immidiate response from
esp
acceppted responds: Summary, SMS, AirCon, Garage, Switch, Page, Connected,
Disconnected, Error, OK, None, RST
***************************************************************************************
************/
void Lisen_For_Events(void)
{
    UART_RxString();
    Handle_Switches();
    Add_Client();
    Remove_Client();
    Validate_Client();
    Handle_Summary();
    Handle_SMS();
    Handle_Server();
    Handle_AutoClocks();
    Admin_Requests();
    Client_Requests();
}
/**************************************************************************************
*************
void Reply_To_Response(void)
***************************************************************************************
************
* I/P Arguments: none.
* Return value : none
* description : function acts as the "next state" trigger, and decides if the
previous sent commands
were successfully received. either way all new states of the system
are determined.
***************************************************************************************
************/
void Reply_To_Response(void)
{
    switch (srv.cmd)
    {
    case AT:
        if (srv.rsp == OK)
        {
            strcpy(Console_buff, (U8 *)"Successfully initialized");
            PrintInConsole(1, 0, 0);
            srv.cmd = RST;
            Server_OFF = ~Server_OFF;
            Server_ON = ~Server_ON;
        }
        else
        {
            strcpy(Console_buff, (U8 *)"Unsuccessfully initialized");
            PrintInConsole(2, 0, 0);
            srv.cmd = NOP;
        }
        break;
    case RST:
        if (srv.rsp == OK)
        {
            strcpy(Console_buff, (U8 *)"Successfully resetted");
            PrintInConsole(1, 0, 0);
            srv.cmd = CWMODE;
            srv.st = OFF;
            // Server_OFF = ~Server_OFF;
            // Server_ON = ~Server_ON;
            delay_ms(500);
        }
        else
        {
            strcpy(Console_buff, (U8 *)"Unsuccessfully resetted");
            PrintInConsole(2, 0, 0);
            srv.cmd = NOP;
        }
        break;
    case CWMODE:
        if (srv.rsp == OK)
        {
            strcpy(Console_buff, (U8 *)"Configured as an AP");
            PrintInConsole(1, 0, 0);
            srv.cmd = CIPMUX;
        }
        else
        {
            strcpy(Console_buff, (U8 *)"Unsuccessfully Configured \nas an AP");
            PrintInConsole(2, 0, 0);
            srv.cmd = NOP;
        }
        break;
    case CIPMUX:
        if (srv.rsp == OK)
        {
            strcpy(Console_buff, (U8 *)"Multiple connections \nmode ready");
            PrintInConsole(1, 0, 0);
            srv.cmd = CIPSERVER;
        }
        else
        {
            strcpy(Console_buff, (U8 *)"Unsccessfully configured\nmultiple connection");
            PrintInConsole(2, 0, 0);
            srv.cmd = NOP;
        }
        break;
    case CIPSERVER:
        if (srv.rsp == OK)
        {
            strcpy(Console_buff, (U8 *)"TCP Connection ready.\nWaiting for clients.");
            PrintInConsole(1, 0, 0);
            srv.cmd = NOP;
            srv.st = ON;
            Server_OFF = ~Server_OFF;
            Server_ON = ~Server_ON;
        }
        else
        {
            strcpy(Console_buff, (U8 *)"TCP Connection Error");
            PrintInConsole(2, 0, 0);
            srv.cmd = NOP;
        }
        break;

    ///////////////end of connection details request by demand////////////////////
    case CIPSEND: // CIPSEND has 2 possibilities to be summoned:
        // 1. for general sensor summary transmitting.
        // 2. for SMS initiated by a client - echoedback to all clients
        // datasheet note: must transmit in 20-ms interval between each
        packet, and a maximum of 2048 bytes per packet.
                    // wait for at least one second before sending the next AT command.
                    if (srv.rsp == OK)
        {
            strcpy(Console_buff, (U8 *)"\nSending data to ");
            // strcat(Console_buff, (U8*)srv.curr_id->link->name);
            PrintInConsole(1, 0, 0);
            srv.cmd = AwaitingAck;
        }
        else
        {
            strcpy(Console_buff, (U8 *)"\nError Preparing Data to ");
            // strcat(Console_buff, (U8*)srv.curr_id->link->name);
            PrintInConsole(2, 0, 0);
            srv.curr_id = NULL;
            srv.cmd = NOP;
        }
        break;
    case AwaitingAck:
        if (srv.rsp == OK)
        {
            PrintInConsole(1, 0, 0);
            srv.curr_id->rqst = None;
            srv.cmd = CIPSEND;
        }
        else
        {
            strcpy(Console_buff, (U8 *)"\nError Sending Data to ");
            // strcat(Console_buff, (U8*)srv.curr_id->link->name);
            PrintInConsole(2, 0, 0);
            srv.cmd = NOP;
        }
        break;
    ///////////////end of Message from server////////////////////
    case CIPCLOSE:
    {
        if (srv.rsp == ShutDown)
        {
            strcpy(Console_buff, (U8 *)"Server was taken down\n");
            PrintInConsole(3, 0, 0);
            srv.cmd = NOP;
            srv.st = OFF;
            Server_OFF = ~Server_OFF;
            Server_ON = ~Server_ON;
        }
        else
        {
            strcpy(Console_buff, (U8 *)"Error. Server is still up\n");
            PrintInConsole(2, 0, 0);
            srv.cmd = NOP;
        }
        break;
    }
    }
}

/**************************************************************************************
int Add_Client(void)
***************************************************************************************
* I/P Arguments: id.
* Return value : none
* description :This function changes client status in accordance with real time
changes.
***************************************************************************************/

void Add_Client(void)
{
    if (strstr(Rx_buff, (U8 *)"0,CONNECT"))
    {
        srv.curr_id = &id0;
        id0.st = OFF;
        id0.rqst = Try;
    }
    else if (strstr(Rx_buff, (U8 *)"1,CONNECT"))
    {
        srv.curr_id = &id1;
        id1.st = OFF;
        id1.rqst = Try;
    }
    else if (strstr(Rx_buff, (U8 *)"2,CONNECT"))
    {
        srv.curr_id = &id2;
        id2.st = OFF;
        id2.rqst = Try;
    }
    else if (strstr(Rx_buff, (U8 *)"3,CONNECT"))
    {
        srv.curr_id = &id3;
        id3.st = OFF;
        id3.rqst = Try;
    }
    // if(srv.curr_id->rqst==Try)
    // {
    // strcat(Console_buff, (U8*)srv.curr_id->link->name);
    // strcpy(Console_buff, (U8*)" is trying to\nconnect to server.\nPassward is required\n ");
    // PrintInConsole(3,0,0);
    // srv.curr_id->link=NULL;
    // srv.curr_id->rqst=None;
    // }
}

/**************************************************************************************
void Remove_Client(int id)
***************************************************************************************
* I/P Arguments: id.
* Return value : none
* description :This function changes client status in accordance with real time
changes.
***************************************************************************************/
void Remove_Client(void)
{
    if (strstr(Rx_buff, (U8 *)"0,CLOSED"))
    {
        srv.curr_id = &id0;
        id0.st = OFF;
        id0.rqst = Disconnected;
    }
    else if (strstr(Rx_buff, (U8 *)"1,CLOSED"))
    {
        srv.curr_id = &id1;
        id1.st = OFF;
        id1.rqst = Disconnected;
    }
    else if (strstr(Rx_buff, (U8 *)"2,CLOSED"))
    {
        srv.curr_id = &id2;
        id2.st = OFF;
        id2.rqst = Disconnected;
    }
    else if (strstr(Rx_buff, (U8 *)"3,CLOSED"))
    {
        srv.curr_id = &id3;
        id3.st = OFF;
        id3.rqst = Disconnected;
    }
}
/**************************************************************************************
void Validate_Client(int id)
***************************************************************************************
* I/P Arguments: id.
* Return value : none
* description :This function changes client status in accordance with real time
changes.
***************************************************************************************/
void Validate_Client(void)
{
    if (strstr(Rx_buff, (U8 *)"+IPD,0") && strstr(Rx_buff, (U8 *)a_pass))
    {
        srv.curr_id = &id0;
        id0.st = ON;
        id0.rqst = Connected;
        id0.link = &a;
    }
    else if (strstr(Rx_buff, (U8 *)"+IPD,1") && strstr(Rx_buff, (U8 *)a_pass))
    {
        srv.curr_id = &id1;
        id1.st = ON;
        id1.rqst = Connected;
        id1.link = &a;
    }
    else if (strstr(Rx_buff, (U8 *)"+IPD,2") && strstr(Rx_buff, (U8 *)a_pass))
    {
        srv.curr_id = &id2;
        id2.st = ON;
        id2.rqst = Connected;
        id2.link = &a;
    }
    else if (strstr(Rx_buff, (U8 *)"+IPD,3") && strstr(Rx_buff, (U8 *)a_pass))
    {
        srv.curr_id = &id3;
        id3.st = ON;
        id3.rqst = Connected;
        id3.link = &a;
    }
    else if (strstr(Rx_buff, (U8 *)"+IPD,0") && strstr(Rx_buff, (U8 *)b_pass))
    {
        srv.curr_id = &id0;
        id0.st = ON;
        id0.rqst = Connected;
        id0.link = &b;
    }
    else if (strstr(Rx_buff, (U8 *)"+IPD,1") && strstr(Rx_buff, (U8 *)b_pass))
    {
        srv.curr_id = &id1;
        id1.st = ON;
        id1.rqst = Connected;
        id1.link = &b;
    }
    else if (strstr(Rx_buff, (U8 *)"+IPD,2") && strstr(Rx_buff, (U8 *)b_pass))
    {
        srv.curr_id = &id2;
        id2.st = ON;
        id2.rqst = Connected;
        id2.link = &b;
    }
    else if (strstr(Rx_buff, (U8 *)"+IPD,3") && strstr(Rx_buff, (U8 *)b_pass))
    {
        srv.curr_id = &id3;
        id3.st = ON;
        id3.rqst = Connected;
        id3.link = &b;
    }
    else if (strstr(Rx_buff, (U8 *)"+IPD,0") && strstr(Rx_buff, (U8 *)c_pass))
    {
        srv.curr_id = &id0;
        id0.st = ON;
        id0.rqst = Connected;
        id0.link = &c;
    }
    else if (strstr(Rx_buff, (U8 *)"+IPD,1") && strstr(Rx_buff, (U8 *)c_pass))
    {
        srv.curr_id = &id1;
        id1.st = ON;
        id1.rqst = Connected;
        id1.link = &c;
    }
    else if (strstr(Rx_buff, (U8 *)"+IPD,2") && strstr(Rx_buff, (U8 *)c_pass))
    {
        srv.curr_id = &id2;
        id2.st = ON;
        id2.rqst = Connected;
        id2.link = &c;
    }
    else if (strstr(Rx_buff, (U8 *)"+IPD,3") && strstr(Rx_buff, (U8 *)c_pass))
    {
        srv.curr_id = &id3;
        id3.st = ON;
        id3.rqst = Connected;
        id3.link = &c;
    }
    else if (strstr(Rx_buff, (U8 *)"+IPD,0") && strstr(Rx_buff, (U8 *)d_pass))
    {
        srv.curr_id = &id0;
        id0.st = ON;
        id0.rqst = Connected;
        id0.link = &d;
    }
    else if (strstr(Rx_buff, (U8 *)"+IPD,1") && strstr(Rx_buff, (U8 *)d_pass))
    {
        srv.curr_id = &id1;
        id1.st = ON;
        id1.rqst = Connected;
        id1.link = &d;
    }
    else if (strstr(Rx_buff, (U8 *)"+IPD,2") && strstr(Rx_buff, (U8 *)d_pass))
    {
        srv.curr_id = &id2;
        id2.st = ON;
        id2.rqst = Connected;
        id2.link = &d;
    }
    else if (strstr(Rx_buff, (U8 *)"+IPD,3") && strstr(Rx_buff, (U8 *)d_pass))
    {
        srv.curr_id = &id3;
        id3.st = ON;
        id3.rqst = Connected;
        id3.link = &d;
    }
}
/**************************************************************************************
void Admin_Requests(void)
***************************************************************************************
* I/P Arguments: id.
* Return value : none
* description :This function changes client status in accordance with real time
changes.
***************************************************************************************/
void Admin_Requests(void)
{
    if (No_Valid_Request)
        return;
    else if (Valid_Request_0 && strstr(Rx_buff, (U8 *)"Kill") && id0.link->isAdmin)
    {
        srv.curr_id = &id0;
        id0.rqst = KillServer;
    }
    else if (Valid_Request_1 && strstr(Rx_buff, (U8 *)"Kill") && id1.link->isAdmin)
    {
        srv.curr_id = &id1;
        id1.rqst = KillServer;
    }
    else if (Valid_Request_2 && strstr(Rx_buff, (U8 *)"Kill") && id2.link->isAdmin)
    {
        srv.curr_id = &id2;
        id2.rqst = KillServer;
    }
    else if (Valid_Request_3 && strstr(Rx_buff, (U8 *)"Kill") && id3.link->isAdmin)
    {
        srv.curr_id = &id3;
        id3.rqst = KillServer;
    }
}
/**************************************************************************************
void Handle_SMS(void)
***************************************************************************************
* I/P Arguments: client structure, an action made by client
* Return value : none
* description :This function logs all client actions with timestamps
***************************************************************************************/
void Handle_SMS(void)
{
    if (No_Valid_Request)
        return;
    else if (Valid_Request_0 && strstr(Rx_buff, (U8 *)"SMS"))
    {
        srv.curr_id = &id0;
        id0.rqst = SentSMS;
        id1.rqst = GetSMS;
        id2.rqst = GetSMS;
        id3.rqst = GetSMS;
    }
    else if (Valid_Request_1 && strstr(Rx_buff, (U8 *)"SMS"))
    {
        srv.curr_id = &id1;
        id1.rqst = SentSMS;
        id0.rqst = GetSMS;
        id2.rqst = GetSMS;
        id3.rqst = GetSMS;
    }
    else if (Valid_Request_2 && strstr(Rx_buff, (U8 *)"SMS"))
    {
        srv.curr_id = &id2;
        id2.rqst = SentSMS;
        id0.rqst = GetSMS;
        id1.rqst = GetSMS;
        id3.rqst = GetSMS;
    }
    else if (Valid_Request_3 && strstr(Rx_buff, (U8 *)"SMS"))
    {
        srv.curr_id = &id3;
        id3.rqst = SentSMS;
        id0.rqst = GetSMS;
        id1.rqst = GetSMS;
        id2.rqst = GetSMS;
    }
}
/**************************************************************************************
void Handle_Summary(void)
***************************************************************************************
* I/P Arguments: client structure, an action made by client
* Return value : none
* description :This function logs all client actions with timestamps
***************************************************************************************/
void Handle_Summary(void)
{
    if (No_Valid_Request)
        return;
    else if (Valid_Request_0 && strstr(Rx_buff, (U8 *)"Summary"))
    {
        srv.curr_id = &id0;
        id0.rqst = GetSummary;
    }
    else if (Valid_Request_1 && strstr(Rx_buff, (U8 *)"Summary"))
    {
        srv.curr_id = &id1;
        id1.rqst = GetSummary;
    }
    else if (Valid_Request_2 && strstr(Rx_buff, (U8 *)"Summary"))
    {
        srv.curr_id = &id2;
        id2.rqst = GetSummary;
    }
    else if (Valid_Request_3 && strstr(Rx_buff, (U8 *)"Summary"))
    {
        srv.curr_id = &id3;
        id3.rqst = GetSummary;
    }
}
/**************************************************************************************
void Handle_AutoClocks(void)
***************************************************************************************
* I/P Arguments: client structure, an action made by client
* Return value : none
* description :This function logs all client actions with timestamps
***************************************************************************************/
void Handle_AutoClocks(void)
{
    if (No_Valid_Request)
        return;
    else
        RemoteSwitch = 1;
    if (Valid_Request_0 && strstr(Rx_buff, (U8 *)"Set AirCon"))
    {
        srv.curr_id = &id0;
        id0.rqst = SetClock;
        RTC_SetTime(&AirCon, ON, Rx_buff[12], Rx_buff[14], 0x00);
    }
    else if (Valid_Request_1 && strstr(Rx_buff, (U8 *)"Set AirCon"))
    {
        srv.curr_id = &id1;
        id1.rqst = SetClock;
        RTC_SetTime(&AirCon, ON, Rx_buff[12], Rx_buff[14], 0x00);
    }
    else if (Valid_Request_2 && strstr(Rx_buff, (U8 *)"Set AirCon"))
    {
        srv.curr_id = &id2;
        id2.rqst = SetClock;
        RTC_SetTime(&AirCon, ON, Rx_buff[12], Rx_buff[14], 0x00);
    }
    else if (Valid_Request_3 && strstr(Rx_buff, (U8 *)"Set AirCon"))
    {
        srv.curr_id = &id3;
        id3.rqst = SetClock;
        RTC_SetTime(&AirCon, ON, Rx_buff[12], Rx_buff[14], 0x00);
    }
    else if (Valid_Request_0 && strstr(Rx_buff, (U8 *)"Set Garage"))
    {
        srv.curr_id = &id0;
        id0.rqst = SetClock;
        RTC_SetTime(&Garage, ON, Rx_buff[12], Rx_buff[14], 0x00);
    }
    else if (Valid_Request_1 && strstr(Rx_buff, (U8 *)"Set Garage"))
    {
        srv.curr_id = &id1;
        id1.rqst = SetClock;
        RTC_SetTime(&Garage, ON, Rx_buff[12], Rx_buff[14], 0x00);
    }
    else if (Valid_Request_2 && strstr(Rx_buff, (U8 *)"Set Garage"))
    {
        srv.curr_id = &id2;
        id2.rqst = SetClock;
        RTC_SetTime(&Garage, ON, Rx_buff[12], Rx_buff[14], 0x00);
    }
    else if (Valid_Request_3 && strstr(Rx_buff, (U8 *)"Set Garage"))
    {
        srv.curr_id = &id3;
        id3.rqst = SetClock;
        RTC_SetTime(&Garage, ON, Rx_buff[12], Rx_buff[14], 0x00);
    }
    // AutoClock setting
    //  if(strstr(Rx_buff, (U8*)"AirCon") || strstr(Rx_buff, (U8*)"air conditioner")||strstr(Rx_buff, (U8*)"AIRCON"))
    // {RTC_SetTime(AirCon, ON, 0x09,0x00,0x00); ButtonNum=4;
    // Update_Client_LOG(c, 3);
    // srv.rsp = AirCon;
    // return;
}
// if(strstr(Rx_buff, (U8*)"Garage") || strstr(Rx_buff, (U8*)"garage")||strstr(Rx_buff, (U8*)"GARAGE"))
// {RTC_SetTime(Garage, ON, 0x09,0x00,0x00); ButtonNum=4; Update_Client_LOG(c, 4); srv.rsp = Garage; return;}}

/**************************************************************************************
void Handle_Server(void)
***************************************************************************************
* I/P Arguments: client structure, an action made by client
* Return value : none
* description :This function logs all client actions with timestamps
***************************************************************************************/
void Handle_Server(void)
{
    if (strstr(Rx_buff, (U8 *)"OK") || strstr(Rx_buff, (U8 *)"ATE0"))
        srv.rsp = OK;
    else if (strstr(Rx_buff, (U8 *)"ready"))
        srv.st = OFF;
    else if (strstr(Rx_buff, (U8 *)"UNLINK"))
        srv.rsp = ShutDown;
    else if (strstr(Rx_buff, (U8 *)"ERROR") || strstr(Rx_buff, (U8 *)"not valid"))
        srv.rsp = Error;
    else if (strstr(Rx_buff, (U8 *)">"))
        srv.rsp = AwaitingAck;
}
/**************************************************************************************
void Handle_Switches(void)
***************************************************************************************
* I/P Arguments: client structure, an action made by client
* Return value : none
* description :This function logs all client actions with timestamps
***************************************************************************************/
void Handle_Switches(void)
{
    if (No_Valid_Request)
        return;
    else
        RemoteSwitch = 1;
    // LCD Displays
    if (strstr(Rx_buff, (U8 *)"Menu"))
        ButtonNum = 0;
    else if (strstr(Rx_buff, (U8 *)"Clients"))
        ButtonNum = 3;
    else if (strstr(Rx_buff, (U8 *)"Sensors"))
        ButtonNum = 4;
    else if (strstr(Rx_buff, (U8 *)"Appliances"))
        ButtonNum = 5;
    // Clocks Toggels
    else if (strstr(Rx_buff, (U8 *)"AirCon_s"))
        ButtonNum = 6;
    else if (strstr(Rx_buff, (U8 *)"Garage_s"))
        ButtonNum = 10;
    else if (strstr(Rx_buff, (U8 *)"AirCon_a"))
        ButtonNum = 14;
    else if (strstr(Rx_buff, (U8 *)"Garage_a"))
        ButtonNum = 15;
    // Appliances Toggels
    else if (strstr(Rx_buff, (U8 *)"Cameras"))
        ButtonNum = 7;
    else if (strstr(Rx_buff, (U8 *)"Alarm"))
        ButtonNum = 8;
    else if (strstr(Rx_buff, (U8 *)"MainLights"))
        ButtonNum = 9;
    else if (strstr(Rx_buff, (U8 *)"PetFeeder"))
        ButtonNum = 11;
    else if (strstr(Rx_buff, (U8 *)"FrontGate"))
        ButtonNum = 12;
    else if (strstr(Rx_buff, (U8 *)"Heater"))
        ButtonNum = 13;
}
/**************************************************************************************
void Client_Requests(void)
***************************************************************************************
* I/P Arguments: client structure, an action made by client
* Return value : none
* description :This function logs all client actions with timestamps
***************************************************************************************/

void Client_Requests(void)
{
    switch (srv.curr_id->rqst)
    {
    case Try:
    {
        MenuScreen();
        // strcat(Console_buff, (U8*)srv.curr_id->link->name);
        strcpy(Console_buff, (U8 *)" is trying to\nconnect to server.\nPassward is required\n");
        PrintInConsole(3, 0, 0);
        srv.curr_id->link = NULL;
        srv.curr_id->rqst = None;
    }
    break;
    case Disconnected:
    {
        MenuScreen();
        // strcat(Console_buff, (U8*)srv.curr_id->link->name);
        strcpy(Console_buff, (U8 *)" has disconnected\n");
        PrintInConsole(2, 0, 0);
        strcat(srv.curr_id->link->log, (U8 *)"\n[");
        strcat(srv.curr_id->link->log, &Real.time);
        strcat(srv.curr_id->link->log, (U8 *)"]\nDisconnected");
        srv.curr_id->link = NULL;
        srv.curr_id->rqst = None;
    }
    break;
    case Connected:
    {
        MenuScreen();
        // strcat(Console_buff, (U8*)srv.curr_id->link->name);
        strcpy(Console_buff, (U8 *)" has\nsuccessfully connected.\n");
        PrintInConsole(1, 0, 0);
        strcat(srv.curr_id->link->log, (U8 *)"\n[");
        strcat(srv.curr_id->link->log, &Real.time);
        strcat(srv.curr_id->link->log, (U8 *)"]\nConnected");
        srv.curr_id->link = NULL;
        srv.curr_id->rqst = None;
    }
    break;
    case SentSMS:
    {
        // MenuScreen(); memset(SMS_buff,0,strlen(SMS_buff)+1); strcpy(SMS_buff,Rx_buff);
        // srv.cmd = CIPSEND;
        // strcat(srv.curr_id->link->log, (U8*)"\n[");
        // // strcat(srv.curr_id->link->log, &Real.time);
        // strcat(srv.curr_id->link->log, (U8*)"]\n");
        // strcat(srv.curr_id->link->log, SMS_buff);
        // strcat(srv.curr_id->link->log, (U8*)"\n");
    }
    break;
    case GetSummary:
    {
        MenuScreen();
        Read_Sensors();
        srv.cmd = CIPSEND;
        strcpy(Console_buff, Summary_buff);
        PrintInConsole(3, 0, 50);
        strcat(srv.curr_id->link->log, (U8 *)"\n[");
        strcat(srv.curr_id->link->log, &Real.time);
        strcat(srv.curr_id->link->log, (U8 *)"]\nSummary Demand");
    }
    break;
    case SetClock:
    {
        SensorsScreen();
        // srv.cmd = CIPSEND;
        // strcpy(Console_buff, Summary_buff); PrintInConsole(3,0,50);
        strcat(srv.curr_id->link->log, (U8 *)"\n[");
        strcat(srv.curr_id->link->log, &Real.time);
        strcat(srv.curr_id->link->log, (U8 *)"]\nSet Clock");
        srv.curr_id->link = NULL;
        srv.curr_id->rqst = None;
    }
    break;
    case KillServer:
    {
        srv.cmd = CIPCLOSE;
        strcat(srv.curr_id->link->log, (U8 *)"\n[");
        strcat(srv.curr_id->link->log, &Real.time);
        strcat(srv.curr_id->link->log, (U8 *)"]\nRemote ShutDown");
        srv.curr_id->link = NULL;
        srv.curr_id->rqst = None;
    }
    break;
    }
}