//-----------------------------------------------------------------------------
// F3xx_USB0_Standard_Requests.c
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
#include "F3xx_USB0_ReportHandler.h"

//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------

extern code device_descriptor DEVICEDESC;   // These are created in
                                            // F3xx_USB0_Descriptor.h
extern unsigned char* STRINGDESCTABLE[];

// Additional declarations for HID:
extern code hid_configuration_descriptor HIDCONFIGDESC;
extern code hid_report_descriptor HIDREPORTDESC;
extern code hid_report_descriptor HIDREPORTDESC2;

extern setup_buffer SETUP;             // Buffer for current device request
                                       // information
extern unsigned int DATASIZE;
extern unsigned int DATASENT;
extern unsigned char* DATAPTR;

// These are response packets used for
code unsigned char ONES_PACKET[2] = {0x01, 0x00};
// Communication with host
code unsigned char ZERO_PACKET[2] = {0x00, 0x00};

extern unsigned char USB0_STATE;       // Determines current usb device state

//-----------------------------------------------------------------------------
// Definitions
//-----------------------------------------------------------------------------
// Redefine existing variable names to refer to the descriptors within the
// HID configuration descriptor.
// This minimizes the impact on the existing source code.
#define ConfigDesc     (HIDCONFIGDESC.hid_configuration_descriptor)
#define InterfaceDesc  (HIDCONFIGDESC.hid_interface_descriptor)
#define HidDesc     (HIDCONFIGDESC.hid_descriptor)
#define Endpoint1Desc  (HIDCONFIGDESC.hid_endpoint_in_descriptor)
#define Endpoint2Desc  (HIDCONFIGDESC.hid_endpoint_out_descriptor)

//-----------------------------------------------------------------------------
// Get_Status
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change for custom HID designs.
//
//-----------------------------------------------------------------------------
void Get_Status (void)                 // This routine returns a two byte
{                                      // status packet to the host

   if (SETUP.wValue.U8[MSB] || SETUP.wValue.U8[LSB] ||
                                       // If non-zero return length or data
                                       // length not
   SETUP.wLength.U8[MSB]    || (SETUP.wLength.U8[LSB] != 2))
                                       // Equal to 2 then send a stall
   {                                   // indicating invalid request
      Force_Stall ();
   }

   switch(SETUP.bmRequestType)         // Determine if recipient was device,
   {                         // interface, or EP
      case OUT_DEVICE:                 // If recipient was device
         if (SETUP.wIndex.U8[MSB] || SETUP.wIndex.U8[LSB])
         {
            Force_Stall ();            // Send stall if request is invalid
         }
         else
         {
        // Otherwise send 0x00, indicating bus power and no
        // remote wake-up supported
            DATAPTR = (unsigned char*)&ZERO_PACKET;
            DATASIZE = 2;
         }
         break;

      case OUT_INTERFACE:              // See if recipient was interface
         if ((USB0_STATE != DEV_CONFIGURED) ||
         SETUP.wIndex.U8[MSB] || SETUP.wIndex.U8[LSB])
                                       // Only valid if device is configured
                                       // and non-zero index
         {
            Force_Stall ();            // Otherwise send stall to host
         }
         else
         {
            // Status packet always returns 0x00
            DATAPTR = (unsigned char*)&ZERO_PACKET;
            DATASIZE = 2;
         }
         break;

      case OUT_ENDPOINT:               // See if recipient was an endpoint
         if ((USB0_STATE != DEV_CONFIGURED) ||
         SETUP.wIndex.U8[MSB])         // Make sure device is configured
                                       // and index msb = 0x00
         {                             // otherwise return stall to host
            Force_Stall();
         }
         else
         {
         // Handle case if request is directed to EP 1
            if (SETUP.wIndex.U8[LSB] == IN_EP1)
            {
               if (EP_STATUS[1] == EP_HALT)
               {                       // If endpoint is halted,
                                       // return 0x01,0x00
                  DATAPTR = (unsigned char*)&ONES_PACKET;
                  DATASIZE = 2;
               }
               else
               {
                  // Otherwise return 0x00,0x00 to indicate endpoint active
                  DATAPTR = (unsigned char*)&ZERO_PACKET;
                  DATASIZE = 2;
               }
            }
            else
            {
               Force_Stall ();         // Send stall if unexpected data
                                       // encountered
            }
         }
         break;

      default:
         Force_Stall ();
         break;
   }
   if (EP_STATUS[0] != EP_STALL)
   {
     // Set serviced SETUP Packet, Endpoint 0 in transmit mode, and
     // reset DATASENT counter
      POLL_WRITE_BYTE (E0CSR, rbSOPRDY);
      EP_STATUS[0] = EP_TX;
      DATASENT = 0;
   }
}

