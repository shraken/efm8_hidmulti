//-----------------------------------------------------------------------------
// F3xx_USBtoUART.h
//-----------------------------------------------------------------------------
// Copyright 2008 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// Program Description:
//
// Stub file for Firmware Template.
//
//
// How To Test:    See Readme.txt
//
//
// Target:         C8051F32x, C8051F34x, C8061T622/3 or 'T326/7
// Tool chain:     Keil / Raisonance
//                 Silicon Laboratories IDE version 3.4x
// Command Line:   See Readme.txt
// Project Name:   HIDtoUART
//
// Release 1.1
//    -11 SEP 2008 (TP)
//    -Updated for 'T622/3
//
// Release 1.0
//    -Initial Revision (PD)
//    -04 JUN 2008
//

#ifndef  _USBTOUART_H_
#define  _USBTOUART_H_

#define IN_DATA  0x01
#define IN_DATA_SIZE 63
#define OUT_DATA 0x02
#define OUT_DATA_SIZE 60

void System_Init (void);
void Usb_Init (void);

extern unsigned char TX_Ready;

extern unsigned char xdata IN_PACKET[];
extern unsigned char xdata OUT_PACKET[];

extern unsigned char USB_OUT_SUSPENDED;

#endif  /* _USB_DESC_H_ */

//-----------------------------------------------------------------------------
// End of File
//-----------------------------------------------------------------------------
