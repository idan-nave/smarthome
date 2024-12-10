#ifndef __WIFI_H__
#define __WIFI_H__
/**************************************************************************************
Function Prototypes
***************************************************************************************/
#define a_name "Idan"
#define a_pass "w123"
#define a_isAdmin 1
#define b_name "Ariel"
#define b_pass "x123"
#define b_isAdmin 1
#define c_name "Shay"
#define c_pass "y123"
#define c_isAdmin 0
#define d_name "Shahar"
#define d_pass "z123"
#define d_isAdmin 0
#define Valid_Request_0 strstr(Rx_buff, (U8 *)"+IPD,0") && id0.st == ON
#define Valid_Request_1 strstr(Rx_buff, (U8 *)"+IPD,1") && id1.st == ON
#define Valid_Request_2 strstr(Rx_buff, (U8 *)"+IPD,2") && id2.st == ON
#define Valid_Request_3 strstr(Rx_buff, (U8 *)"+IPD,3") && id3.st == ON
#define No_Valid_Request !(Valid_Request_0 || Valid_Request_1 || Valid_Request_2 || Valid_Request_3)
#define No_Connected_Clients !(id0.st == ON || id1.st == ON || id2.st == ON || id3.st == ON)

// ENUMs for state machine
typedef enum
{
    AT,
    RST,
    CWMODE,
    CIPMUX,
    CIPSERVER,
    CIPSEND,
    CIPCLOSE,
    NOP,
    AwaitingAck
} Command;

typedef enum
{
    OFF,
    ON
} State;

typedef enum
{
    SentSMS,
    GetSMS,
    GetSummary,
    SetClock,
    Try,
    Connected,
    Disconnected,
    Switch,
    Error,
    OK,
    None,
    KillServer,
    ShutDown
} Interrupt;

typedef struct
{
    int isAdmin;
    U8 name[10]; // client name
    U8 pass[10]; // client password to execute functions
    U8 log[100]; // client accumulating log of actions
} Client;

typedef struct
{
    int id;
    State st;
    Client *link; // linked client to the channel
    Interrupt rqst;
} Channel;

typedef struct
{
    State st;
    Channel *curr_id; // current active client that initiates requests
    Command cmd;      // present command
    Interrupt rsp;    // response from esp
} Server;

void Init_Wifi(void);
void Server_Routine(void);
void Lisen_For_Events(void);
void Reply_To_Response(void);
void Add_Client(void);
void Remove_Client(void);
void Validate_Client(void);
void Handle_SMS(void);
void Handle_Summary(void);
void Handle_Switches(void);
void Handle_Server(void);
void Handle_AutoClocks(void);
void Client_Requests(void);
void Admin_Requests(void);

/**************************************************************************************
EXTERNS
***************************************************************************************/
extern U8 SMS_buff[75];
extern Client a, b, c, d;
extern Server srv;
extern Channel id0, id1, id2, id3;
#endif