//-----------------------------------------------------------------------------
// Clear_Feature
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change in custom HID designs.
//
//-----------------------------------------------------------------------------
void Clear_Feature ()                  // This routine can clear Halt Endpoint
{                                      // features on endpoint 1

   // Send procedural stall if device isn't configured
   if ( (USB0_STATE != DEV_CONFIGURED) ||
   // Or request is made to host(remote wakeup not supported)
   (SETUP.bmRequestType == IN_DEVICE) ||
   // Or request is made to interface
   (SETUP.bmRequestType == IN_INTERFACE) ||
   // Or msbs of value or index set to non-zero value
   SETUP.wValue.U8[MSB]  || SETUP.wIndex.U8[MSB] ||
   // Or data length set to non-zero.
   SETUP.wLength.U8[MSB] || SETUP.wLength.U8[LSB])
   {
      Force_Stall ();
   }

   else
   {
      // Verify that packet was directed at an endpoint
      if ( (SETUP.bmRequestType == IN_ENDPOINT)&&
      // The feature selected was HALT_ENDPOINT
      (SETUP.wValue.U8[LSB] == ENDPOINT_HALT)  &&
      // And that the request was directed at EP 1 in
      ((SETUP.wIndex.U8[LSB] == IN_EP1) ) )
      {
         if (SETUP.wIndex.U8[LSB] == IN_EP1)
         {
            POLL_WRITE_BYTE (INDEX, 1);// Clear feature endpoint 1 halt
            POLL_WRITE_BYTE (EINCSR1, rbInCLRDT);
            EP_STATUS[1] = EP_IDLE;    // Set endpoint 1 status back to idle
         }
      }
      else
      {
         Force_Stall ();               // Send procedural stall
      }
   }
   POLL_WRITE_BYTE (INDEX, 0);         // Reset Index to 0
   if (EP_STATUS[0] != EP_STALL)
   {
      POLL_WRITE_BYTE (E0CSR, (rbSOPRDY | rbDATAEND));
                                       // Set Serviced Out packet ready and
                                       // data end to indicate transaction
                                       // is over
   }
}

//-----------------------------------------------------------------------------
// Set_Feature
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change in custom HID designs.
//
//-----------------------------------------------------------------------------
void Set_Feature (void)                // This routine will set the EP Halt
{                                      // feature for endpoint 1

   // Make sure device is configured, SETUP data
   if ((USB0_STATE != DEV_CONFIGURED) ||
   // is all valid and that request is directed at an endpoint
   (SETUP.bmRequestType == IN_DEVICE) ||
   (SETUP.bmRequestType == IN_INTERFACE) ||
   SETUP.wValue.U8[MSB]  || SETUP.wIndex.U8[MSB] ||
   SETUP.wLength.U8[MSB] || SETUP.wLength.U8[LSB])
   {
      Force_Stall ();                  // Otherwise send stall to host
   }

   else
   {
      // Make sure endpoint exists and that halt
      if ( (SETUP.bmRequestType == IN_ENDPOINT)&&
      // endpoint feature is selected
      (SETUP.wValue.U8[LSB] == ENDPOINT_HALT) &&
      ((SETUP.wIndex.U8[LSB] == IN_EP1)        ||
      (SETUP.wIndex.U8[LSB] == OUT_EP1) ) )
      {
         if (SETUP.wIndex.U8[LSB] == IN_EP1)
         {
            POLL_WRITE_BYTE (INDEX, 1);// Set feature endpoint 1 halt
            POLL_WRITE_BYTE (EINCSR1, rbInSDSTL);
            EP_STATUS[1] = EP_HALT;
         }
      }
      else
      {
         Force_Stall ();               // Send procedural stall
      }
   }
   POLL_WRITE_BYTE (INDEX, 0);
   if (EP_STATUS[0] != EP_STALL)
   {
      POLL_WRITE_BYTE (E0CSR, (rbSOPRDY | rbDATAEND));
                                       // Indicate SETUP packet has been
                                       // serviced
   }
}

