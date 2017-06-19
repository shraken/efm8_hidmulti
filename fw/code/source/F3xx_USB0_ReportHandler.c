//-----------------------------------------------------------------------------
// F3xx_USB0_ReportHandler.c
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
// Header files
//-----------------------------------------------------------------------------

#include "c8051f3xx.h"
#include "F3xx_USB0_ReportHandler.h"
#include "F3xx_USB0_InterruptServiceRoutine.h"
#include "F3xx_HIDtoUART.h"

//-----------------------------------------------------------------------------
// Local Function Prototypes
//-----------------------------------------------------------------------------

void IN_Data (void);
void OUT_Data (void);
void IN_Data2 (void);
void OUT_Data2 (void);

extern unsigned int timer0_count;

//-----------------------------------------------------------------------------
// Local Definitions
//-----------------------------------------------------------------------------

//*****************************************************************************
// Set Definitions to sizes corresponding to the number of reports
//*****************************************************************************

#define IN_VECTORTABLESize 1
#define OUT_VECTORTABLESize 1

#define IN2_VECTORTABLESize 1
#define OUT2_VECTORTABLESize 1

//-----------------------------------------------------------------------------
// Global Constant Declaration
//-----------------------------------------------------------------------------


//*****************************************************************************
// Link all Report Handler functions to corresponding Report IDs
//*****************************************************************************
const VectorTableEntry IN_VECTORTABLE[IN_VECTORTABLESize] =
{
   // FORMAT: Report ID, Report Handler
   IN_DATA, IN_Data,
};

//*****************************************************************************
// Link all Report Handler functions to corresponding Report IDs
//*****************************************************************************
const VectorTableEntry OUT_VECTORTABLE[OUT_VECTORTABLESize] =
{
   // FORMAT: Report ID, Report Handler
   OUT_DATA, OUT_Data,
};

const VectorTableEntry IN_VECTORTABLE2[IN2_VECTORTABLESize] =
{
   // FORMAT: Report ID, Report Handler
   IN_DATA2, IN_Data2,
};

//*****************************************************************************
// Link all Report Handler functions to corresponding Report IDs
//*****************************************************************************
const VectorTableEntry OUT_VECTORTABLE2[OUT2_VECTORTABLESize] =
{
   // FORMAT: Report ID, Report Handler
   OUT_DATA2, OUT_Data2,
};

//-----------------------------------------------------------------------------
// Global Variable Declaration
//-----------------------------------------------------------------------------

BufferStructure IN_BUFFER, OUT_BUFFER;
BufferStructure IN_BUFFER2, OUT_BUFFER2;

//-----------------------------------------------------------------------------
// Local Variable Declaration
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Local Functions
//-----------------------------------------------------------------------------

//*****************************************************************************
// Add all Report Handler routines here
//*****************************************************************************

//*****************************************************************************
// For Input Reports:
// Point IN_BUFFER.Ptr to the buffer containing the report
// Set IN_BUFFER.Length to the number of bytes that will be
// transmitted.
//
// REMINDER:
// Systems using more than one report must define Report IDs
// for each report.  These Report IDs must be included as the first
// byte of an IN report.
// Systems with Report IDs should set the FIRST byte of their buffer
// to the value for the Report ID
// AND
// must transmit Report Size + 1 to include the full report PLUS
// the Report ID.
//
//*****************************************************************************

//-----------------------------------------------------------------------------
// IN_Data
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Report handler moves data from UART buffer into IN_BUFFER.  These bytes
// will then be copied to the USB IN interrupt FIFO.
//
//-----------------------------------------------------------------------------
void IN_Data (void)
{
	 static unsigned char count = 0;
	 unsigned char i;
	
   IN_PACKET[0] = IN_DATA;
	
	 for (i = 1; i < IN_DATA_SIZE; i++) {
		 IN_PACKET[i] = count;
		 count++;
	 }
	
	 IN_BUFFER.Ptr = IN_PACKET;
	 IN_BUFFER.Length = IN_DATA_SIZE + 1;

}

void IN_Data2 (void)
{
	 static unsigned char count = 0;
	 unsigned char i;
	
   IN_PACKET2[0] = IN_DATA2;
	
	 for (i = 1; i < IN_DATA_SIZE2; i++) {
		 IN_PACKET2[i] = count;
		 count++;
	 }
	
	 IN_BUFFER2.Ptr = IN_PACKET2;
	 IN_BUFFER2.Length = IN_DATA_SIZE2 + 1;

}

//*****************************************************************************
// For Output Reports:
// Data contained in the buffer OUT_BUFFER.Ptr will not be
// preserved after the Report Handler exits.
// Any data that needs to be preserved should be copyed from
// the OUT_BUFFER.Ptr and into other user-defined memory.
//
//*****************************************************************************

