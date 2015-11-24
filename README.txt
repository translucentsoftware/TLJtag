Translucent Software (Josh Gibson) patches to JTMOD and tjtag v3.0.1
------------------------------------------------

These patches were done to create a usable Arduino jtag adapter for Mac OS X.
JTMOD was designed for linux and Windows but Mac OS X required some significant changes
in order for it to properly communicate with the Arduino and not have any communication errors.

It is based heavily off the source code of JTMOD (https://github.com/zoobab/tjtag-arduino).
The serial communication library portion was completely rewritten but the Arduino sketch only 
had slight modifications for clarity and cable selection.

------------------------------------------------

- Mac OS X only
This patch only works on Mac OS X but still contains the modifications in tjtag from JTMOD.
tjtag has also been patched in places to make it compatible with Mac OS X.

------------------------------------------------

- Arduino
arduiggler is JTMODs Arduino sketch and has been patched to provide the ability
to use different cable types both Wiggler and Xilinx style cables. 

Wiggler is the preferred cable choice as it works without issue. 

The Xilinx cable seems to lack some jtag abilities. Currently it does not identify the 
EJTAG version or probe the flash. It also seems unable to restart the processor in debug mode.
  
------------------------------------------------

The cable hookups are as follows:

	JTAG	   - Wiggler -	     Arduino		JTAG	   - Xilinx -	    Arduino
	1  - nTRST ———————————-——————> 6 		1  - nTRST ——————————————————> X
	3  - TDI ————————————————————> 5		3  - TDI ————————————————————> 2
	5  - TDO ————————————————————> 7		5  - TDO ————————————————————> 5
	7  - TMS ————————————————————> 3		7  - TMS ————————————————————> 4
	9  - TCK ————————————————————> 4		9  - TCK ————————————————————> 3
	11 - nSRST ——————————————————> 2		11 - nSRST ——————————————————> X

Note: JTAG pin 1 is ALWAYS the square pad and is the top-left orientation.

The Arduino pins need to be connected with 1K Ohm resisters on 5v Arduinos as JTAG pins are 3v.
The ground pins 4 - 10 (Right side) need to be hooked up to the Arduino ground. 

------------------------------------------------

tl_tjtag is the Mac OS X patched version of tjtag and accepts all of the same commands as tjtag

The environment variable TL_TJTAG__PORT needs to be set to the Arduino port,
it should be something like: /dev/tty.usbserial*

Example:
	~> set TL_TJTAG_PORT=/dev/tty.usbserial-A603UE5O
	~> ./tl_tjtag -probeonly /wiggler

------------------------------------------------

tl_tjtag_tests is a test application that will test turing on and off the pins.
		LEDs can be attached to each pin to verify everything is working.


------------------------------------------------

As with the JTMOD, this patch also runs very slow! It most likely is due to the latency experienced with
sending to, waiting for and finally receiving back from the Arduino. But it works and can keep you from
spending money on buying a jtag cable.