//-----------------------------------------------------------------------------
// Set_Address
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change in custom HID designs.
//
//-----------------------------------------------------------------------------
void Set_Address (void)                // Set new function address
{
   // Request must be directed to device
   if ((SETUP.bmRequestType != IN_DEVICE) ||
   // with index and length set to zero.
   SETUP.wIndex.U8[MSB]  || SETUP.wIndex.U8[LSB]||
   SETUP.wLength.U8[MSB] || SETUP.wLength.U8[LSB]||
   SETUP.wValue.U8[MSB]  || (SETUP.wValue.U8[LSB] & 0x80))
   {
     Force_Stall ();                   // Send stall if SETUP data invalid
   }

   EP_STATUS[0] = EP_ADDRESS;          // Set endpoint zero to update
                                       // address next status phase
   if (SETUP.wValue.U8[LSB] != 0)
   {
      USB0_STATE = DEV_ADDRESS;        // Indicate that device state is now
                                       // address
   }
   else
   {
      USB0_STATE = DEV_DEFAULT;        // If new address was 0x00, return
   }                                   // device to default state
   if (EP_STATUS[0] != EP_STALL)
   {
      POLL_WRITE_BYTE (E0CSR, (rbSOPRDY | rbDATAEND));
                                       // Indicate SETUP packet has
                                       // been serviced
   }
}

//-----------------------------------------------------------------------------
// Get_Descriptor
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change in custom HID designs.
//
//-----------------------------------------------------------------------------
void Get_Descriptor (void)             // This routine sets the data pointer
{                                      // and size to correct descriptor and
                                       // sets the endpoint status to transmit

   switch(SETUP.wValue.U8[MSB])        // Determine which type of descriptor
   {                                   // was requested, and set data ptr and
      case DSC_DEVICE:                 // size accordingly
         DATAPTR = (unsigned char*) &DEVICEDESC;
         DATASIZE = DEVICEDESC.bLength;
         break;

      case DSC_CONFIG:
         DATAPTR = (unsigned char*) &ConfigDesc;
                                       // Compiler Specific - The next statement
                                       // reverses the bytes in the configuration
                                       // descriptor for the compiler
         DATASIZE = ConfigDesc.wTotalLength.U8[MSB] +
                    256*ConfigDesc.wTotalLength.U8[LSB];
         break;

    case DSC_STRING:
         DATAPTR = STRINGDESCTABLE[SETUP.wValue.U8[LSB]];
                                       // Can have a maximum of 255 strings
         DATASIZE = *DATAPTR;
         break;

      case DSC_INTERFACE:
         DATAPTR = (unsigned char*) &InterfaceDesc;
         DATASIZE = InterfaceDesc.bLength;
         break;

      case DSC_ENDPOINT:
         if (SETUP.wValue.U8[LSB] == IN_EP1)
         {
            DATAPTR = (unsigned char*) &Endpoint1Desc;
            DATASIZE = Endpoint1Desc.bLength;
         }
         else if (SETUP.wValue.U8[LSB] == IN_EP2)
         {
            DATAPTR = (unsigned char*) &Endpoint2Desc;
            DATASIZE = Endpoint2Desc.bLength;
         }
         else
         {
            Force_Stall();
         }
         break;

   case DSC_HID:                       // HID Specific (HID class descriptor)
      DATAPTR = (unsigned char*)&HidDesc;
      DATASIZE = HidDesc.bLength;
      break;

   case DSC_HID_REPORT:                // HID Specific (HID report descriptor)
		  /*
      DATAPTR = (unsigned char*)&HIDREPORTDESC;
      DATASIZE = HID_REPORT_DESCRIPTOR_SIZE;
      break;
			*/
	 
			if (SETUP.wIndex.U8[LSB] == 0x00) {
				DATAPTR = (unsigned char*)&HIDREPORTDESC;
				DATASIZE = HID_REPORT_DESCRIPTOR_SIZE;
				break;	
			} else if (SETUP.wIndex.U8[LSB] == 0x01) {
				DATAPTR = (unsigned char*)&HIDREPORTDESC2;
				DATASIZE = HID_REPORT_DESCRIPTOR_SIZE;
				break;
			}
	 
      default:
         Force_Stall ();               // Send Stall if unsupported request
         break;
   }

   // Verify that the requested descriptor is valid
   if (SETUP.wValue.U8[MSB] == DSC_DEVICE ||
   SETUP.wValue.U8[MSB] == DSC_CONFIG     ||
   SETUP.wValue.U8[MSB] == DSC_STRING     ||
   SETUP.wValue.U8[MSB] == DSC_INTERFACE  ||
   SETUP.wValue.U8[MSB] == DSC_ENDPOINT)
   {
      if ((SETUP.wLength.U8[LSB] < DATASIZE) &&
      (SETUP.wLength.U8[MSB] == 0))
      {
         DATASIZE = SETUP.wLength.U16; // Send only requested amount of data
      }
   }
   if (EP_STATUS[0] != EP_STALL)       // Make sure endpoint not in stall mode
   {
     POLL_WRITE_BYTE (E0CSR, rbSOPRDY);// Service SETUP Packet
     EP_STATUS[0] = EP_TX;             // Put endpoint in transmit mode
     DATASENT = 0;                     // Reset Data Sent counter
   }
}