//-----------------------------------------------------------------------------
// OUT_Data
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Report handler copies data received from USB host into the UART transmit
// buffer.
//
//-----------------------------------------------------------------------------
void OUT_Data (void)
{

}

void OUT_Data2 (void)
{

}

//-----------------------------------------------------------------------------
// Global Functions
//-----------------------------------------------------------------------------

//*****************************************************************************
// Configure Setup_OUT_BUFFER
//
// Reminder:
// This function should set OUT_BUFFER.Ptr so that it
// points to an array in data space big enough to store
// any output report.
// It should also set OUT_BUFFER.Length to the size of
// this buffer.
//
//*****************************************************************************

void Setup_OUT_BUFFER(void)
{
   OUT_BUFFER.Ptr = OUT_PACKET;
   OUT_BUFFER.Length = 63;
}

//-----------------------------------------------------------------------------
// Vector Routines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// ReportHandler_IN...
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - Report ID
//
// These functions match the Report ID passed as a parameter
// to an Input Report Handler.
// the ...FG function is called in the SendPacket foreground routine,
// while the ...ISR function is called inside the USB ISR.  A lock
// is set whenever one function is called to prevent a call from the
// other from disrupting the routine.
// However, this should never occur, as interrupts are disabled by SendPacket
// before USB operation begins.
//-----------------------------------------------------------------------------
void ReportHandler_IN_ISR(unsigned char R_ID)
{
   unsigned char index;

   index = 0;

   while(index <= IN_VECTORTABLESize)
   {
      // Check to see if Report ID passed into function
       // matches the Report ID for this entry in the Vector Table
      if(IN_VECTORTABLE[index].ReportID == R_ID)
      {
         IN_VECTORTABLE[index].hdlr();
         break;
      }

      // If Report IDs didn't match, increment the index pointer
      index++;
   }

}
void ReportHandler_IN_Foreground(unsigned char R_ID)
{
   unsigned char index;

   index = 0;

   while(index <= IN_VECTORTABLESize)
   {
      // Check to see if Report ID passed into function
      // matches the Report ID for this entry in the Vector Table
      if(IN_VECTORTABLE[index].ReportID == R_ID)
      {
         IN_VECTORTABLE[index].hdlr();
         break;
      }

      // If Report IDs didn't match, increment the index pointer
      index++;
   }

}

//-----------------------------------------------------------------------------
// ReportHandler_OUT
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// This function matches the Report ID passed as a parameter
// to an Output Report Handler.
//
//-----------------------------------------------------------------------------
void ReportHandler_OUT(unsigned char R_ID){

   unsigned char index;

   index = 0;

   while(index <= OUT_VECTORTABLESize)
   {
      // Check to see if Report ID passed into function
      // matches the Report ID for this entry in the Vector Table
      if(OUT_VECTORTABLE[index].ReportID == R_ID)
      {
         OUT_VECTORTABLE[index].hdlr();
         break;
      }

      // If Report IDs didn't match, increment the index pointer
      index++;
   }
}




void ReportHandler_IN_ISR2(unsigned char R_ID)
{
   unsigned char index;

   index = 0;

   while(index <= IN2_VECTORTABLESize)
   {
      // Check to see if Report ID passed into function
       // matches the Report ID for this entry in the Vector Table
      if(IN_VECTORTABLE2[index].ReportID == R_ID)
      {
         IN_VECTORTABLE2[index].hdlr();
         break;
      }

      // If Report IDs didn't match, increment the index pointer
      index++;
   }

}

void ReportHandler_IN_Foreground2(unsigned char R_ID)
{
   unsigned char index;

   index = 0;

   while(index <= IN2_VECTORTABLESize)
   {
      // Check to see if Report ID passed into function
      // matches the Report ID for this entry in the Vector Table
      if(IN_VECTORTABLE2[index].ReportID == R_ID)
      {
         IN_VECTORTABLE2[index].hdlr();
         break;
      }

      // If Report IDs didn't match, increment the index pointer
      index++;
   }

}

void ReportHandler_OUT2(unsigned char R_ID){

   unsigned char index;

   index = 0;

   while(index <= OUT2_VECTORTABLESize)
   {
      // Check to see if Report ID passed into function
      // matches the Report ID for this entry in the Vector Table
      if(OUT_VECTORTABLE2[index].ReportID == R_ID)
      {
         OUT_VECTORTABLE2[index].hdlr();
         break;
      }

      // If Report IDs didn't match, increment the index pointer
      index++;
   }
}

//-----------------------------------------------------------------------------
// End of File
//-----------------------------------------------------------------------------
