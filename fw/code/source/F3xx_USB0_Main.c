//-----------------------------------------------------------------------------
// F3xx_USB_Main.c
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
// Target:         C8051F32x, C8051F34x, C8051T622/3 or 'T326/7
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
//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------

#include "c8051f3xx.h"
#include "F3xx_USB0_Register.h"
#include "F3xx_USB0_InterruptServiceRoutine.h"
#include "F3xx_USB0_Descriptor.h"
#include "F3xx_HIDtoUART.h"

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Local Variables
//-----------------------------------------------------------------------------

extern void delay_usec(unsigned int usec);
extern void delay_msec(unsigned int msec);

//-----------------------------------------------------------------------------
// Main Routine
//-----------------------------------------------------------------------------
void main(void)
{
   PCA0MD &= ~0x40;
   System_Init ();
   Usb_Init ();

   EA = 1;

   while (1)
   {
			//delay_usec(10000);
			//delay_msec(10);
		 
			if (!SendPacketBusy) {
					SendPacket (IN_DATA);
			}
			
			if (!SendPacketBusy2) {
					SendPacket2 (IN_DATA2);
			}
	 }
}

//-----------------------------------------------------------------------------
// End of File
//-----------------------------------------------------------------------------