//-----------------------------------------------------------------------------
// Get_Configuration
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change in custom HID designs.
//
//-----------------------------------------------------------------------------
void Get_Configuration (void)          // This routine returns current
{                                      // configuration value
   // This request must be directed to the device
   if ( (SETUP.bmRequestType != OUT_DEVICE)    ||
   // With value word set to zero
   SETUP.wValue.U8[MSB]  || SETUP.wValue.U8[LSB]||
   // And index set to zero
   SETUP.wIndex.U8[MSB]  || SETUP.wIndex.U8[LSB]||
   // And SETUP length set to one
   SETUP.wLength.U8[MSB] || (SETUP.wLength.U8[LSB] != 1) )
   {
      Force_Stall ();                  // Otherwise send a stall to host
   }

   else
   {
      if (USB0_STATE == DEV_CONFIGURED)// If the device is configured, then
      {                                // return value 0x01 since this software
                                       // only supports one configuration
         DATAPTR = (unsigned char*)&ONES_PACKET;
         DATASIZE = 1;
      }
      if (USB0_STATE == DEV_ADDRESS)   // If the device is in address state, it
      {                                // is not configured, so return 0x00
         DATAPTR = (unsigned char*)&ZERO_PACKET;
         DATASIZE = 1;
      }
   }
   if (EP_STATUS[0] != EP_STALL)
   {
      // Set Serviced Out Packet bit
      POLL_WRITE_BYTE (E0CSR, rbSOPRDY);
      EP_STATUS[0] = EP_TX;            // Put endpoint into transmit mode
      DATASENT = 0;                    // Reset Data Sent counter to zero
   }
}

