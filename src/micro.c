#include "compiler_defs.h"
#include "C8051F380_defs.h"
#include "micro.h"
#include "lcd.h"
// Peripheral specific initialization functions,
// Called from the Init_Device() function
void PCA_Init(void)
{
    // Watchdog disable for proper continuity of system
    PCA0MD &= ~0x40;
    PCA0MD = 0x00;
}
void Timer_Init(void)
{
    // Timer 1 enable
    TCON = 0x40;
    // Timer 1 mod 8bit auto-reload for UART0 auto BAUD rate handling
    TMOD = 0x20;
    // sysclock as configured in Oscillator_Init
    CKCON = 0x08;
    // 115200 BAUD equivalent value to reload in hex (at 48MHz)
    TH1 = 0x30;
    // Timer 2 enable
    TMR2CN = 0x04;
    SFRPAGE = CONFIG_PAGE;
    // Timer 5 enable
    TMR5CN = 0x04;
    SFRPAGE = LEGACY_PAGE;
    // interrupt for timer 5
    EIE2 = 0x20;
}
void UART_Init(void)
{
    SCON0 = 0x30;
}
void Port_IO_Init(void)
{
    // I\O pins enable:
    XBR1 = 0x40;
    P0MDOUT = 0xBF;
    P1MDOUT = 0xFF;
    P2MDOUT = 0x77;
    // UART pins Rx & Tx enable:
    XBR0 = 0x01;
}
void Oscillator_Init(void)
{
    FLSCL = 0x90;  // set as system main clock
    CLKSEL = 0x03; // enable crossbar system clock output
    OSCICN = 0xC3; // enable internal High Frequency oscilator- 48MHz
}
void Voltage_Reference_Init(void)
{
    REF0CN = 0x08;
}
// Initialization function for device,
// Call Init_Device() from your main program
void Init_Device(void)
{
    // Here all above functions are called
    PCA_Init();
    Timer_Init();
    UART_Init();
    Port_IO_Init();
    Oscillator_Init();
    Voltage_Reference_Init();
    initSYS(); // lcd
}