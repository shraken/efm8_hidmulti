# efm8_hid_multi

Demonstrates a USB device using multiple HID interfaces using a full speed
USB Silicon Labs EFM8UB2 device.  This approach allows higher data rates over 
USB interrupt endpoints by spreading out the communication across multiple 
interfaces.  The example provided demonstrates a dual interface allowing 128kB/sec
but could be expanded to three interfaces for 192kB/sec on the EFM8UB2 device family.  
Other USB microcontrollers with more FIFO endpoints can expand this scaling
even further.  

A software project using the `hidapi` library is provided to verify data
receipt on the host.  

This project builds for a Silicon Labs EFM8UB2 device type and was tested on the
EFM8UB2 Universal Bee starter kit.  

The porting effort is described in a [blog entry](https://shrakeblogshere.blogspot.com/2017/11/using-multiple-usb-hid-interfaces-for.html) I posted.


## Requirements

* Keil uVision v5.2x
* CMake 2.8+

## Building and Flashing It

### Firmware
Open the hid_multi.uvproj file in Keil and build.  Flash the hex file to
the microcontroller using the internal JLink OB.  You can use Simplicity
Studio GUI or JLinkExe.

### Software
`mkdir build`

`cd build`

`cmake ..`

On Unix-based systems:

`make`

On Windows-based systems:
Open the file in Visual Studio and initiate a build.

## Using It

Locate where the software binary using hidapi was built.  Launch the
application with no arguments and record the `path` outputs for the
device with a Manufacturer name of `SLAB` and a Product name of
`HIDtoUART example`.  There should be two entries, one corresponding
to each HID interface.  

Open two terminals and launch the software binary in each terminal with
the unique paths recorded in the previous step.  You should observe a dump
to console with the read data where the first byte of the array indicates
the report number (0x01 and 0x03).