//-----------------------------------------------------------------------------
// Set_Configuration
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change in custom HID designs.
//
//-----------------------------------------------------------------------------
void Set_Configuration (void)          // This routine allows host to change
{                                      // current device configuration value

   // Device must be addressed before configured
   if ((USB0_STATE == DEV_DEFAULT) ||
   // and request recipient must be the device
   (SETUP.bmRequestType != IN_DEVICE) ||
   // the index and length words must be zero
   SETUP.wIndex.U8[MSB]  || SETUP.wIndex.U8[LSB]||
   SETUP.wLength.U8[MSB] || SETUP.wLength.U8[LSB] ||
   SETUP.wValue.U8[MSB]  || (SETUP.wValue.U8[LSB] > 1))
   // This software only supports config = 0,1
   {
      Force_Stall ();                  // Send stall if SETUP data is invalid
   }

   else
   {
      if (SETUP.wValue.U8[LSB] > 0)    // Any positive configuration request
      {                                // results in configuration being set
                                // to 1
         USB0_STATE = DEV_CONFIGURED;
         EP_STATUS[1] = EP_IDLE;       // Set endpoint status to idle (enabled)
				 EP_STATUS[2] = EP_IDLE;
				
         POLL_WRITE_BYTE (INDEX, 1);   // Change index to endpoint 1
         // Set DIRSEL to indicate endpoint 1 is IN/OUT
         POLL_WRITE_BYTE (EINCSR2, rbInSPLIT);
				
				 POLL_WRITE_BYTE (INDEX, 2);   // Change index to endpoint 2
				 // Set DIRSEL to indicate endpoint 1 is IN/OUT
         POLL_WRITE_BYTE (EINCSR2, rbInSPLIT);
				
         // Double buffering is disabled.
		 // Because Split mode is enabled, Endpoint 1 is divided into two 64-byte
		 // sections. With double buffering enabled, they would further be divided
		 // into 32-byte sections.

         POLL_WRITE_BYTE (INDEX, 0);   // Set index back to endpoint 0

         Handle_In1();
				 Handle_In2();
      }
      else
      {
         USB0_STATE = DEV_ADDRESS;     // Unconfigures device by setting state
         EP_STATUS[1] = EP_HALT;       // to address, and changing endpoint 1
				 EP_STATUS[2] = EP_HALT;
      }
   }
   if (EP_STATUS[0] != EP_STALL)
   {
      POLL_WRITE_BYTE (E0CSR, (rbSOPRDY | rbDATAEND));
                                       // Indicate SETUP packet has been
                                       // serviced
   }
}

//-----------------------------------------------------------------------------
// Get_Interface
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - Non
//
// Standard request that should not change in custom HID designs.
//
//-----------------------------------------------------------------------------
void Get_Interface (void)              // This routine returns 0x00, since
{                                      // only one interface is supported by
                                       // this firmware

   // If device is not configured
   if ((USB0_STATE != DEV_CONFIGURED) ||
   // or recipient is not an interface
   (SETUP.bmRequestType != OUT_INTERFACE) ||
   // or non-zero value or index fields
   SETUP.wValue.U8[MSB]  ||SETUP.wValue.U8[LSB] ||
   // or data length not equal to one
   SETUP.wIndex.U8[MSB]  ||SETUP.wIndex.U8[LSB] ||
   SETUP.wLength.U8[MSB] ||(SETUP.wLength.U8[LSB] != 1))
   {
      Force_Stall ();                  // Then return stall due to invalid
                                       // request
   }

   else
   {
     // Otherwise, return 0x00 to host
      DATAPTR = (unsigned char*)&ZERO_PACKET;
      DATASIZE = 1;
   }
   if (EP_STATUS[0] != EP_STALL)
   {
     // Set Serviced SETUP packet, put endpoint in transmit mode and reset
     // Data sent counter
      POLL_WRITE_BYTE (E0CSR, rbSOPRDY);
      EP_STATUS[0] = EP_TX;
      DATASENT = 0;
   }
}

//-----------------------------------------------------------------------------
// Set_Interface
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Standard request that should not change in custom HID designs.
//
//-----------------------------------------------------------------------------
void Set_Interface (void)
{
   // Make sure request is directed at interface
   if ((SETUP.bmRequestType != IN_INTERFACE)  ||
   // and all other packet values are set to zero
   SETUP.wLength.U8[MSB] ||SETUP.wLength.U8[LSB]||
   SETUP.wValue.U8[MSB]  ||SETUP.wValue.U8[LSB] ||
   SETUP.wIndex.U8[MSB]  ||SETUP.wIndex.U8[LSB])
   {
      Force_Stall ();                  // Othewise send a stall to host
   }
   if (EP_STATUS[0] != EP_STALL)
   {
      POLL_WRITE_BYTE (E0CSR, (rbSOPRDY | rbDATAEND));
                                       // Indicate SETUP packet has been
                                       // serviced
   }
}

//-----------------------------------------------------------------------------
// Get_Idle
//-----------------------------------------------------------------------------
// Not supported.
//
//-----------------------------------------------------------------------------
void Get_Idle (void) { }

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Get_Protocol
//-----------------------------------------------------------------------------
// Not supported.
//
//-----------------------------------------------------------------------------
void Get_Protocol (void) { }

//-----------------------------------------------------------------------------
// Set_Protocol
//-----------------------------------------------------------------------------
// Not supported.
//
//-----------------------------------------------------------------------------
void Set_Protocol (void) { }

//-----------------------------------------------------------------------------
// Set_Idle()
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Description: Sets the idle feature on interrupt in endpoint.
//
//-----------------------------------------------------------------------------
void Set_Idle (void)
{
   if (EP_STATUS[0] != EP_STALL)
   {
      // Set serviced SETUP Packet
      POLL_WRITE_BYTE (E0CSR, (rbSOPRDY | rbDATAEND));
   }
}

//-----------------------------------------------------------------------------
// Get_Report()
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Description: Sends a given report type to the host.
//
//-----------------------------------------------------------------------------
void Get_Report (void)
{
	 /*
   // call appropriate handler to prepare buffer
   ReportHandler_IN_ISR(SETUP.wValue.U8[LSB]);
   // set DATAPTR to buffer used inside Control Endpoint
   DATAPTR = IN_BUFFER.Ptr;
   DATASIZE = IN_BUFFER.Length;
	 */
	
	 if (SETUP.wIndex.U8[LSB] == 0x00) {
	    // call appropriate handler to prepare buffer
			ReportHandler_IN_ISR(SETUP.wValue.U8[LSB]);
			// set DATAPTR to buffer used inside Control Endpoint
			DATAPTR = IN_BUFFER.Ptr;
			DATASIZE = IN_BUFFER.Length;
	 } else if (SETUP.wIndex.U8[LSB] == 0x01) {
			// call appropriate handler to prepare buffer
			ReportHandler_IN_ISR2(SETUP.wValue.U8[LSB]);
			// set DATAPTR to buffer used inside Control Endpoint
			DATAPTR = IN_BUFFER2.Ptr;
			DATASIZE = IN_BUFFER2.Length;
	 }
	
   if (EP_STATUS[0] != EP_STALL)
   {
     // Set serviced SETUP Packet
      POLL_WRITE_BYTE (E0CSR, rbSOPRDY);
      EP_STATUS[0] = EP_TX;            // Endpoint 0 in transmit mode
      DATASENT = 0;                    // Reset DATASENT counter
   }
}

//-----------------------------------------------------------------------------
// Set_Report()
//-----------------------------------------------------------------------------
//
// Return Value - None
// Parameters - None
//
// Description: Receives a report sent from the host.
//
//-----------------------------------------------------------------------------
void Set_Report (void)
{
   // prepare buffer for OUT packet
   Setup_OUT_BUFFER ();

   // set DATAPTR to buffer
   DATAPTR = OUT_BUFFER.Ptr;
   DATASIZE = SETUP.wLength.U16;

   if (EP_STATUS[0] != EP_STALL)
   {
     // Set serviced SETUP Packet
      POLL_WRITE_BYTE (E0CSR, rbSOPRDY);
      EP_STATUS[0] = EP_RX;            // Endpoint 0 in transmit mode
      DATASENT = 0;                    // Reset DATASENT counter
   }
}

//-----------------------------------------------------------------------------
// End of File
//-----------------------------------------------------------------------------
